/*******************************************************************************
 * @�ļ�    MCR20A_port.c
 * @����  LiXianmao
 * @�汾 V0.0.1
 * @����    2016.9.13
 * @����   mcr20a ��STM32 Ӳ���ӿڣ�SPI�ӿڣ�IRQ�жϽӿڣ���ֲ��
 * @��ע  �ӿ���instanceָ����ΪSPI�˿ڣ�����MCR20A�̶�ΪSPI1���instance����Ҫ��
 *        ����Ϊ�˱�֤��ֲ�ļ��ݣ�instance���Ǳ����ˣ�����Ҫ�������κδ���
 * @��ע
 *******************************************************************************/

/*! *********************************************************************************
 *************************************************************************************
 * Include
 *************************************************************************************
 ********************************************************************************** */

#include "MCR20A_port.h"

#if 0
IRQ_Handler_t gMCR20_Handler;

#endif

uintn16_t spi_lowSpeed;
uint16_t spi_highSpeed;
/*! *********************************************************************************
 *************************************************************************************
 * Public functions
 *************************************************************************************
 ********************************************************************************** */
void spi_master_init(uint32_t instance) {
	//���ó�Masterģʽ����ʼ����������Ϊ15MHz
	SPI_MCR20A_Init();
	//7.5MHz=SPI1����Ƶ��60MHz/8
	SPI_MCR20A_Speed(SPI_BaudRatePrescaler_8);
	spi_lowSpeed = SPI_MCR20A_CR1_GET();
	//15MHz=SPI1����Ƶ��60MHz/4
	SPI_MCR20A_Speed(SPI_BaudRatePrescaler_4);
	spi_highSpeed = SPI_MCR20A_CR1_GET();

	//����CS����
	spi_cs_init();
}

/*****************************************************************************
 MCR20AоƬSPI����Ƶ�ʣ�д���18MHz�������9MHz
 Ϊ�˱��ּ��ݣ�mMCR20SpiWriteSpeed_c��mMCR20SpiReadSpeed_c���䣬���£�
 #define mMCR20SpiWriteSpeed_c (16000000)
 #define mMCR20SpiReadSpeed_c   (8000000)
 ����ʵ��STM32F207ZG SPIֻ��ʵ��15MHz��7.5MHz
 mMCR20SpiWriteSpeed_c = 15MHz
 mMCR20SpiReadSpeed_c = 7.5MHz
 *****************************************************************************/
void spi_master_configure_speed(uint32_t instance, uint32_t freq) {
	uintn16_t prescaler;
#if 0
	if (freq > 8000000) {
		prescaler = SPI_BaudRatePrescaler_4; //60MHz/4=15MHz
	} else {
		prescaler = SPI_BaudRatePrescaler_8; //60MHz/8=7.5MHz
	}

	SPI_MCR20A_Speed(SPI_BaudRatePrescaler_8);
#else
	if (freq > 8000000) {
		SPI_MCR20A_CR1_SET(spi_highSpeed);
	} else {
		SPI_MCR20A_CR1_SET(spi_lowSpeed);
	}
#endif
}

/*****************************************************************************/
/*****************************************************************************/
void spi_master_transfer(uint32_t instance, uint8_t * sendBuffer,
		uint8_t * receiveBuffer, int16_t transferByteCount) {
	volatile uint8_t dummy;

	if (!transferByteCount)
		return;

	if (!sendBuffer && !receiveBuffer)
		return;

	//*��ѡ*��SPI�������ݣ���֤û������

	//��������
	while (transferByteCount--) {
		//dummy=0x01;
		if (sendBuffer) {
			dummy = *sendBuffer;
			sendBuffer++;
		}

		dummy = SPI_MCR20A_WR(dummy);
		if (receiveBuffer) {
			*receiveBuffer = dummy;
			receiveBuffer++;
		}
	}
}
void spi_cs_init() {
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_MCR20A_CS_PIN_EN();

	GPIO_InitStructure.GPIO_Pin = SPI_MCR20A_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(SPI_MCR20A_CS_PORT, &GPIO_InitStructure);
}
//ʹ��CS���͵�ƽ��Ч
void spi_cs_enable() {
	SPI_MCR20A_CS_LOW();
}

//����CS���ߵ�ƽ��Ч
void spi_cs_disable() {
	SPI_MCR20A_CS_HIGH();
}

//��ʼ��RST�ܽţ�MCR20 RST��STM32 RST�Ĺܽ�ֱ������
//��ˣ��޷�ͨ���������RST�ܽſ���MCR20A��λ���ú���Ϊ��
void MCR20A_RST_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	MCR20A_RESET_PIN_EN();
	GPIO_InitStructure.GPIO_Pin = MCR20A_RESET_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; //Ƶ�ʲ���̫��
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //Ӳ���Ѿ�������������
	GPIO_Init(MCR20A_RESET_PORT, &GPIO_InitStructure);

}
//��ʼ��IRQ�ж�����
void MCR20A_IRQ_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/*---����IRQ����Ϊ����--*/
	//ʹ��GPIO��ʱ��
	MCR20A_IRQ_PIN_EN();

	GPIO_InitStructure.GPIO_Pin = MCR20A_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //Ƶ�ʲ���̫��
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //Ӳ���Ѿ�������������
	GPIO_Init(MCR20A_IRQ_PORT, &GPIO_InitStructure);

	/*---��IRQ���Ź���EXTI�ж�----*/
	//ʹ��SYSCFGʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	//����LINE
	MCR20A_IRQ_LINE_CFG();

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = MCR20A_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; //��ʼ��ʱ�����ж�
	EXTI_Init(&EXTI_InitStructure);
	/* Enable and set EXTI Line0 Interrupt to the  priority */
	NVIC_InitStructure.NVIC_IRQChannel = MCR20A_IRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			MCR20A_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCR20A_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //��ʼ����ʱ�򲻴��ж�
	NVIC_Init(&NVIC_InitStructure);

	//�����жϱ�־����֤�ж�ʹ��֮ǰ�����󴥷��жϡ�
	MCR20A_IRQ_CLEAR();
}

void MCR20A_IRQ_Disable() {
	NVIC_InitTypeDef NVIC_InitStructure;
#if 0
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = MCR20A_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
#endif

	/* Enable and set EXTI Line Interrupt to the  priority */
	NVIC_InitStructure.NVIC_IRQChannel = MCR20A_IRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			MCR20A_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCR20A_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void MCR20A_IRQ_Enable() {
	NVIC_InitTypeDef NVIC_InitStructure;
#if 0
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = MCR20A_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
#endif

	/* Enable and set EXTI Line Interrupt to the  priority */
	NVIC_InitStructure.NVIC_IRQChannel = MCR20A_IRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			MCR20A_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCR20A_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
#if 0	//���øú�������halstack.c����
//�ж����
void MCR20A_IRQ_Entry()
{
	//���жϱ�־
	MCR20A_IRQ_CLEAR();
	//��Ϊ����ִ�лص�����
	if(0!=gMCR20_Handler)
	gMCR20_Handler();
}
#endif

#ifdef MCR20A_MCU_RNG_ENABLE
//��ʼ��RNG�����������
void MCR20A_MCU_RNG_Init() {
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}
//��ȡRNG�����ֵ
uint8_t MCR20A_MCU_GetRNG() {
	uint8_t ret;
	while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET)
		; //�ȴ������׼�����
	ret = (uint8_t) RNG_GetRandomNumber(); //����
}
#endif

