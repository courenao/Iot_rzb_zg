/*
 V0.1 Initial Release   10/July/2006  RBR

 */

#include "hal.h"
#include "halStack.h"
#include "console.h"
//#include "debug.h"
#include "ieee_lrwpan_defs.h"
#include "memalloc.h"
#include "phy.h"
#include "mac.h"
#include "MCR20Reg.h"
#include "MCR20Drv.h"
#include "MCR20A_Phy.h"
#include "MCR20A_port.h"

#define MCR20A_PAN   0  //PAN0��PAN1��MCR20A����˫PAN����
static volatile RADIO_FLAGS local_radio_flags; //��¼��Ƶģ�鹤��ģʽ״̬
static volatile PHY_FREQ_ENUM freq_pre; //��¼����
static volatile BYTE channel_pre;
BYTE halRadioInited = 0; //��Ƶģ���ʼ����־��0-δ��ʼ����1-�Ѿ���ʼ��
BYTE halSendStart = 0; //���Ϳ�ʼ��־��1-���Ϳ�ʼ��0-���ͽ�����δ��ʼ
BYTE txRetry = 1; //�ط�����(ʵ���ܵķ��ʹ�����1��ʾֻ����1��û���ط���2��ʾ����2�Σ�3����3��)

#if RX_AUTO_TIMEOUT_ENABLE//����RX�Զ���ʱ�˳�����
//802.15.4Э���� һ�����ݵȴ��ԼΪ5ms������ACK������������趨Ϊ6ms
#define RX_TIMEOUT   375 //����ʱ��ʱֵ,T3ÿ16usһ�μ�����16us*375=6ms
#endif

//irq status�����ֽ���ǰ��
typedef struct {
	BYTE seq :1; //����ʱ�����
	BYTE tx :1; //txʱ�����
	BYTE rx :1; //rxʱ�����
	BYTE cca :1; //CCAʱ�����
	BYTE rxWaterMark :1; //�������ݸ��������趨��ֵ
	BYTE filterFail :1; //
	BYTE pllUnlock :1;
	BYTE rxFramePend :1; //����

	BYTE wake :1; //����
	BYTE pb_err :1; //���ݰ�����
	BYTE aes :1; //AES�������
	BYTE tmrStatus :1; //��ʱ��״̬
	BYTE pi :1; //���յ��İ��Ƿ�����������
	BYTE srcaddr :1; //Դ��ַƥ��
	BYTE ccaBusy :1; //cca����״̬
	BYTE crcValid :1; //CRC��Ч��־

	BYTE tmr1 :1; //TMR1�жϣ�1��Ч
	BYTE tmr2 :1; //TMR2�жϣ�1��Ч
	BYTE tmr3 :1; //TMR3�жϣ�1��Ч
	BYTE tmr4 :1; //TMR4�жϣ�1��Ч
	BYTE tmr1msk :1; //TMR1�ж�ʹ�ܣ�0��Ч
	BYTE tmr2msk :1; //TMR2�ж�ʹ�ܣ�0��Ч
	BYTE tmr3msk :1; //TMR3�ж�ʹ�ܣ�0��Ч
	BYTE tmr4msk :1; //TMR4�ж�ʹ�ܣ�0��Ч
} irq_t;
irq_t irq;

#ifdef  LRWPAN_ASYNC_INTIO  //�첽�����շ������û��崦��
static volatile BYTE serio_rxBuff[LRWPAN_ASYNC_RX_BUFSIZE];
static volatile BYTE serio_rxHead, serio_rxTail;
#endif

void MCR20A_SetSeq(uint8_t seq) {
	BYTE phyReg;
	if (gIdle_c == seq || gTR_c == seq || gTX_c == seq || gRX_c == seq
			|| gCCA_c == seq) {

		phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
		phyReg &= ~(cPHY_CTRL1_XCVSEQ);
		phyReg |= seq;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
	}
}

/*---------------------------------------------------------------------------
 * @����: halInit
 * @����: ��ʼ��Ӳ���豸
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/
void halInit(void) {
	//Set clock source
	local_radio_flags.val = 0;

	//��ʼ��Э��ջ��ӡ����˿�
	halInitUart();
	//��ʼ��Ӳ������ʼ����MCR20A_PAN
	MCR20A_HwInit(MCR20A_PAN);

#ifdef LRWPAN_COORDINATOR
	//PAN_COORDINATOR �Ƿ���Э������Э�����������е�ַ�����ݣ�����ֻ�����Լ�ΪĿ�������
	//����ΪPAN_COORDINATORģʽ���������е�ַ������
	MCR20A_SetMacRole(PAN_COORDINATOR, MCR20A_PAN);
#else
	//ֻ����Ŀ���ַΪ�Լ�������
	MCR20A_SetMacRole(PAN_ENDDIVCE, MCR20A_PAN);
#endif

#if DATA_ENCRYPTION_ENABLE //����ASMӲ������
	if (MCR20A_ASM_Selftest()) {
		conPrintROMString_func("Data Encryption Test OK !\n");
	} else {
		conPrintROMString_func(
				"Data Encryption Test FAILED ! **End Program** !\n");
		exit(-1); //������������ݼ��ܵ��Ǽ��ܼ������Բ���ʧ�ܣ���ǿ����ֹ��������
	}
#endif

}

/*---------------------------------------------------------------------------
 * @����: halInitUart
 * @����: ��ʼ��UART�豸
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/
void halInitUart(void) {

#ifdef LRWPAN_ASYNC_INTIO
	serio_rxHead = 0;
	serio_rxTail = 0;
	//���������ж�
	//INT_ENABLE_URX0(INT_ON);
#endif

}

#ifdef  LRWPAN_ASYNC_INTIO

/*---------------------------------------------------------------------------
 * @����: halGetch
 * @����: (���巽ʽ)�Ӵ����л�ȡһ���ֽڵ�����
 *        (�첽��ʽ��ֱ�Ӷ���������������Դ���жϷ���)
 *        ����������ѭ���ṹ����ȡ������β���������»ص�ͷ��
 * @����: ��
 * @���: char������
 * @��ע: ��ģ�麯�����κ�Ӳ���޹أ�����ֱ����ֲ��
 *---------------------------------------------------------------------------*/
char halGetch(void) {
	char x;
	do {
		x = serio_rxHead; //use tmp because of volt decl
	}while (serio_rxTail == x);
	serio_rxTail++;
	if (serio_rxTail == LRWPAN_ASYNC_RX_BUFSIZE)
	serio_rxTail = 0;
	return (serio_rxBuff[serio_rxTail]);
}

/*---------------------------------------------------------------------------
 * @����: halInitUart
 * @����: (���巽ʽ)UART�Ƿ������ݿɶ�״̬��
 *        ֻҪ��ǰ�����ͷ��־��β����־���ȼ���Ϊ���������ݡ�
 * @����: ��
 * @���: BOOL��״̬
 * @��ע: ��ģ�麯�����κ�Ӳ���޹أ�����ֱ����ֲ��
 *---------------------------------------------------------------------------*/
BOOL halGetchRdy(void) {
	char x;
	x = serio_rxHead;
	return (serio_rxTail != x);
}

#else

/*---------------------------------------------------------------------------
 * @����: halGetch
 * @����: (ֱ�Ӷ�ȡ)�Ӵ����л�ȡһ���ֽڵ�����
 * @����: ��
 * @���: char������
 * @��ע:
 *---------------------------------------------------------------------------*/
char halGetch(void) {
	char c;

	return c;
}

/*---------------------------------------------------------------------------
 * @����: halGetchRdy
 * @����: (ֱ�Ӷ�ȡ)UART�Ƿ������ݿɶ�״̬��
 * @����: ��
 * @���: BOOL��״̬
 * @��ע:
 *---------------------------------------------------------------------------*/
BOOL halGetchRdy(void) {
	BOOL ret = 0;

	return ret;
}

#endif

/*---------------------------------------------------------------------------
 * @����: halUtilMemCopy
 * @����: �ڴ濽��
 * @����: ��
 * @���: ��
 * @��ע: ��ģ�麯�����κ�Ӳ���޹أ�����ֱ����ֲ��
 *---------------------------------------------------------------------------*/
void halUtilMemCopy(BYTE *dst, BYTE *src, BYTE len) {
	while (len) {
		*dst = *src;
		dst++;
		src++;
		len--;
	}
}

/*---------------------------------------------------------------------------
 * @����: halGetMACTimer
 * @����: ��ȡMACʱ��
 *        ��ȡMCR20A��ʱ��ֵ�����ڼ���CCA
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/
UINT32 halGetMACTimer(void) {
	UINT32 ret;
#if MCR20A_TIMER_ENABLE
	ret = MCR20A_GetMACTimer();
#else
	ret = GetTimeMS();
#endif
	return ret;
}

#ifdef LRWPAN_COMPILER_NO_RECURSION

/*---------------------------------------------------------------------------
 * @����: halISRGetMACTimer
 * @����: �жϻ�ȡMACʱ��
 *        ��ȡMCR20A��ʱ��ֵ�����ڼ���CCA
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

UINT32 halISRGetMACTimer(void)
{
	return halGetMACTimer();
}

#endif

/*---------------------------------------------------------------------------
 * @����: halMacTicksToUs
 * @����: ����һ��us��Ҫ��Ticks��
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/
UINT32 halMacTicksToUs(UINT32 ticks) {
	return GetUsFromTicks(ticks);
}

/*---------------------------------------------------------------------------
 * @����: halGetRandomByte
 * @����: ��ȡһ���ֽڵ������
 * @����: ��
 * @���: UINT8�͵������
 * @��ע:
 *---------------------------------------------------------------------------*/
UINT8 halGetRandomByte(void) {
	UINT8 byte = 0;
	UINT32 u32 = 0; //�洢32λ����

#if MCR20A_MCU_RNG_ENABLE //ʹ��STM32_CPU ��RNG�����������������Ч���Ч��
	byte = MCR20A_MCU_GetRNG();
#else//ʹ��MCR20AоƬ�Դ���RNG�����������鲻����
	MCR20A_GetRandomNo((uint32_t *) &u32);
	byte = (UINT8) u32; //��ȡ8λ����
#endif
	return (byte);
}

/*---------------------------------------------------------------------------
 * @����: halPutch
 * @����: ����дһ���ֽڵ���
 * @����: char������
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

void halPutch(char c) {
	xUSART2_putchar(c);
}

/*---------------------------------------------------------------------------
 * @����: halRawPut
 * @����: ����дһ���ֽڵ���(��halPutch��ͬ)
 * @����: char������
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/
void halRawPut(char c) {
	xUSART2_putchar(c);
}

/*---------------------------------------------------------------------------
 * @����: halSetChannel
 * @����: ����2.4GHz����Ƶģ���ͨ��
 * @����: BYTE channel-ͨ��
 * @���: LRWPAN_STATUS_ENUMö�����͵�״ֵ̬
 * @��ע: ��2.4G��ͨ���Ѿ�������Ƶ�ʵľ���ֵ
 *---------------------------------------------------------------------------*/
LRWPAN_STATUS_ENUM halSetChannel(BYTE channel) {
	if ((channel < 11) || (channel > 26))
		return (LRWPAN_STATUS_PHY_FAILED);

	MCR20A_SetCurrentChannelRequest(channel, MCR20A_PAN); //Ĭ�ϲ���PAN0

	return LRWPAN_STATUS_SUCCESS;
}

/*---------------------------------------------------------------------------
 * @����: halSetRadioIEEEFrequency
 * @����: ������Ƶģ���Ƶ�ʺ�ͨ��
 * @����: BYTE *buf -��ַ����׵�ַ
 * @���: ��
 * @��ע: 1.��˴洢��ַ �� 2.����ռ���ſ��Դ����FLAH��
 *---------------------------------------------------------------------------*/

LRWPAN_STATUS_ENUM halSetRadioIEEEFrequency(PHY_FREQ_ENUM frequency,
		BYTE channel) {
	//ֻ֧��2.4G
	if (PHY_FREQ_2405M != frequency)
		return LRWPAN_STATUS_PHY_FAILED; //ֱ���˳�

	return halSetChannel(channel);
}

/*---------------------------------------------------------------------------
 * @����: halGetProcessorIEEEAddress
 * @����: ��FLASH�л�ȡIEEE��ַ
 * @����: BYTE *buf -��ַ����׵�ַ
 * @���: ��
 * @��ע: 1.��˴洢��ַ �� 2.����ռ���ſ��Դ����FLAH��
 *---------------------------------------------------------------------------*/

void halGetProcessorIEEEAddress(BYTE *buf) {
#if USE_AUTO_MAC
	Board_Mac_Get(buf);
#else
	buf[0] = aExtendedAddress_B0;
	buf[1] = aExtendedAddress_B1;
	buf[2] = aExtendedAddress_B2;
	buf[3] = aExtendedAddress_B3;
	buf[4] = aExtendedAddress_B4;
	buf[5] = aExtendedAddress_B5;
	buf[6] = aExtendedAddress_B6;
	buf[7] = aExtendedAddress_B7;
#endif
}

/*---------------------------------------------------------------------------
 * @����: halSetRadioIEEEAddress
 * @����: ��IEEE��ַ���õ�MCR20A��ƵоƬ
 * @����: ��
 * @���: ��
 * @��ע: IEEE��ַ���뱣֤ÿ���ڵ���Ψһ��
 *---------------------------------------------------------------------------*/

void halSetRadioIEEEAddress(void) {
	BYTE buf[8];
	halGetProcessorIEEEAddress(buf);

	MCR20A_SetLongAddr(buf, MCR20A_PAN);
}

/*---------------------------------------------------------------------------
 * @����: halSetRadioPANID
 * @����: ������ƵоƬ��PANIDֵ
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

void halSetRadioPANID(UINT16 panid) {
	MCR20A_SetPanId((uint8_t *) &panid, MCR20A_PAN);
}

/*---------------------------------------------------------------------------
 * @����: halSetRadioShortAddr
 * @����: ������ƵоƬ�������ַ
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

void halSetRadioShortAddr(SADDR saddr) {
	MCR20A_SetShortAddr((uint8_t *) &saddr, MCR20A_PAN);
}

//return value of non-zero indicates failure
//Turn on Radio, initialize for RF mode
//assumes that auto-ack is enabled
//this function based on sppInit example from ChipCon
//also sets the IEEE address
//if listen_flag is true, then radio is configured for
//listen only (no auto-ack, no address recognition)

/*
 Eventually, change this so that auto-ack can be configured as
 on or off. When Coordinator is trying to start a network,
 auto-ack, addr decoding will be off as Coordinator will be doing an energy
 scan and detecting PAN collisions, and thus will not be doing
 any acking of packets. After the network is started, then
 will enable auto-acking.
 Routers and End devices will always auto-ack

 i
 */

/*---------------------------------------------------------------------------
 * @����: halSetRadioShortAddr
 * @����: ������ƵоƬ��Ƶ��
 * @����: Ƶ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

LRWPAN_STATUS_ENUM halInitRadio(PHY_FREQ_ENUM frequency, BYTE channel,
		RADIO_FLAGS radio_flags) {
	LRWPAN_STATUS_ENUM status;
	//Ƶ���б仯������Ƶ��ֵ
	if (freq_pre != frequency || channel_pre != channel) {
		freq_pre = frequency;
		halSetRadioIEEEAddress();
		//������Ƶģ���Ƶ�ʺ�ͨ��
		status = halSetRadioIEEEFrequency(frequency, channel);
		if (status != LRWPAN_STATUS_SUCCESS)
			return (status);

#ifdef LRWPAN_COORDINATOR	//Э����Ĭ�Ͻ������ʱ��
		BYTE phyReg;
		//����RXʱ��
		phyReg = mStatusAndControlRegs[PHY_CTRL1_INDEX_c];
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD);
		phyReg |= gRX_c;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
#if RX_AUTO_TIMEOUT_ENABLE
		//����T3��ʱ��������Ӳ���Զ���ʱ�˳�RXʱ��

		phyTime_t timeOut = 0;
		MCR20Drv_DirectAccessSPIMultiByteRead(EVENT_TMR_LSB, (BYTE *) &timeOut,
				3);
		timeOut += RX_TIMEOUT;
		timeOut &= 0x0FFFFFF;
		MCR20Drv_DirectAccessSPIMultiByteWrite(T3CMP_LSB, (BYTE *) &timeOut, 3);
		/* enable autosequence stop by TC3 match */
		mStatusAndControlRegs[PHY_CTRL4_INDEX_c] |= cPHY_CTRL4_TC3TMOUT;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL4,
				mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
#endif//endof RX_AUTO_TIMEOUT_ENABLE
#endif//endof LRWPAN_COORDINATOR
	}
	halRadioInited = 1;

	return (LRWPAN_STATUS_SUCCESS);
}

/*---------------------------------------------------------------------------
 * @����: doIEEE_backoff
 * @����: ����CCA��ͻ����
 * @����: ��
 * @���: ��
 * @��ע: ���MCR20AӲ����ʼ��ʱ������ΪCCABeforeTx��
 *---------------------------------------------------------------------------*/

void doIEEE_backoff(void) {
	BYTE be, nb, tmp, rannum;
	UINT32 delay, start_tick;

	be = aMinBE;
	nb = 0;
	do {
		if (be) {
			//do random delay
			tmp = be;
			//compute new delay
			delay = 1;
			while (tmp) {
				delay = delay << 1; //delay = 2**be;
				tmp--;
			}
			rannum = halGetRandomByte() & (delay - 1); //rannum will be between 0 and delay-1
			delay = 0;
			while (rannum) {
				delay += SYMBOLS_TO_MACTICKS(aUnitBackoffPeriod);
				rannum--;
			} //delay = aUnitBackoff * rannum
			  //now do backoff
			start_tick = halGetMACTimer();
			while (halMACTimerNowDelta(start_tick) < delay)
				;
		}
		//check CCA
		//���CCA״̬�����CCA�������������˳�
#if 0
		if (PIN_CCA)
		break;
#else

		MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
				&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 1);
		if (mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] & cIRQSTS2_CCA) {
			//busy
		} else
			//CCA��鷢��IDLE
			break;
#endif
		nb++;
		be++;
		if (be > aMaxBE)
			be = aMaxBE;
	} while (nb <= macMaxCSMABackoffs); //�������5�Σ�[0,4]
	return;
}

#ifdef  LRWPAN_ASYNC_INTIO

/*---------------------------------------------------------------------------
 * @����: urx0_service_IRQ
 * @����: UART0�жϷ������
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/INTERRUPT_FUNC urx0_service_IRQ(
		void) {

	BYTE x, y;

	serio_rxHead++;
	if (serio_rxHead == LRWPAN_ASYNC_RX_BUFSIZE)
	serio_rxHead = 0;
	x = serio_rxHead; //use tmp variables because of Volatile decl
	//y = U0DBUF; //��ȡ����
	serio_rxBuff[x] = y;
}

#endif

//This timer interrupt is the periodic interrupt for
//evboard functions
//����ʹ�ã���֧��
#undef LRWPAN_ENABLE_SLOW_TIMER
#ifdef LRWPAN_ENABLE_SLOW_TIMER

// assuming 16us period, have 1/16us = 62500 tics per seocnd
#define T2CMPVAL (62500/SLOWTICKS_PER_SECOND)

/*---------------------------------------------------------------------------
 * @����: t2_service_IRQ
 * @����: T2�жϷ�������������ʹ��
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/
INTERRUPT_FUNC t2_service_IRQ(void)
{
	UINT32 t;

	INT_GLOBAL_ENABLE(INT_OFF);
	INT_SETFLAG_T2(INT_CLR); //clear processor interrupt flag
	//compute next compare value by reading current timer value, adding offset
	t = 0x0FF & T2OF0;
	t += (((UINT16)T2OF1) << 8);
	t += (((UINT32) T2OF2 & 0x0F) << 16);
	t += T2CMPVAL;//add offset
	T2PEROF0 = t & 0xFF;
	T2PEROF1 = (t >> 8) & 0xFF;
	//enable overflow count compare interrupt
	T2PEROF2 = ((t >> 16) & 0x0F) | 0x20;
	T2CNF = 0x03;//this clears the timer2 flags
	evbIntCallback();//Evaluation board callback
	usrSlowTimerInt();//user level interrupt callback
	INT_GLOBAL_ENABLE(INT_ON);
}
#endif


void halGetPanId(BYTE *pPanId)
{
	MCR20Drv_IndirectAccessSPIMultiByteRead((uint8_t) MACPANID0_LSB,
					pPanId, 2);
}

void halGetShortId(BYTE *id)
{
	MCR20Drv_IndirectAccessSPIMultiByteRead((uint8_t) MACSHORTADDRS0_LSB,
			id, 2);
}
//���Ϳ���״̬������Ƶģ��Ϊ����״̬
void halIdle() {
	//halResetSeq();
}

//���Ͳ�����ACK�ȴ���ʱ,�˳�ACK���յȴ�ʱ��
void halTxTimeOut() {
	BYTE phyReg, xcvseq, rxAckRqd, seqState;

	//�ر��ж�
	MCR20Drv_IRQ_Disable();
	MCR20Drv_IRQ_Clear();

	phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
	rxAckRqd = phyReg & (cPHY_CTRL1_RXACKRQD); //��ȡ��ǰACK�ȴ���־

	if (rxAckRqd) { //cPHY_CTRL1_RXACKRQD ��Ч���ʾ��ǰ����ACK�ȴ�

		xcvseq = phyReg & (cPHY_CTRL1_XCVSEQ); //��ȡ��ǰʱ��
		//��ǰ�Ƿ��ڽ���״̬
		if (gRX_c == xcvseq || gTR_c == xcvseq) {
			seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;
			if (seqState) { //��Ϊ0����ʾ��Ƶģ�鲻Ϊ����״̬

				if ((seqState <= 0x06) || (seqState == 0x15)
						|| (seqState == 0x16)) {
					//RXʱ�����ڽ������ݰ����������κ�ʱ�򣬵ȴ��������
					MCR20Drv_IRQ_Enable();
					return LRWPAN_STATUS_PHY_TX_START_FAILED;
				}

				//��ϵ�ǰ����ʱ��Ϊ������������ʱ����׼��
				phyReg &= ~(cPHY_CTRL1_XCVSEQ);
				phyReg |= gIdle_c;
				MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
				//�ȴ�ʱ����ֹ���
				while (MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F)
					;
			}
		}

		//�����Ƶģ���жϼĴ��������ͺ����в������ж϶�����Ҫ����
		mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
				MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
						&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 2);
		//���жϱ�־
		MCR20Drv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs,
				3);

		//ȥ��ACK�ȴ�
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD);
		phyReg |= gRX_c;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
	}
	MCR20Drv_IRQ_Enable();
}

void halResetSeq() {
	MCR20Drv_IRQ_Disable();
	//MCR20Drv_IRQ_Clear();

	BYTE seqState = 0;
	seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;

	//����ģʽ����
	mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
			MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
					&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 7);

	BYTE phyReg;
	phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);

	//��ƵоƬ��������������Ӧ��һֱ����RX��TX״̬��ֻ�����жϴ������лᴦ�ڿ���
	//��һоƬ���ڿ��е�����û�н����жϺ�������Ҫ�ô��뽫����ΪRX״̬���������жϺ���
	if ((0 == seqState)) {
		//����RXʱ��
		phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD);
		phyReg |= gRX_c;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);

#if RX_AUTO_TIMEOUT_ENABLE
		//����T3��ʱ��������Ӳ���Զ���ʱ�˳�RXʱ��

		phyTime_t timeOut = 0;
		MCR20Drv_DirectAccessSPIMultiByteRead(EVENT_TMR_LSB, (BYTE *) &timeOut,
				3);
		timeOut += RX_TIMEOUT;
		timeOut &= 0x0FFFFFF;
		MCR20Drv_DirectAccessSPIMultiByteWrite(T3CMP_LSB, (BYTE *) &timeOut, 3);
		/* enable autosequence stop by TC3 match */
		mStatusAndControlRegs[PHY_CTRL4_INDEX_c] |= cPHY_CTRL4_TC3TMOUT;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL4,
				mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
#endif

	}
	MCR20Drv_IRQ_Enable();
}

void evbPoll() {

}

void halRadioGetFrame(BYTE flen) {

	//BYTE ack_bytes[LRWPAN_ACKFRAME_LENGTH];
	BYTE ack;
	BYTE rssi;
	BYTE *ptr = NULL, *rx_frame = NULL;
	BYTE delayTmp = 50;

	if (5 > flen || flen > 127) //�Ƿ����ȣ�ֱ���˳�����С����ΪACK=5
		return;

	if (LRWPAN_ACKFRAME_LENGTH == flen) { //ACK
		//MCR20A�Զ�У��ACK
		if (LRWPAN_GET_ACK_REQUEST(*(phy_pib.currentTxFrm))) {
			phyTxEndCallBack(); //��TX��ʶ
		}
		ack = LRWPAN_ACKFRAME_LENGTH;
		macRxCallback(&ack, rssi);
	} else {
		if (!macRxBuffFull()) {
			//����ռ䣬ʵ����Ҫ�����ݿռ�Ϊflen-2+1
			rx_frame = MemAlloc(flen - 2 + 1);
			ptr = rx_frame;
		} else {
			//MAC RX buffer is full
		}
		if (ptr == NULL) {
			//���ڴ棬������
		} else {
			//������ݳ��ȣ�1���ֽ�
			*ptr = flen;
			ptr++;
			//MCR20A������CRC2�ֽ�Ӳ���Զ�У�飬����Ҫ��ȡ,ʵ�ʶ�ȡ����flen-2
			MCR20Drv_PB_SPIBurstRead((uint8_t *) (ptr), flen - 2);

			rssi = MCR20Drv_DirectAccessSPIRead(LQI_VALUE);
			if (0 == rssi) {
				while (delayTmp--)
					; //�ȴ�һ��ʱ�䣬�������ʱ���д���֤
				rssi = MCR20Drv_DirectAccessSPIRead(LQI_VALUE);
			}
			//��RSSIֵ��2�Ĵη���ʾת���� 0~0XFF����ֵ MCR20Drv_DirectAccessSPIRead(LQI_VALUE);����Ҫ�ٴ���
			rssi = MCR20A_LqiConvert(rssi);

			//����PHY�ص�����
			phyRxCallback();
			macRxCallback(rx_frame, rssi);
		}
	}
}

/*---------------------------------------------------------------------------
 * @����: halShutdown
 * @����: �ر�
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

void halShutdown(void) {
	//disable some interrupts
#ifdef LRWPAN_ENABLE_SLOW_TIMER
	INT_ENABLE_T2(INT_OFF);
#endif

	//�ر�MCR20A�ж�
	//����MCR20A�ж�
	//��MCR20A����ΪIDLE״̬
	MCR20A_Abort();

}

/*---------------------------------------------------------------------------
 * @����: halWarmstart
 * @����: ��Ƶģ��������
 * @����: ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

void halWarmstart(void) {
#if 0
	UINT32 myticks;
	//re-enable the timer interrupt
#ifdef LRWPAN_ENABLE_SLOW_TIMER
	INT_ENABLE_T2(INT_ON);
#endif
	//turn on the radio again
	RFPWR = RFPWR & ~(1 << 3);//RFPWR.RREG_RADIO_PD = 0;
	//wait for power to stabilize
	myticks = halGetMACTimer();
	while (halMACTimerNowDelta(myticks) < MSECS_TO_MACTICKS(10)) {
		//check the power up bit, max time is supposed to be 2 ms
		if (!(RFPWR & ~(1 << 4))) break;
	}

#endif
}

/*---------------------------------------------------------------------------
 * @����: halSleep
 * @����: ��Ƶģ������
 * @����: UINT32 msecs-����ʱ��
 * @���: ��
 * @��ע:
 *---------------------------------------------------------------------------*/

void halSleep(UINT32 msecs) {
	//�ж���Ƶģ���Ƿ���IDLE״̬
	//�����IDLE���������

	//�ر������ж�(��Ƶ���)
	//���жϱ�־

	//������Ƶģ������߶�ʱ��

	//������Ƶģ��Ϊ�͹���ģʽ

	//����Ƶģ����ж�

}

//cPHY_CTRL2_SEQMSK����SEQ�жϣ������ж��շ�
//���ܽ��յ�ACK(ACK���ͼ����10ms֮�ڣ���Ӳ���Զ����ͣ�������Ӧ������)
void IRQ_Handler() {

	BYTE xcvseq = 0, seqState;
	BYTE *ptr;
	BYTE tmpbuf[LRWPAN_MAX_FRAME_SIZE + 1]; //maximum payload size
	BYTE phyReg = 0, seqNext = gRX_c;
	BYTE flen = 0;
	phyTime_t timeOut = 0;

	/* Disable and clear transceiver(IRQ_B) interrupt */
	MCR20Drv_IRQ_Disable();
	MCR20Drv_IRQ_Clear();

	mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
			MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
					&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 7);
	//���жϱ�־
	MCR20Drv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs, 3);

	//��ȡ��ǰ�ж�����
	halUtilMemCopy((BYTE *) &irq, mStatusAndControlRegs, sizeof(irq));
	//�����жϣ���ƵоƬ��λ��Ĭ�Ͽ���
	if (irq.wake
			|| ((mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] == 0)
					&& (mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] == 0))) {
		//ֱ���˳��жϲ�����
		MCR20Drv_IRQ_Enable();
		return;
	}

	//��ȡ��ǰʱ���������״̬
	xcvseq = mStatusAndControlRegs[PHY_CTRL1_INDEX_c] & cPHY_CTRL1_XCVSEQ;
	//�������ݳ���
	flen = mStatusAndControlRegs[PHY_RX_FRM_LEN_INDEX_c];
	//����ʱ��״̬
	phyReg = mStatusAndControlRegs[PHY_CTRL1_INDEX_c];

	/*-----------------------�����жϴ���(����Ƚ��մ�����ִ��)------------------------*/
	if (irq.tx) {
		if (irq.ccaBusy) //CCA��⵽�ŵ�æ��������ֹ����Ҫ���·���
		{
			//PB���ݻ������л��������ϴε����ݣ�����Ҫ����װ�����ݣ�ֱ������TXʱ�򼴿���ɿ�������
			if (txRetry > 0) //�ط��������ݼ�
				txRetry--;
			if (txRetry && halSendStart) {
				//��ǰʱ��Ϊ����ʱ�����ط�����ֻ��Ҫ��дʱ�������Ѿ��ڻ��������ˡ�
				if (gTX_c == xcvseq || gTR_c == xcvseq) {
					MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
					MCR20Drv_IRQ_Enable();
					return; //�˳�
				}
			}
		} else { //���ͳɹ�
			if (LRWPAN_GET_ACK_REQUEST(*(phy_pib.currentTxFrm))) {
				//��ҪACKȷ��
				//��ȡ��ACK��ŵ���phyTxEndCallBack();
			} else { //����ҪACK
				phyTxEndCallBack();
			}
			halSendStart = 0; //���ͽ���
			txRetry = 0;
		}
	}

	/*-----------------------------�������ݴ���--------------------------*/

	if (irq.rx) { //������ձ�־��Ч��ʾһ�����������ݰ�������ɣ�ֱ�Ӵ���
		halRadioGetFrame(flen);
	}

	seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;
	/*-----------------------------ʱ���������--------------------------*/
	if (irq.seq || (seqState == 0)) { //ʱ��������������������״̬
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD); //ȥ��ACK�ȴ�
		seqNext = gRX_c;
		phyReg |= seqNext;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
#if RX_AUTO_TIMEOUT_ENABLE
		//����T3��ʱ��������Ӳ���Զ���ʱ�˳�RXʱ��

		MCR20Drv_DirectAccessSPIMultiByteRead(EVENT_TMR_LSB, (BYTE *) &timeOut,
				3);
		timeOut += RX_TIMEOUT;
		timeOut &= 0x0FFFFFF;
		MCR20Drv_DirectAccessSPIMultiByteWrite(T3CMP_LSB, (BYTE *) &timeOut, 3);
		/* enable autosequence stop by TC3 match */
		mStatusAndControlRegs[PHY_CTRL4_INDEX_c] |= cPHY_CTRL4_TC3TMOUT;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL4,
				mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
#endif
	}

	MCR20Drv_IRQ_Enable();

}
void MCR20A_IRQ_Entry() {
	if (EXTI_GetITStatus(MCR20A_IRQ_EXTI_LINE) != RESET) {
		uint32_t us_pre, us_now;
		us_pre = GetTimeUS();
		IRQ_Handler();
		us_now = GetTimeUS();
		uint32_t delta = us_now - us_pre;
	}
}

//�����жϷ��ͣ��ú����������ݴ���
LRWPAN_STATUS_ENUM halSendPacket(BYTE flen, BYTE *frm) {
	LRWPAN_STATUS_ENUM res;
	//total length, does not include length byte itself
	//last two bytes are the FCS bytes that are added automatically
	if (flen < 5 || flen + PACKET_FOOTER_SIZE > 127) {
		//packet size is too large!
		return LRWPAN_STATUS_PHY_TX_PKT_TOO_BIG;
	}

	BYTE xcvseq, seqState = 1; //��ʼseqState ��Ϊ0
	BYTE *ptr;
	BYTE tmpbuf[LRWPAN_MAX_FRAME_SIZE + 1]; //maximum payload size
	BYTE phyReg = 0, seqNext;
	BYTE txRetry = 0; //�����ط���1��Ч
	BYTE nextSeqEnable = 0; //������һ��ʱ���ʶ��1��Ч
	BYTE timeOutCount; //���RX����TRʱ��ʱ������

	phyTxStartCallBack();

	//�ر��ж�
	MCR20Drv_IRQ_Disable();
	MCR20Drv_IRQ_Clear();

	halSendStart = 1; //��ʾ���Ϳ�ʼ��
	phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
	xcvseq = phyReg & (cPHY_CTRL1_XCVSEQ); //��ȡ��ǰʱ��
	phyReg &= ~(cPHY_CTRL1_XCVSEQ); //

	//��ǰ�Ƿ��ڽ���״̬
	if (gRX_c == xcvseq || gTR_c == xcvseq) {
		seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;
		if (seqState) { //��Ϊ0����ʾ��Ƶģ�鲻Ϊ����״̬

			if ((seqState <= 0x06) || (seqState == 0x15)
					|| (seqState == 0x16)) {
				//RXʱ�����ڽ������ݰ����������κ�ʱ�򣬵ȴ��������
				MCR20Drv_IRQ_Enable();
				return LRWPAN_STATUS_PHY_TX_START_FAILED;
			}

			//��ϵ�ǰ����ʱ��Ϊ������������ʱ����׼��
			phyReg |= gIdle_c;
			MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
			//�ȴ�ʱ����ֹ���
			while (MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F)
				;
		}
	}
	/*--------��װ����׼������----*/
	ptr = &tmpbuf[0];
	*ptr = phy_pib.currentTxFlen + PACKET_FOOTER_SIZE; //length does not include length byte
	ptr++;
	//copy frame
	memcpy(ptr, phy_pib.currentTxFrm, phy_pib.currentTxFlen);

	//д���ݵ���ƵоƬ
	MCR20Drv_PB_SPIBurstWrite(tmpbuf, phy_pib.currentTxFlen + 1);
	if (LRWPAN_GET_ACK_REQUEST(*(phy_pib.currentTxFrm))) {
		//��ҪACK��ֱ�Ӳ���TRʱ�򣬿�����Ӧ
		phyReg |= cPHY_CTRL1_RXACKRQD; //����ACK�ȴ�
		seqNext = gTR_c;
		macTxCallback();
	} else {
		/* ����TXʱ�� ������CCABFRTX*/
		seqNext = gTX_c;
		//��ͨ��TXʱ����TXʱ�������ŵ���phyTxEndCallBack
	}

	//�����Ƶģ���жϼĴ��������ͺ����в������ж϶�����Ҫ����
	mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
			MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
					&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 2);
	//���жϱ�־
	MCR20Drv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs, 3);

	phyReg |= seqNext;
	MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);

	//���ж�
	MCR20Drv_IRQ_Enable();

	return LRWPAN_STATUS_SUCCESS;
}
//AES-CTR��ʽ���ܺͽ�������
UINT8 halDataEncryptAndDecrypt(UINT8 key[16], UINT8 in[16], UINT8 cnt[16],
		UINT8 out[16]) {
	//ÿ���µļ��ܶ���Ҫ���ASM������
	MCR20A_ASM_Clear();
	//����CTR��������
	return MCR20A_CTR128_Encrypt(key, in, cnt, out);
}
