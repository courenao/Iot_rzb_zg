/**
 * ��������(������)��RZB���ڵ㣩->RZG��Э����/UDP�ͻ��ˣ�->BOX��UDP��������
 * ZigbeeͨѶ��ZigBee�������ӣ�Ӧ�ò����ط����������ݰ��ط����������ط���
 * UDPͨѶ�����������ӣ�Ӧ�ò㲻�ط�
 * �������ƣ�����ֻ�������ݷ���ʱ������������ͨѶʱ������������
 */

#include "msstate_lrwpan.h"
#include "stack_entry.h"
#include "board.h"
#ifdef USE_INTERNET
#include "netconf.h"
#include "lwip/udp.h"
#endif

/*-----------------------------ȫ�ֱ���----------------------*/
#define ADDR_ALL_DEVICES		0xFFFF		//�㲥�������豸�������ڵ㡢·������Э������
#define ADDR_ALL_ENDNOTES		0xFFFD		//�㲥�����нڵ㣨����Ҫ�ڵ���뵽����ŷ��ͣ�
#define ADDR_ALL_ROUTERS		0xFFFC		//�㲥������·����
LADDR_UNION dstADDR = { 0 }; //Ĭ��ΪЭ����
UINT32 my_timer;
UINT32 last_tx_start;
UINT8 app_started; //Ӧ�ó���������־��0--δ������1--�Ѿ�����
UINT32 app_startTime; //��ʱ500ms����
UINT32 heart_startTime; //��������ʼʱ��

#define ZIGBEE_DATA_STATIC_LEN		8//Zigbee_packed_t��ǰ8���ֽ�Ϊ�̶�����
#if DATA_ENCRYPTION_ENABLE
#define HEART_BEAT_LEN				16//�������ܺ�������С����Ӧ��Ϊ16�������������Ϊ16
#else
#define HEART_BEAT_LEN			ZIGBEE_DATA_STATIC_LEN	//���������ȣ���С���ݳ���
#endif

typedef struct {
	UINT8 id[4]; //��ǩID��
	UINT8 move[2]; //�ƶ�
	UINT8 rssi; //RSSI�ź�ǿ��
	UINT8 power; //������Ϣ
} Rfid_Data_t; //RFID���ݣ�8���ֽ�
#define RFID_PACK_COUNT_MAX		9	//���9������Ӧ��Zigbee_packed_t����Ϊ80
#define RFID_PACK_COUNT			9	//ʵ��PACK���������ܴ��������RFID_PACK_COUNT_MAX
typedef struct {
	UINT8 len; //���ݳ���
	UINT8 type; //�Ƿ���������,��4λΪ����,1--��������0--���ݰ�
	UINT8 id[3]; //zigbee�豸ID��
	UINT8 unUsed[3]; //���ã��������8�ֽڶ���
	Rfid_Data_t rfid[RFID_PACK_COUNT]; //�ڵ����ݰ����������9���ڵ�����
} Zigbee_packed_t;
static Zigbee_packed_t data_packed; //���������ݰ�
static UINT8 data_count = 0; //���������,

#if DATA_ENCRYPTION_ENABLE	//�������ݼ���
//�������ݳ���
const UINT8 AES_KEY[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
	0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
const UINT8 AES_CNT[16] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
	0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
#endif

#define TX_RETRY_COUNT 			3

//#define TX_TIMEOUT_MS  		2000 //����ʧ�ܳ�ʱ����λms
//#define IDLE_TIMEOUT_MS 		2000	//���ͳɹ��ļ��ʱ�䣬��λms����С����Ϊ20ms��С��20��Ч
//#define HEART_BEAT_TIMEOUT  	5000	//5000ms,��������ʱʱ��5s
Network_setting_t zigbee_sets; //��������������ʼ��ʱ����

//��ȡ�����������ݣ���㲥���͸����нڵ�0xFFFD
UINT8 zg_get_settings; //ZG UDP�Ƿ��ȡ���������������ݣ�0-���������ݣ�1--����������

//�ڵ��ȡ�������ò�������Ҫ��������״̬������״̬����λ��
UINT8 zigbee_get_settings; //�ڵ㣨zigbee�����յ��˲������ã�0-δ���յ���1--���յ�����Ҫ����
/*-----------------------------��������----------------------*/
void Zigbee_Data_Service(void);
void printJoinInfo(void);

/*-----------------------------��������----------------------*/

void Network_Config() {
	Network_setting_t *sets; //����ָ��
	Device_Info_t *ptr = Board_DeviceInfo_Get();
	sets = &(ptr->sets);

	if ((sets->type) & (TYPE_MASK_IDLE_TIME)) { //Flash�е�������Ч
		zigbee_sets.idle = sets->idle;
		zigbee_sets.type = sets->type; //�����������ͣ�ֻҪ��һ��������Ч�ͱ���
	} else { //Flash��������Ч
		zigbee_sets.idle = DEFAULT_VALUE_SET_IDLE; //����ȱʡֵ
	}

	if ((sets->type) & (TYPE_MASK_TX_TIMEOUT)) { //Flash�е�������Ч
		zigbee_sets.tx = sets->tx;
		zigbee_sets.type = sets->type; //�����������ͣ�ֻҪ��һ��������Ч�ͱ���
	} else { //Flash��������Ч
		zigbee_sets.tx = DEFAULT_VALUE_SET_TX; //����ȱʡֵ
	}

	if ((sets->type) & (TYPE_MASK_HEART_TIMEOUT)) { //Flash�е�������Ч
		zigbee_sets.heart = sets->heart;
		zigbee_sets.type = sets->type; //�����������ͣ�ֻҪ��һ��������Ч�ͱ���
	} else { //Flash��������Ч
		zigbee_sets.heart = DEFAULT_VALUE_SET_HEART; //����ȱʡֵ
	}

}

//��ʼ��ZigBee_Data��ر���ֵ��ÿ�η��ͳɹ�����Ҫ ����һ�Σ�ȷ���´η��͵���������
void Zigbee_Data_Init() {
	static UINT8 _once = 1;
	//������ݼ�����
	data_count = 0;

	//�����������
	data_packed.len = 0;
	data_packed.type = 0; //����λΪ0����ʾ����
#if !DATA_ENCRYPTION_ENABLE	//������ܿ��� �Ļ���ÿ�ζ���Ҫ���³�ʼ��data_packed������ֻ��Ҫִ��һ��
	if (_once)
#endif
	{
		UINT8 *mac = Board_Mac_Get_Ptr();
		//ȡMAC��ַ��ǰ6���ֽ���ΪID
		for (int i = 0; i < sizeof(data_packed.id); i++) {
			data_packed.id[i] = mac[i];
		}

		_once = 0;
	}
}


#ifdef LRWPAN_COORDINATOR//RZG��ΪЭ����
//����0--��ʾ�ɹ�����0--��ʾʧ��
UINT8 Zg_Data_Send() {
	BYTE len, *ptr;
	UINT8 ret;

	len = aplGetRxMsgLen();
	ptr = aplGetRxMsgData();

	//��̫����������
	struct udp_pcb *pcb = UDP_Client_Pcb_Get();
	struct pbuf *udpbuf;
#if DATA_ENCRYPTION_ENABLE	//�������ݼ���
	if(len>=16) { //���뱣֤���ݳ��ȴ��ڵ���16���ֽ�
		//ֻ����ǰ16���ֽ�
		halDataEncryptAndDecrypt(AES_KEY, ptr, AES_CNT, ptr);
	}
#endif
	udpbuf = pbuf_alloc(PBUF_RAW, len, PBUF_REF);
	if (NULL == udpbuf)
	return -1; //ERROR
	udpbuf->payload = ptr;

	ret = UDP_Target_Send(pcb, udpbuf);
	//udp_sendto(pcb, udpbuf,&ipAddrTarget,devInfo->udp.port);

	pbuf_free(udpbuf);

	Dprintf("Got len %d form NetId 0x%x\n", len,aplGetRxSrcSADDR());
	return ret;
}

void Zg_Heart_Send() {

	data_packed.len = HEART_BEAT_LEN; //����������
	data_packed.type = (1 << 4)|(1);//��4λΪ������־������λΪ1����4λΪZG��־

	//��̫����������
	struct udp_pcb *pcb = UDP_Client_Pcb_Get();
	struct pbuf *udpbuf;
	udpbuf = pbuf_alloc(PBUF_RAW, data_packed.len, PBUF_REF);
	if (NULL != udpbuf) {
#if DATA_ENCRYPTION_ENABLE	//�������ݼ���
		if (sizeof(data_packed) >= 16) {
			//ֻ����ǰ16�ֽڣ���������ƻ�ԭ�����ݿռ�
			halDataEncryptAndDecrypt(AES_KEY, &data_packed, AES_CNT,
					&data_packed);
		}
#endif
		udpbuf->payload = &data_packed;

		UDP_Target_Send(pcb, udpbuf);
		pbuf_free(udpbuf);

		Dprintf("ZG Send HeartBeat.\n");
	} else {
		//�ڴ�����ʧ�ܣ��쳣
	}
}

#else//RZB��Ϊ�ڵ�
//���ýڵ��������
void Zigbee_Recv_Settings() {
	BYTE len, *ptr;

	len = aplGetRxMsgLen();
	ptr = aplGetRxMsgData();
	if (len == sizeof(Network_setting_t)) { //��ȡ�������ò�������
		Network_setting_t *settings = ptr;

		//�����������
		Board_Network_Set(settings); //ʵ�ʲ�����Ҫ9183us����ʱ�ܳ���

		zigbee_get_settings = 1; //�ýڵ���յ��������ñ�־Ϊ1
	}
}

//Zigbee��������
UINT8 Zigbee_Heart_Beat() {
	UINT8 ret = 0; //����ֵ��0--δ���ͣ�>=1--�ѷ���
	UINT8 *mac;
	//Zigbee_data_t heartbeat;
	data_packed.len = HEART_BEAT_LEN; //����������
	data_packed.type = (1 << 4); //������

	if (halMACTimerNowDelta(heart_startTime) > zigbee_sets.heart) {

#if DATA_ENCRYPTION_ENABLE	//�������ݼ���
		if (sizeof(data_packed) >= 16) {
			//ֻ����ǰ16�ֽڣ���������ƻ�ԭ�����ݿռ�
			halDataEncryptAndDecrypt(AES_KEY, &data_packed, AES_CNT,
					&data_packed);
		}
#endif

		//������ʱʱ�䵽������һ֡������
		aplSendMSG(APS_DSTMODE_SHORT, &dstADDR, 1, //dst EP
				0, //cluster is ignored for direct message
				1, //src EP
				&data_packed, HEART_BEAT_LEN, //msg length
				apsGenTSN(), FALSE);
		ret = 1;
		Dprintf("No Data,Sending HeartBeat.\n");
	}
	return ret;
}

//���ݷ���
#define RFID_GETDATA 1

UINT8 Zigbee_Data_Send() {
	UINT8 ret = 0; //����ֵ��0--δ���ͣ�>=1--�ѷ���
	UINT8 len;
	//�������ݳ���
	len = data_packed.len;
	//�����ݷ�����ʼ����
	if (len) { //���ݳ���>0��ʶ������
#if DATA_ENCRYPTION_ENABLE	//�������ݼ���
	if (sizeof(data_packed) >= 16) {
		//ֻ����ǰ16�ֽڣ���������ƻ�ԭ�����ݿռ�
		halDataEncryptAndDecrypt(AES_KEY, &data_packed, AES_CNT,
				&data_packed);
	}
#endif

		aplSendMSG(APS_DSTMODE_SHORT, &dstADDR, 1, //dst EP
				0, //cluster is ignored for direct message
				1, //src EP
				&data_packed, len, //msg length
				apsGenTSN(), FALSE);

		Dprintf("Send Data len=%d\n", len);
		ret = 1;
	} else { //�������򲻷�������
		ret = Zigbee_Heart_Beat();
	}
	return ret;
}
static int rfid_count=0;
//ZigBee�������״̬�����RFID���ݲ�����RFID���
UINT8 Zigbee_Idle_Service() {
	UINT8 ret = 0;
	//RFIDһ�����ݰ������÷�������ʽ��ȡ���ݣ�һ�����ڿ���ֻ������һ������ݣ���Ҫ��̬���棩
	static Rfid_Data_t rfid;
#if !RFID_GETDATA
	//��ʵ���
	UINT16 bytes;
	//��������ʽ��ȡ���ݣ����ջ�������Ǿ�̬��
	bytes=Board_Rfid_Nowait_Gets(&rfid,sizeof(rfid));

	if(bytes==sizeof(rfid))
#else
	if (RFID_GETDATA)
#endif
	{
		rfid.id[0]=rfid_count++;
		//���data_count����RFID_PACK_COUNT�����ʾ�ϴδ���õ����ݻ�û���ͣ���β�����
		if (data_count < RFID_PACK_COUNT) {
			//���ݳ���Ϊ����ǰ5���ֽڹ̶�����+RFID���ݰ�����*һ��RFID���ݰ�����
			data_packed.len = ZIGBEE_DATA_STATIC_LEN
					+ sizeof(Rfid_Data_t) * (data_count + 1);

			//��������
			halUtilMemCopy(&(data_packed.rfid[data_count]), &rfid,
					sizeof(Rfid_Data_t));

			data_count++; //����������

			//RFID���ݰ����ﵽ���������RFID_PACK_COUNT=9����һ����������
			if (data_count == RFID_PACK_COUNT) {
				data_count = 0;
				ret = 1; //����ﵽ�����ֵ����Ҫ��������
			}
		}
	}
	return ret;
}

#endif//endof LRWPAN_COORDINATOR
//Э��ջά��
void Zigbee_Data_Service() {
	typedef enum {
		ENUM_TX_START = 0,
		ENUM_TX_WAIT,
		ENUM_RX_START,
		ENUM_RX_WAIT,
		ENUM_RADIO_IDLE
	} State_t;
	State_t state = ENUM_TX_START;
	uint32_t idle_time; //��¼��ʼ�������״̬��ʱ��
	uint32_t tx_time;
	uint8_t tx_retry; //�������Լ���

	heart_startTime = 0; //��ʼ��������ʱΪ0

	tx_retry = TX_RETRY_COUNT;

	//��ʼ������
	Zigbee_Data_Init();


	while (1) {
		apsFSM(); //wait for finish

#ifdef LRWPAN_COORDINATOR
		//Э����ά����̫��
		//LwIP_Periodic_Handle(GetTimeMS());//������ջ�dhcp
		//��̫�����ط�����
		if ((halMACTimerNowDelta(heart_startTime)) > zigbee_sets.heart) {
			Zg_Heart_Send(); //����UDP��������
			heart_startTime = halGetMACTimer();
		}
		if (zg_get_settings) { //��������������Ҫ����
			zg_get_settings = 0;//�����־
			dstADDR.saddr = ADDR_ALL_DEVICES;//ָ���ַΪ���нڵ㣬�㲥����

			aplSendMSG(APS_DSTMODE_SHORT, &dstADDR, 1,//dst EP
					0,//��Ч
					1,//src EP
					&zigbee_sets, sizeof(zigbee_sets),//�����òη���
					apsGenTSN(), FALSE);
		}

#else

		//�ڵ����ò�����ǿ���˳��ϴη��ͣ����½���TX_START״̬
		if (zigbee_get_settings) {
			state = ENUM_TX_START; //ǿ������״̬�������½����ʼ״̬
			zigbee_get_settings = 0; //�����־
		}

		switch (state) {
		case ENUM_TX_START: //����״̬
			//��ӷ�������
			if (Zigbee_Data_Send() == 0) { //δ�����κ����ݷ��ͣ����½��뷢��״̬
				//state = ENUM_TX_START; //���½��뷢��״̬
				break;//���¶�������
			}

			tx_time = halGetMACTimer();
			state = ENUM_TX_WAIT;
			break;
		case ENUM_TX_WAIT:
			if (apsBusy()) {
				break; //���ͻ�û��ɣ�ֱ���˳�
			}
			if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
				//���ͳɹ�
				state = ENUM_RADIO_IDLE; //�������״̬���ȴ�һ��ʱ������·���
				//���м�ʱ��ʼ
				idle_time = halGetMACTimer();

				//������������ʱ�����������ݻ����������ͣ�ֻҪ�ɹ�����������������ʱ��
				heart_startTime = halGetMACTimer();

				tx_retry = TX_RETRY_COUNT;

				Zigbee_Data_Init(); //���ͳɹ�����ԭ����Ϊ��ʼֵ
			} else { //����ʧ�ܣ����볬ʱ�ж�
				if ((halMACTimerNowDelta(tx_time)) > zigbee_sets.tx) {
					if (tx_retry > 0) {
						tx_retry--;
						Dprintf("Send Retry.\n");
						state = ENUM_TX_START; //���½��뷢��״̬
					}
					if (0 == tx_retry) { //�ط������þ�
						//������������Ĭ��3��
						if (Board_PWM_IsOff()) {
							Board_PWM_SetCount(3);
							Board_PWM_On();
						}
						return; //�����쳣���˳�������������������������״̬
					}

				}

			}

			break;
		case ENUM_RADIO_IDLE:
			if ((halMACTimerNowDelta(idle_time)) > zigbee_sets.idle) { //2s���г�ʱ
				state = ENUM_TX_START;
			} else {
				//����״̬������RFID�������

				if (Zigbee_Idle_Service()) { //�����ݰ���Ҫ���ͣ����˳�����״̬�����뷢��״̬
#if RFID_GETDATA	//����ʹ�ã���ʵ��������
					if ((halMACTimerNowDelta(idle_time))
							> (zigbee_sets.idle / 10)) //����1/10���������͵�ǰ��������ݰ�
#endif
						state = ENUM_TX_START;
				}
			}
			break;
		default:
			break;
		}

#endif //endof #ifdef LRWPAN_COORDINATOR
	}
}

void printJoinInfo(void) {

	Dprintf(
			"�����̵�ַ: 0x%X�����ڵ�̵�ַ��0x%X\n", aplGetMyShortAddress(), aplGetParentShortAddress());
	if (debugFlag) {
		BYTE *ptr = aplGetParentLongAddress();
		Dprintf("���ڵ㳤��ַ�� ");
		for (uint8_t i = 0; i < 8; i++) {
			Dprintf("%02X", ptr[i]);
		}
		Dprintf("\n");
	}
}

typedef enum _JOIN_STATE_ENUM {
	JOIN_STATE_START_JOIN,
	JOIN_STATE_WAIT_FOR_JOIN,
	JOIN_STATE_RUN_APP1,
	JOIN_STATE_RUN_APP2,
	JOIN_STATE_RUN_APP3,
	JOIN_STATE_START_REJOIN,
	JOIN_STATE_WAIT_FOR_REJOIN,
} JOIN_STATE_ENUM;

JOIN_STATE_ENUM joinState;

#define MAX_REJOIN_FAILURES 3 //�����������
//����������
void stack_main(void) {

	UINT8 failures;

	my_timer = 0;
	halInit(); //��ʼ����ƵӲ��
	aplInit(); //init the stack

	//conPrintConfig();

	Network_Config(); //�����������

	EVB_LED1_OFF();
	EVB_LED2_OFF();

	joinState = JOIN_STATE_START_JOIN; //main while(1) initial state

	//debug_level = 10;

	while (1) {
		apsFSM();

		switch (joinState) {

		case JOIN_STATE_START_JOIN:
			EVB_LED1_OFF();
#ifdef LRWPAN_COORDINATOR
			aplFormNetwork()
			;
#else
			aplJoinNetwork();
#endif
			joinState = JOIN_STATE_WAIT_FOR_JOIN;
			break;
		case JOIN_STATE_WAIT_FOR_JOIN:
			if (apsBusy())
				break; //if stack is busy, continue
#ifdef LRWPAN_COORDINATOR
				if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
					printf("���罨����ɡ�\n");
					EVB_LED1_ON(); //turn on LED1 indicating network is formed.
					joinState = JOIN_STATE_RUN_APP1;
				} else {
					//only reason for this to fail is some sort of hardware failure
					printf("���罨��ʧ�ܣ��ȴ�2s֮�����ԣ�\n");
					delayMS(2000);
					joinState = JOIN_STATE_START_JOIN;
				}
#else
			if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
				EVB_LED1_ON();
				printf("��������ɹ���\n");
				printJoinInfo();
				joinState = JOIN_STATE_RUN_APP1;
			} else {
				printf("��������ʧ�ܣ��ȴ�10s֮�����ԣ�\n");
				//����������,2��
				if (Board_PWM_IsOff()) {
					Board_PWM_SetCount(2);
					Board_PWM_On();
				}
				delayMS(10 * 1000);
				joinState = JOIN_STATE_START_JOIN;
			}
#endif

			break;

		case JOIN_STATE_RUN_APP1:
#ifdef LRWPAN_COORDINATOR

			Zigbee_Data_Service(); //�û��������

#else

			Dprintf("��ʼ����Ӧ�ó���!\n");
			joinState = JOIN_STATE_RUN_APP2;
			if (app_started == 0) {
				//����app��ʱ������500ms���Զ���ʼִ��UserApp
				app_startTime = halGetMACTimer();
				app_started = 1;
			}
#endif
			break;
		case JOIN_STATE_RUN_APP2: //�ӳ�һ��ʱ���������ڳ��򣬵ȴ������ȶ�
			//��ӷ��ʹ������ݷ��͵������������ǵȴ�200ms
			if ((halMACTimerNowDelta(app_startTime)) < 200) {
				break; //ʱ��δ�����˳����µȴ�
			} else {
				//û��break;���Կ���ֱ�ӽ�����һ��״̬
				app_started = 0; //���������־��Ϊ�´μ�ʱ��׼��
			}
		case JOIN_STATE_RUN_APP3:
			//switch is pressed, run app
			dstADDR.saddr = 0; //RFD sends to the coordinator
			my_timer = halGetMACTimer(); //timer must be initialized before entering PP_SEND_STATE
			Zigbee_Data_Service(); //only exits on if excessive misses
			//try rejoining network
			failures = 0;
			joinState = JOIN_STATE_START_REJOIN;
			break;
			//rejoin states only executed by RFD
#ifndef LRWPAN_COORDINATOR
		case JOIN_STATE_START_REJOIN:
			EVB_LED1_OFF();
			printf("�������¼�������!\n");
			aplRejoinNetwork();
			joinState = JOIN_STATE_WAIT_FOR_REJOIN;
			break;

		case JOIN_STATE_WAIT_FOR_REJOIN:
			if (apsBusy())
				break; //if stack is busy, continue
			if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
				failures = 0;
				EVB_LED1_ON();
				printf("���������ɹ���\n");
				printJoinInfo();
				joinState = JOIN_STATE_RUN_APP3; //don't wait for button press
			} else {
				failures++;
				if (failures == MAX_REJOIN_FAILURES) {
					//this starts everything over
					//conPrintROMString(" Max Rejoins failed, trying to join.\n");
					joinState = JOIN_STATE_START_JOIN;
				} else {
					//else, wait to try again
					printf("��������ʧ�ܣ��ȴ�2s���³��ԡ�\n");
					my_timer = halGetMACTimer();
					//wait for 2 seconds
					while ((halMACTimerNowDelta(my_timer))
							< MSECS_TO_MACTICKS(2 * 1000))
						;

					joinState = JOIN_STATE_START_REJOIN;
				}
			}
			break;
#endif

		default:
			joinState = JOIN_STATE_START_JOIN;

		}

	}
}

//########## Callbacks ##########

//callback for anytime the Zero Endpoint RX handles a command
//user can use the APS functions to access the arguments
//and take additional action is desired.
//the callback occurs after the ZEP has already taken
//its action.
LRWPAN_STATUS_ENUM usrZepRxCallback(void) {

#ifdef LRWPAN_COORDINATOR
	if (aplGetRxCluster() == ZEP_END_DEVICE_ANNOUNCE) {
		//a new end device has announced itself, print out the
		//the neightbor table and address map
		DEBUG_PRINTNEIGHBORS(DBG_INFO);
	}
#endif
	return LRWPAN_STATUS_SUCCESS;
}

//callback from APS when packet is received
//user must do something with data as it is freed
//within the stack upon return.

LRWPAN_STATUS_ENUM usrRxPacketCallback(void) {

#ifdef LRWPAN_COORDINATOR
	//UDP��������
	Zg_Data_Send();
	heart_startTime = halGetMACTimer();

#else//�ڵ���ղ�������
	Zigbee_Recv_Settings();

#endif

	return LRWPAN_STATUS_SUCCESS;
}

#if 1//#ifdef LRWPAN_FFD
//Callback to user level to see if OK for this node
//to join - implement Access Control Lists here based
//upon IEEE address if desired
//�ڵ���֤��ptrΪ��ǰ���������Ľڵ������ַ��capinfoΪ�豸������Ϣ
//������Ҫ�û��Լ���ӽڵ���˴���
#if STATIC_LADDR_MARK	//��̬���鶨��ķ���
static const LADDR node_list[] = {

	{	0x01, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x02, 0x21, 0x01,
		0x00, 0x00, 0x4B, 0x12, 0x00}, {0x03, 0x21, 0x01, 0x00, 0x00,
		0x4B, 0x12, 0x00}, {0x04, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12,
		0x00}, {0x05, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {
		0x06, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x07, 0x21,
		0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x08, 0x21, 0x01, 0x00,
		0x00, 0x4B, 0x12, 0x00}, {0x09, 0x21, 0x01, 0x00, 0x00, 0x4B,
		0x12, 0x00},
	{	0x0a, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x0b, 0x21, 0x01,
		0x00, 0x00, 0x4B, 0x12, 0x00}, {0x0c, 0x21, 0x01, 0x00, 0x00,
		0x4B, 0x12, 0x00}, {0x0d, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12,
		0x00}, {0x0e, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {
		0x0f, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x10, 0x21,
		0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x11, 0x21, 0x01, 0x00,
		0x00, 0x4B, 0x12, 0x00}, {0x12, 0x21, 0x01, 0x00, 0x00, 0x4B,
		0x12, 0x00},
	{	0x13, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x14, 0x21, 0x01,
		0x00, 0x00, 0x4B, 0x12, 0x00}, {0x15, 0x21, 0x01, 0x00, 0x00,
		0x4B, 0x12, 0x00}};
BOOL checkLongAddr(LADDR *ptr)
{
	for (UINT8 i = 0; i < (sizeof(node_list) / sizeof(LADDR)); i++) {
		for (UINT8 j = 0; j < sizeof(LADDR); j++) {
			if (ptr->bytes[j] != node_list[i].bytes[j])
			break;

			if (j == (sizeof(LADDR) - 1)) {
				return TRUE; //ֱ���˳�������Ϊ��
			}
		}
	}
	return FALSE;
}
#else	//��ַ����ķ�ʽ���з�Χ���ˣ������ڷ�Χ�ڵĶ�ͨ��
//B0    B1    B2    B3    B4    B5    B6    B7
//����,Ŀǰ�ֽ�Ϊ��λ����0--��ʾ�����ģ�1--��ʾ��Ҫ��֤
const BYTE LADDR_MASK[8] = { 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff };
//����
const BYTE LADDR_SAMP[8] = { 0x0, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00 };

BOOL checkLongAddr(LADDR *ptr) {
	for (BYTE i = 0; i < 8; ++i) {
#if 0	//ʹ����������У��ƥ��
		if ((LADDR_MASK[i] & (ptr->bytes[i]))
				!= (LADDR_SAMP[i] & LADDR_MASK[i])) {
			return FALSE; //ʧ��
		}
#else	//ʹ��Э�����Լ���MAC��ַ����У��
		uint8_t *addr = Board_Mac_Get_Ptr();
		if ((LADDR_MASK[i] & (ptr->bytes[i])) != (addr[i] & LADDR_MASK[i])) {
			return FALSE; //ʧ��
		}
#endif
		if (i == 8)
			return TRUE; //�ɹ�
	}

}
#endif //endof STATIC_LADDR_MARK
BOOL usrJoinVerifyCallback(LADDR *ptr, BYTE capinfo) {

	BOOL ret;
	ret = checkLongAddr(ptr);
	if (ret == FALSE) { //�����ӡ
		conPrintROMString("Illegal Device ");
		conPrintLADDR(ptr);
		conPCRLF();
	}
	return ret;
}

BOOL usrJoinNotifyCallback(LADDR *ptr) {

	//allow anybody to join

	conPrintROMString("Node joined: ");
	conPrintLADDR(ptr);
	conPCRLF();
	DEBUG_PRINTNEIGHBORS(DBG_INFO);
	return TRUE;
}
#endif

//called when the slow timer interrupt occurs
#ifdef LRWPAN_ENABLE_SLOW_TIMER
void usrSlowTimerInt(void) {
}
#endif

//general interrupt callback , when this is called depends on the HAL layer.
void usrIntCallback(void) {
}
