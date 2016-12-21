/*******************************************************************************
 * @�ļ�    MCR20Drv.c
 * @����  LiXianmao
 * @�汾 V0.0.1
 * @����    2016.9.13
 * @����   mcr20a�豸�ڲ��Ĵ�����д���������ļ�
 * @��ע
 *******************************************************************************/

/*! *********************************************************************************
 *************************************************************************************
 * Include
 *************************************************************************************
 ********************************************************************************** */
#include "MCR20Drv.h"
#include "MCR20Reg.h"
#include "MCR20A_port.h"

#ifndef NULL
#define NULL (0)
#endif

#ifndef OSA_EnterCritical
#define OSA_EnterCritical(x);   ;
#endif

#ifndef OSA_ExitCritical
#define OSA_ExitCritical(x);	;
#endif

#ifndef kCriticalDisableInt
#define kCriticalDisableInt 0
#endif

/*! *********************************************************************************
 *************************************************************************************
 * Private macros
 *************************************************************************************
 ********************************************************************************** */
#define mMCR20SpiWriteSpeed_c (16000000)
#define mMCR20SpiReadSpeed_c   (8000000)

#define gXcvrSpiInstance_c 0u
#define kGpioXcvrIrq 0

/*! *********************************************************************************
 *************************************************************************************
 * Private variables
 *************************************************************************************
 ********************************************************************************** */
static uint32_t mPhyIrqDisableCnt = 1;
static uint8_t mXcvrPin;

/*! *********************************************************************************
 *************************************************************************************
 * Public functions
 *************************************************************************************
 ********************************************************************************** */

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_Init
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_Init(void) {
	//���ó�Masterģʽ,��ʼ�������Ѿ���mMCR20SpiReadSpeed_c,CS�ź�Ϊ��
	spi_master_init(0);
	//��������mMCR20SpiReadSpeed_c
	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiReadSpeed_c);
	//��ʼ����λ����
	MCR20A_RST_Init();

#if (1|| RST_REQ) //��ƵоƬӲ��RST������MCU��RST����Ӳ���Ѿ�������һ�𣬲���Ҫ���������Ƶģ�鵥����λ
	//��λ��Ƶģ��
	MCR20Drv_RESET();
#endif

	//��ʼ���ж�����
	MCR20A_IRQ_Init();

#ifdef MCR20A_MCU_RNG_ENABLE
	MCR20A_MCU_RNG_Init();
#endif
	//����CS
	//SPI1_CS_HIGH();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_DirectAccessSPIWrite
 * Description: ֱ��д�Ĵ��������ֽڣ�
 * Parameters: uint8_t address-�Ĵ�����ַ��uint8_t value-ֵ
 * Return: ��
 *---------------------------------------------------------------------------*/
void MCR20Drv_DirectAccessSPIWrite(uint8_t address, uint8_t value) {
	uint16_t txData;

	ProtectFromMCR20Interrupt();
	//����SPIд���ʿ���ߵ�16MHz
	spi_master_configure_speed(0, mMCR20SpiWriteSpeed_c);
	//CS����Ϊ�͵�ƽ
	spi_cs_enable();
	//������bit7��bit6λ0�����mask=0x3f
	txData = (address & TransceiverSPI_DirectRegisterAddressMask);
	txData |= value << 8;

	spi_master_transfer(gXcvrSpiInstance_c, (uint8_t *) &txData, NULL,
			sizeof(txData));
	//CS����
	spi_cs_disable();
	UnprotectFromMCR20Interrupt();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_DirectAccessSPIMultiByteWrite
 * Description: ֱ��д�Ĵ��������ֽڣ�
 * Parameters: uint8_t startAddress-��ʼ��ַ��uint8_t * byteArray-�����׵�ַ
 *			  uint8_t numOfBytes-�ֽ���
 * Return: ��
 *---------------------------------------------------------------------------*/
void MCR20Drv_DirectAccessSPIMultiByteWrite(uint8_t startAddress,
		uint8_t * byteArray, uint8_t numOfBytes) {
	uint8_t txData = 0;

	if ((numOfBytes == 0) || (byteArray == NULL)) {
		return;
	}

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(0, mMCR20SpiWriteSpeed_c);

	spi_cs_enable();
	//������bit7��bit6λ0�����mask=0x3f
	txData = (startAddress & TransceiverSPI_DirectRegisterAddressMask);

	spi_master_transfer(gXcvrSpiInstance_c, &txData, NULL, sizeof(txData));
	spi_master_transfer(gXcvrSpiInstance_c, byteArray, NULL, numOfBytes);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_PB_SPIByteWrite
 * Description: Packet Bufferд�Ĵ��������ֽڣ�
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_PB_SPIByteWrite(uint8_t address, uint8_t value) {
	uint32_t txData = 0;

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiWriteSpeed_c);

	spi_cs_enable();

	//������:bit7=0��ʾд,bit6=1��ʾpackbuffer,bit5=1��ʾ���ֽ�,bit0~4��ʹ��
	//����������=0x60
	txData = TransceiverSPI_WriteSelect | TransceiverSPI_PacketBuffAccessSelect
			| TransceiverSPI_PacketBuffByteModeSelect;
	//�ڶ�����Ϊ��ַ
	txData |= (address) << 8;
	//��������Ϊ����ֵ
	txData |= (value) << 16;

	spi_master_transfer(gXcvrSpiInstance_c, (uint8_t*) &txData, NULL, 3);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_PB_SPIBurstWrite
 * Description: PacketBufferд����
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_PB_SPIBurstWrite(uint8_t * byteArray, uint8_t numOfBytes) {
	uint8_t txData;

	if ((numOfBytes == 0) || (byteArray == NULL)) {
		return;
	}

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiWriteSpeed_c);

	spi_cs_enable();

	//������:bit7=0��ʾд,bit6=1��ʾpackbuffer,bit5=0��ʾ���ֽ�,bit0~4��ʹ��
	txData = TransceiverSPI_WriteSelect | TransceiverSPI_PacketBuffAccessSelect
			| TransceiverSPI_PacketBuffBurstModeSelect;

	spi_master_transfer(gXcvrSpiInstance_c, &txData, NULL, 1);
	spi_master_transfer(gXcvrSpiInstance_c, byteArray, NULL, numOfBytes);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_DirectAccessSPIRead
 * Description: ֱ�Ӷ��Ĵ��������ֽڣ�
 * Parameters: uint8_t address-�Ĵ�����ַ
 * Return: ����uint8_t�Ĵ�����ֵ
 *---------------------------------------------------------------------------*/

uint8_t MCR20Drv_DirectAccessSPIRead(uint8_t address) {
	uint8_t txData;
	uint8_t rxData;

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiReadSpeed_c);

	spi_cs_enable();
	//�����֣�bit7=1��ʾ����bit6=0��ʾֱ�Ӳ����Ĵ���
	txData = (address & TransceiverSPI_DirectRegisterAddressMask)
			| TransceiverSPI_ReadSelect;

	spi_master_transfer(gXcvrSpiInstance_c, &txData, NULL, sizeof(txData));
	spi_master_transfer(gXcvrSpiInstance_c, NULL, &rxData, sizeof(rxData));

	spi_cs_disable();

	UnprotectFromMCR20Interrupt();

	return rxData;

}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_DirectAccessSPIMultyByteRead
 * Description: ֱ�Ӷ��Ĵ��������ֽڣ�
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t MCR20Drv_DirectAccessSPIMultiByteRead(uint8_t startAddress,
		uint8_t * byteArray, uint8_t numOfBytes) {
	uint8_t txData;
	uint8_t phyIRQSTS1;

	if ((numOfBytes == 0) || (byteArray == NULL)) {
		return 0;
	}

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiReadSpeed_c);

	spi_cs_enable();
	//������bit7=1��ʾ����bit=6
	txData = (startAddress & TransceiverSPI_DirectRegisterAddressMask)
			| TransceiverSPI_ReadSelect;

	spi_master_transfer(gXcvrSpiInstance_c, &txData, &phyIRQSTS1,
			sizeof(txData));
	spi_master_transfer(gXcvrSpiInstance_c, NULL, byteArray, numOfBytes);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();

	return phyIRQSTS1;
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_PB_SPIBurstRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t MCR20Drv_PB_SPIBurstRead(uint8_t * byteArray, uint8_t numOfBytes) {
	uint8_t txData;
	uint8_t phyIRQSTS1;

	if ((numOfBytes == 0) || (byteArray == NULL)) {
		return 0;
	}

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiReadSpeed_c);

	spi_cs_enable();
	//������ bit7=1,bit6=1,bit5=0,���ౣ��
	txData = TransceiverSPI_ReadSelect | TransceiverSPI_PacketBuffAccessSelect
			| TransceiverSPI_PacketBuffBurstModeSelect;
	//phyIRQSTS1 MCR20RM.PDF 8.3.9 ÿ�ο�����д��ʱ���᷵��IRQSTS1
	spi_master_transfer(gXcvrSpiInstance_c, &txData, &phyIRQSTS1,
			sizeof(txData));
	spi_master_transfer(gXcvrSpiInstance_c, NULL, byteArray, numOfBytes);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();

	return phyIRQSTS1;
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IndirectAccessSPIWrite
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_IndirectAccessSPIWrite(uint8_t address, uint8_t value) {
	uint32_t txData;

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiWriteSpeed_c);

	spi_cs_enable();
	//���д�Ĵ�����bit7=0,bit6=0,bit1~5=1,bit0=0����0x3e
	txData = TransceiverSPI_IARIndexReg;
	txData |= (address) << 8;
	txData |= (value) << 16;

	spi_master_transfer(gXcvrSpiInstance_c, (uint8_t*) &txData, NULL, 3);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IndirectAccessSPIMultiByteWrite
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_IndirectAccessSPIMultiByteWrite(uint8_t startAddress,
		uint8_t * byteArray, uint8_t numOfBytes) {
	uint16_t txData;

	if ((numOfBytes == 0) || (byteArray == NULL)) {
		return;
	}

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiWriteSpeed_c);

	spi_cs_enable();
	//���д�Ĵ�����bit7=0,bit6=0,bit1~5=1,bit0=0����0x3e
	txData = TransceiverSPI_IARIndexReg;
	txData |= (startAddress) << 8;

	spi_master_transfer(gXcvrSpiInstance_c, (uint8_t*) &txData, NULL,
			sizeof(txData));
	spi_master_transfer(gXcvrSpiInstance_c, (uint8_t*) byteArray, NULL,
			numOfBytes);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IndirectAccessSPIRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t MCR20Drv_IndirectAccessSPIRead(uint8_t address) {
	uint16_t txData;
	uint8_t rxData;

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiReadSpeed_c);

	spi_cs_enable();
	//
	txData = TransceiverSPI_IARIndexReg | TransceiverSPI_ReadSelect;
	txData |= (address) << 8;

	spi_master_transfer(gXcvrSpiInstance_c, (uint8_t*) &txData, NULL,
			sizeof(txData));
	spi_master_transfer(gXcvrSpiInstance_c, NULL, &rxData, sizeof(rxData));

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();

	return rxData;
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IndirectAccessSPIMultiByteRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_IndirectAccessSPIMultiByteRead(uint8_t startAddress,
		uint8_t * byteArray, uint8_t numOfBytes) {
	uint16_t txData;

	if ((numOfBytes == 0) || (byteArray == NULL)) {
		return;
	}

	ProtectFromMCR20Interrupt();

	spi_master_configure_speed(gXcvrSpiInstance_c, mMCR20SpiReadSpeed_c);

	spi_cs_enable();

	txData = (TransceiverSPI_IARIndexReg | TransceiverSPI_ReadSelect);
	txData |= (startAddress) << 8;

	spi_master_transfer(gXcvrSpiInstance_c, (uint8_t*) &txData, NULL,
			sizeof(txData));
	spi_master_transfer(gXcvrSpiInstance_c, NULL, byteArray, numOfBytes);

	spi_cs_disable();
	UnprotectFromMCR20Interrupt();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IRQ_PortConfig
 * Description: MCR20�ж���������,��ʼ���������Ѿ������ù���
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_IRQ_PortConfig(void) {

}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IsIrqPending
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint32_t MCR20Drv_IsIrqPending(void) {
#if 0
	if( !EXTI_GetITStatus(MCR20_IRQ_LINE) )
	{
		return TRUE;
	}
#endif
	return FALSE;
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IRQ_Disable
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_IRQ_Disable(void) {
#if 1

	if (mPhyIrqDisableCnt == 0) {
		MCR20A_IRQ_Disable();
	}

	mPhyIrqDisableCnt++;

#else
	MCR20A_IRQ_Disable();
#endif
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IRQ_Enable
 * Description: ʹ��MCR20A�豸�ж�
 * Parameters: IRQ_ZIG ����PA1
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_IRQ_Enable(void) {
#if 1

	if (mPhyIrqDisableCnt) {
		mPhyIrqDisableCnt--;

		if (mPhyIrqDisableCnt == 0) {
			MCR20A_IRQ_Enable();
		}
	}

#else
	MCR20A_IRQ_Enable();
#endif

}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IRQ_IsEnabled
 * Description: Ӳ���ж�״̬
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint32_t MCR20Drv_IRQ_IsEnabled(void) {
	return 0;
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_IRQ_Clear
 * Description: ���MCR20�жϱ�־
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_IRQ_Clear(void) {
	MCR20A_IRQ_CLEAR();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_RST_Assert
 * Description: ��λ��Ч���͵�ƽ��
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_RST_B_Assert(void) {
	MCR20A_RESET_ASSERT();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_RST_Deassert
 * Description: ��λ������ߵ�ƽ��
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_RST_B_Deassert(void) {
	MCR20A_RESET_DEASSERT();
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_SoftRST_Assert
 * Description: �����λ��Ч
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_SoftRST_Assert(void) {
	MCR20Drv_IndirectAccessSPIWrite(SOFT_RESET, (0x80));
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_SoftRST_Deassert
 * Description: �����λ���
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_SoftRST_Deassert(void) {
	MCR20Drv_IndirectAccessSPIWrite(SOFT_RESET, (0x00));
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_Soft_RESET
 * Description: �����λ������Ч->������̣�
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_Soft_RESET(void) {
	//assert SOG_RST
	MCR20Drv_IndirectAccessSPIWrite(SOFT_RESET, (0x80));

	//deassert SOG_RST
	MCR20Drv_IndirectAccessSPIWrite(SOFT_RESET, (0x00));
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_RESET
 * Description: Ӳ����λ������Ч->����Ĺ��̣�
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_RESET(void) {
	volatile uint32_t delay = 3000;
	//assert RST_B
	MCR20Drv_RST_B_Assert();
	//�ȴ�һ��ʱ��
	while (delay--) {
	}

	//deassert RST_B
	MCR20Drv_RST_B_Deassert();
	delay = 3000;
	//�ȴ�һ��ʱ��ȷ��оƬ��λ���
	while (delay--) {
	}
}

/*---------------------------------------------------------------------------
 * Name: MCR20Drv_Set_CLK_OUT_Freq
 * Description: ����MCR20A���ʱ��Ƶ��
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MCR20Drv_Set_CLK_OUT_Freq(uint8_t freqDiv) {
	uint8_t clkOutCtrlReg = (freqDiv & cCLK_OUT_DIV_Mask) | cCLK_OUT_EN
			| cCLK_OUT_EXTEND;

	if (freqDiv == gCLK_OUT_FREQ_DISABLE) {
		clkOutCtrlReg = (cCLK_OUT_EXTEND | gCLK_OUT_FREQ_4_MHz); //reset value with clock out disabled
	}

	MCR20Drv_DirectAccessSPIWrite((uint8_t) CLK_OUT_CTRL, clkOutCtrlReg);
}

/**************************************************************************************************/
/*                                        XCVR GPIO API                                           */
/**************************************************************************************************/

/*! *********************************************************************************
 * \brief  Set the XCVR GPIO pin value to logic 1 (one)
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 * \remarks Use only if the pin is set up as output
 *
 ********************************************************************************** */
void MCR20Drv_SetGpioPin(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_DATA);
	phyReg |= GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_DATA, phyReg);
}

/*! *********************************************************************************
 * \brief  Set the XCVR GPIO pin value to logic 0 (zero)
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 * \remarks Use only if the pin is set up as output
 *
 ********************************************************************************** */
void MCR20Drv_ClearGpioPin(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_DATA);
	phyReg &= ~GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_DATA, phyReg);
}

/*! *********************************************************************************
 * \brief  Read the XCVR GPIO pin value
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 * \return  the value of the requested GPIOs:
 *          bit0 -> value of GPIO1, ... bit7 -> value of GPIO8
 *
 * \remarks Use only if the pin is set up as input
 *
 ********************************************************************************** */
uint8_t MCR20Drv_GetGpioPinValue(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_DATA);
	return (phyReg & GpioMask);
}

/*! *********************************************************************************
 * \brief  Configure the XCVR GPIO pin as output pin
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 ********************************************************************************** */
void MCR20Drv_SetGpioPinAsOutput(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_DIR);
	phyReg |= GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_DIR, phyReg);
}

/*! *********************************************************************************
 * \brief  Configure the XCVR GPIO pin as input pin
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 ********************************************************************************** */
void MCR20Drv_SetGpioPinAsInput(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_DIR);
	phyReg &= ~GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_DIR, phyReg);
}

/*! *********************************************************************************
 * \brief  Enable XCVR GPIO pin PullUp resistor
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 ********************************************************************************** */
void MCR20Drv_EnableGpioPullUp(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_PUL_SEL);
	phyReg |= GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_PUL_SEL, phyReg);

	phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_PUL_EN);
	phyReg |= GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_PUL_EN, phyReg);
}

/*! *********************************************************************************
 * \brief  Enable XCVR GPIO pin PullDown resistor
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 ********************************************************************************** */
void MCR20Drv_EnableGpioPullDown(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_PUL_SEL);
	phyReg &= ~GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_PUL_SEL, phyReg);

	phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_PUL_EN);
	phyReg |= GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_PUL_EN, phyReg);
}

/*! *********************************************************************************
 * \brief  Disable XCVR GPIO pin PullUp/PullDown resistors
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 ********************************************************************************** */
void MCR20Drv_DisableGpioPullUpDown(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_PUL_EN);
	phyReg &= ~GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_PUL_EN, phyReg);
}

/*! *********************************************************************************
 * \brief  Enable hi drive strength on XCVR GPIO pin
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 ********************************************************************************** */
void MCR20Drv_EnableGpioHiDriveStrength(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_DS);
	phyReg &= ~GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_DS, phyReg);
}

/*! *********************************************************************************
 * \brief  Disable hi drive strength on XCVR GPIO pin
 *
 * \param[in]  GpioMask  bitmask of GPIOs: bit0 -> GPIO1, ..., bit7 -> GPIO8
 *
 ********************************************************************************** */
void MCR20Drv_DisableGpioHiDriveStrength(uint8_t GpioMask) {
	uint8_t phyReg = MCR20Drv_IndirectAccessSPIRead(GPIO_DS);
	phyReg &= ~GpioMask;
	MCR20Drv_IndirectAccessSPIWrite(GPIO_DS, phyReg);
}
