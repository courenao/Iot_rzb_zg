/**
 *@file board.c
 *@brief RZG��Ӳ���ӿڶ���(MCU-STM32F207ZG)
 *@author lxm
 *@date 2016.10.13
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX��˾
 *@par �޸���־
 *      0.1.0 xx 2016.10.11 �����ļ�
 *@details Ӳ���嵥���board_config.h
 *@details !!!STM32F207û��Ӳ�����ܿ�����!!!
 **/
#include "board.h"
#include "xprintf.h"

uint8_t debugFlag = 0; //��ӡ��Ϣ��־��0--�رմ�ӡ��1--�򿪵��Դ�ӡ
uint32_t board_SW_Version; //����汾��

/*-------------COM����--------------------------*/
USART_TypeDef* COM_USART[COMn] = { EVAL_COM1, EVAL_COM2 };

GPIO_TypeDef* COM_TX_PORT[COMn] = { EVAL_COM1_TX_GPIO_PORT,
		EVAL_COM2_TX_GPIO_PORT };

GPIO_TypeDef* COM_RX_PORT[COMn] = { EVAL_COM1_RX_GPIO_PORT,
		EVAL_COM2_RX_GPIO_PORT };

const uint32_t COM_USART_CLK[COMn] = { EVAL_COM1_CLK, EVAL_COM2_CLK };

const uint32_t COM_TX_PORT_CLK[COMn] = { EVAL_COM1_TX_GPIO_CLK,
		EVAL_COM2_TX_GPIO_CLK };

const uint32_t COM_RX_PORT_CLK[COMn] = { EVAL_COM1_RX_GPIO_CLK,
		EVAL_COM2_RX_GPIO_CLK };

const uint16_t COM_TX_PIN[COMn] = { EVAL_COM1_TX_PIN, EVAL_COM2_TX_PIN };

const uint16_t COM_RX_PIN[COMn] = { EVAL_COM1_RX_PIN, EVAL_COM2_RX_PIN };

const uint8_t COM_TX_PIN_SOURCE[COMn] = { EVAL_COM1_TX_SOURCE,
		EVAL_COM2_TX_SOURCE };

const uint8_t COM_RX_PIN_SOURCE[COMn] = { EVAL_COM1_RX_SOURCE,
		EVAL_COM2_RX_SOURCE };

const uint8_t COM_TX_AF[COMn] = { EVAL_COM1_TX_AF, EVAL_COM2_TX_AF };

const uint8_t COM_RX_AF[COMn] = { EVAL_COM1_RX_AF, EVAL_COM2_RX_AF };

//��������ʹ��
void Board_COM2_Input_Enable() {
	EVAL_COM2_EN_CLKEN();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

	GPIO_InitStructure.GPIO_Pin = EVAL_COM2_EN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EVAL_COM2_EN_PORT, &GPIO_InitStructure);

	//�͵�ƽ��Ч
	GPIO_ResetBits(EVAL_COM2_EN_PORT, EVAL_COM2_EN_PIN);

}
//�����������
void Board_COM2_Input_Disable() {
	//�ߵ�ƽ����
	GPIO_SetBits(EVAL_COM2_EN_PORT, EVAL_COM2_EN_PIN);
}
/**
 * @brief  Configures COM port.
 * @param  COM: Specifies the COM port to be configured.
 *   This parameter can be one of following parameters:
 *     @arg COM1
 *     @arg COM2
 * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
 *   contains the configuration information for the specified USART peripheral.
 * @retval None
 */
void Board_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM], ENABLE);

	if (COM == COM1) {
		/* Enable UART clock */
		RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	} else {
		/* Enable UART clock */
		RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	}

	/* Connect PXx to USARTx_Tx*/
	GPIO_PinAFConfig(COM_TX_PORT[COM], COM_TX_PIN_SOURCE[COM], COM_TX_AF[COM]);

	/* Connect PXx to USARTx_Rx*/
	GPIO_PinAFConfig(COM_RX_PORT[COM], COM_RX_PIN_SOURCE[COM], COM_RX_AF[COM]);

	/* Configure USART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

	GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

	/* Configure USART Rx as alternate function  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

	/* USART configuration */
	USART_Init(COM_USART[COM], USART_InitStruct);

	/* Enable USART */
	USART_Cmd(COM_USART[COM], ENABLE);
}

const uint16_t GPIO_PIN[LEDn] = { LED_ALARM_PIN, LED_RUN_PIN };
GPIO_TypeDef* GPIO_PORT[LEDn] = { LED_ALARM_PORT, LED_RUN_PORT };
const uint32_t GPIO_CLK[LEDn] = { LED_ALARM_CLK, LED_RUN_CLK };
/**
 * @brief  Configures LED GPIO.
 * @param  Led: Specifies the Led to be configured.
 *   This parameter can be one of following parameters:
 *     @arg ALARM
 *     @arg RUN
 * @retval None
 */
void Board_LEDInit(Led_TypeDef Led) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(GPIO_CLK[Led], ENABLE);

	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);

	//��ʼ���ر�LED��
	Board_LedOff(Led);
}
void Board_LedOn(Led_TypeDef Led) {
	//�͵�ƽ����
	GPIO_ResetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}

void Board_LedOff(Led_TypeDef Led) {
	//�ߵ�ƽ���
	GPIO_SetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}
//��ʼ��IO�ܽ�
void Board_PWM_Pin_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(PWM_CLK, ENABLE);

	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = PWM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_PORT, &GPIO_InitStructure);
}

//ʹ��PWM���Ʒ���������ʼ���ܽ�
void Board_PWM_Init() {
	Board_PWM_Pin_Init();

#if PWM_USE_TIM  //ʹ�ö�ʱ������PWM
	/*---��ʼ����ʱ��TIM1----*/

	//ʹ�ܶ�ʱ��ʱ��
	PWM_TIM_CLK_EN();

	//TIM1/8 ����Ƶ��Ϊ30MHz ��TIM2~7����Ƶ��Ϊ60MHz
	//����Ƶ��=����Ƶ��/(PWM_CLKDIV+1)/(PWM_PRESCALER+1)/PERIOD
	//Ŀ��PWMƵ��=4KHz��60M/4K=15000��ʹ��IO�ߵ͵�ƽ��ת�Ļ���Ҫ����2��15000/2=7500,
	//��ʱ���ж�Ƶ��Ϊ60M/7500=8KHz
#define PWM_PERIOD  			(100-1)
#define PWM_PULSE   			(PWM_PERIOD/2)		//ռ�ձ�Ϊ50%
#define PWM_PRESCALER  			(75-1)	//
#define PWM_CLKDIV				0
	/*-���ö�ʱ��Ƶ��-*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD; //һ�����ڶ�Ӧ�ļ�����ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = PWM_PRESCALER;
	TIM_TimeBaseStructure.TIM_ClockDivision = PWM_CLKDIV; //Ԥ��Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //�������м�ֵ�����жϣ����ڷ�ת���
	TIM_TimeBaseInit(PWM_TIM, &TIM_TimeBaseStructure);

	//�Զ�װ�����ڼ���ֵ
	TIM_ARRPreloadConfig(PWM_TIM, ENABLE);
	//ʹ��TIM
	TIM_Cmd(PWM_TIM, DISABLE); //��ʼ����������ʱ����ʹ��PWM_ON��������

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = PWM_TIM_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			PWM_TIM_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = PWM_TIM_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//������ж�
	TIM_ClearITPendingBit(PWM_TIM, TIM_IT_Update);
	//ʹ��TIM�жϣ�TIM_IT_UpdateΪ����ж�
	TIM_ITConfig(PWM_TIM, TIM_IT_Update, ENABLE);
#endif
}
#if PWM_USE_TIM  //ʹ�ö�ʱ������PWM
//����PWM
void Board_PWM_On() {
	//ʹ��TIM
	TIM_Cmd(PWM_TIM, ENABLE);
}
void Board_PWM_Off() {
	//ʹ��TIM
	TIM_Cmd(PWM_TIM, DISABLE);
}
static uint8_t isLow = 0;
static uint32_t periodCount = 0, //��������һ��ά�ֵ�����������֤�������͹�100���ɡ�
		delayCount = 0, //����������ʱ������������1s�Ļ����ֵ���Ϊ4000
		beep1sCount = 0; //��������Ĵ�����ÿ�μ��ʱ��1s
static uint32_t beep1sMax = 3; //���������1s��������������

static uint8_t beepIsOn = 1; //0--�������رգ�1--�����������죬��ʼΪ1��֤��һ�ν�����
static uint8_t beepIsBusy = 0;
//�޸ķ�������һ��ά�ֵ���������Ĭ��Ϊ3
ErrorStatus Board_PWM_SetCount(uint8_t c) {
	if (beepIsBusy || 0 == c)
		return ERROR; //ʧ�ܣ�PWM���ڹ�������ȴ�ֹͣ
	beep1sMax = c; //���÷�������������
	return SUCCESS;
}
//��ǰPWM�Ƿ����ڹ�����0--���ڹ�����1--�Ѿ�ֹͣ
uint8_t Board_PWM_IsOff() {
	return beepIsBusy == 0 ? 1 : 0;
}

#define COUNT_1S		8000	//��ʱ��Ƶ��Ϊ8KHz��1s��Ӧ�ļ���ֵΪ8000
#define COUNT_PERIOD    200		//200�Ϳ���������200/4K=50ms��
//��ʱ���ж�
void Board_PWM_IRQHandler() {
	TIM_ClearITPendingBit(PWM_TIM, TIM_IT_Update);
	beepIsBusy = 1;

	//COUNT_PERIOD�����ں�ֹͣ����֤���ʱ���㹻����������
	if (periodCount > COUNT_PERIOD) {
		periodCount = 0;
		delayCount = 0;
		beepIsOn = 0; //�رշ�����
	}

	if (beepIsOn) {
		//���PWM����������
		if (isLow) {
			PWM_LOW();
			isLow = 0;
			periodCount++;
		} else {
			PWM_HIGH();
			isLow = 1;
		}
	} else {
		//��ʼ����
		delayCount++;
	}

	if (delayCount > 0 && beep1sMax == 1) { //ֻ��1��ֱ��ֹͣ
	//ֹͣ��ʱ�����رշ�����PWM
		TIM_Cmd(PWM_TIM, DISABLE);
		/*-��ԭ���м�����Ϊ��ʼֵ-*/
		periodCount = 0;
		delayCount = 0;
		beep1sCount = 0;
		beep1sMax = 3;
		beepIsOn = 1;
		beepIsBusy = 0; //��־����������
	} else {
		//1sʱ�䵽��
		if (delayCount > COUNT_1S) {
			beep1sCount++;
			//�������������
			if (beep1sCount >= beep1sMax) {
				//ֹͣ��ʱ�����رշ�����PWM
				TIM_Cmd(PWM_TIM, DISABLE);
				/*-��ԭ���м�����Ϊ��ʼֵ-*/
				periodCount = 0;
				delayCount = 0;
				beep1sCount = 0;
				beep1sMax = 3;
				beepIsOn = 1;
				beepIsBusy = 0; //��־����������
			} else {
				//���´򿪷�����
				beepIsOn = 1;
			}
			delayCount = 0;
		}
	}
}
#else

//4.1KHz ��PWM���Σ�ά��100�����ڣ���ά��ʱ��24ms
void Board_PWM_On() {
	uint16_t i, j;
	for (i = 0; i < 100; i++) {
		for (j = 0; j < 1200; j++)
		;
		PWM_LOW();
		for (j = 0; j < 1200; j++)
		;
		PWM_HIGH();
	}
}

#endif //ENDOF PWM_USE_TIM
#define RTC_CLOCK_SOURCE_LSE

void RTC_Config(void) {
	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI)  /* LSI used as RTC source clock*/
	/* The RTC Clock may varies due to LSI frequency dispersion. */
	/* Enable the LSI OSC */
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

#elif defined (RTC_CLOCK_SOURCE_LSE) /* LSE used as RTC source clock */
	/* Enable the LSE OSC */
	RCC_LSEConfig(RCC_LSE_ON);

	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#else
#error Please select the RTC Clock source inside the main.c file
#endif /* RTC_CLOCK_SOURCE_LSI */

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();
}
//RTC������ֵ�����ڱ�־RTC�豸�Ƿ��ʼ�����ˡ�
//��RTC�豸��ʼ����ɺ󽫸�ֵд�뱸������������綪ʧ�����ʾ��Ҫ���³�ʼ��RTC������ֱ�Ӷ�дRTC��
#define RTC_BKP_VALUE  0x32F2
//��ʼ��RTCΪ1sһ���ж�
void Board_RTC_Init() {
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_BKP_VALUE) {
		RTC_Config();

		/*---��ʼ��RTCʱ��--*/
		RTC_InitTypeDef RTC_InitStruct;
		RTC_StructInit(&RTC_InitStruct); //����Ĭ��ֵ���г�ʼ��
		/* Check on RTC init */
		if (RTC_Init(&RTC_InitStruct) == ERROR) {
			//��ʼ��ʧ��
		}

		RTC_DateTypeDef RTC_DateStruct;
		RTC_DateStruct.RTC_Year = 16; //0~99
		RTC_DateStruct.RTC_Month = 11; //1~12
		RTC_DateStruct.RTC_Date = 1; //1~31
		RTC_DateStruct.RTC_WeekDay = 2; //
		if (RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct) == ERROR) {
			//��������ʧ��
		}

		RTC_TimeTypeDef RTC_TimeStruct;
		RTC_TimeStruct.RTC_Hours = 11;
		RTC_TimeStruct.RTC_Minutes = 45;
		RTC_TimeStruct.RTC_Seconds = 0;
		RTC_TimeStruct.RTC_H12 = 0;
		if (RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct) == ERROR) {
			//����ʱ��ʧ��
			printf("RTC ��ʼ��ʧ��");
		} else { //�ɹ���д����������־RTC��ʼ�����
			RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_BKP_VALUE);
		}

	} else {

		/* Check if the Power On Reset flag is set */
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) {
			printf("\r\n Power On Reset occurred....\n");
		}
		/* Check if the Pin Reset flag is set */
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) {
			printf("\r\n External Reset occurred....\n");
		}

		printf("\r No need to configure RTC....\n");

		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC */
		PWR_BackupAccessCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();

		/* Clear the RTC Alarm Flag */
		RTC_ClearFlag(RTC_FLAG_ALRAF);

		/* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
		EXTI_ClearITPendingBit(RTC_IRQ_LINE_ALARM | RTC_IRQ_LINE_TS);

	}
#if 0

#if RTC_TAMP_ENABLE	//����ʹ��TAMP��STAMP
	//ʹ�����ж�
	RTC_ITConfig(RTC_IT_TS, ENABLE);
	/*------------------
	 * RTC tamp��stampʹ����ͬ���жϣ�
	 * �ڷ������������EXTI_GetITStatus
	 * --------------------*/
	//�����жϼĴ�����RTC_IRQ_LINE_TS�ж�
	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_InitStructure.EXTI_Line = RTC_IRQ_LINE_TS;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//ʹ��NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQ_CHANNEL_TS;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
	RTC_IRQ_PRIO_EMPTION_TS;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = RTC_IRQ_PRIO_SUB_TS;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#else
	RTC_AlarmTypeDef RTC_AlarmStructure;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = 0;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 30;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_All
	& (~RTC_AlarmMask_Seconds); //ֻ������
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
	//ʹ�����ж�
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	/*------------------
	 * RTC tamp��stampʹ����ͬ���жϣ�
	 * �ڷ������������EXTI_GetITStatus
	 * --------------------*/
	//�����жϼĴ�����RTC_IRQ_LINE_TS�ж�
	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_InitStructure.EXTI_Line = RTC_IRQ_LINE_ALARM;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//ʹ��NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQ_CHANNEL_ALARM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
	RTC_IRQ_PRIO_EMPTION_ALARM;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = RTC_IRQ_PRIO_SUB_ALARM;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//����ALARMA
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
#endif //endof RTC_TAMP_ENABLE
#endif //
}
//RTC Tamp/Stamp�жϷ������
void RTC_IRQHandler_TS() {
	if (EXTI_GetITStatus(RTC_IRQ_LINE_TS) != RESET) { //STamp�ж�
		//���жϱ�־
		EXTI_ClearITPendingBit(RTC_IRQ_LINE_TS);

		//TODO ִ���жϴ���

	} else { //Tamp�ж�
		//TODO ���жϱ�־

	}
}
//RTC Alarm�����жϷ������
void RTC_IRQHandler_ALARM() {

	EXTI_ClearITPendingBit(RTC_IRQ_LINE_ALARM);
}

//����Flash��ʼ��
//STM32F207����512KB Flash����ʼ��ַ0x08000000��4*16KB+1*16KB+3*128KB
void Board_Flash_Init() {
	//FLASH_SetLatency(FLASH_Latency_3);
	FLASH_Unlock();
	FLASH_ClearFlag(
			FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR
					| FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR
					| FLASH_FLAG_PGSERR);
	//�����������ݿ�
	while (FLASH_EraseSector(USER_FLASH_SECTOR, VoltageRange_3)
			!= FLASH_COMPLETE)
		;
}
//�ӻ���ַ��ʼд��MAC��ַ������8�ֽ�=8*8=64λ
void Board_Flash_Write_Mac(uint64_t data) {
	uint32_t tmp;
	Board_Flash_Init();
	for (int i = 0; i < sizeof(data) / sizeof(tmp); i++) {
		tmp = (uint32_t)(data >> (i * 32));

		//FLASH_ProgramWord�����VoltageRange_3ƥ�䣻
		//��VoltageRange_3ʱ��ֻ��ʹ��FLASH_ProgramWord
		if (FLASH_ProgramWord(USER_FLASH_MAC_BASE + i * 4, tmp)
				== FLASH_COMPLETE) {
			//�ɹ�
		}
	}
	FLASH_Lock();
}
//��ȡFLASH��MAC��ַ��1--��ʾ�ɹ���0--��ʾʧ��
uint8_t Board_Flash_Read_Mac(uint8_t mac[8]) {
	uint8_t *ptr;
	ptr = (uint8_t *) (USER_FLASH_MAC_BASE);
	uint8_t addrValid = 0;

	for (int i = 0; i < 8; i++) {
		mac[i] = *ptr;
		ptr++;
		if (mac[i] != 0xFF) //��ȫΪ0xFF����Ϊ��Ч
			addrValid++;
	}

	if (0 == addrValid) { //FLASH��ȡMAC��ַʧ�ܣ�����Ĭ�ϵ�ַ
		/*
		 mac[0] = aExtendedAddress_B0;
		 mac[1] = aExtendedAddress_B1;
		 mac[2] = aExtendedAddress_B2;
		 mac[3] = aExtendedAddress_B3;
		 mac[4] = aExtendedAddress_B4;
		 mac[5] = aExtendedAddress_B5;
		 mac[6] = aExtendedAddress_B6;
		 mac[7] = aExtendedAddress_B7;
		 */
		return 0; //ʧ��
	}
	//�ɹ�
	return 1;
}

//��ȡMCUӲ��ID��
void Board_MCU_ID(uint32_t id[3]) {
	uint32_t *ptr;
	ptr = (uint32_t *) MCU_ID_BASE;
	for (int i = 0; i < 3; i++) {
		id[i] = *ptr;
		ptr++;
	}
}

static uint8_t deviceMacAddr[12]; //�豸���к��ܳ���Ϊ12�ֽ�
static uint8_t deviceMacOnce = 1;
//�ڲ��������Զ�����MAC��ַ
void Board_Mac_AutoGen() {
	if (deviceMacOnce) { //ִֻ��һ�����ɹ���
		uint8_t *ptr = (uint8_t *) MCU_ID_BASE;
		//printf("Device ID: ");
		for (int i = 0; i < sizeof(deviceMacAddr); i++) {
			deviceMacAddr[i] = *ptr;
			//	printf("%02X", *ptr);
			ptr++;
		}
		deviceMacOnce = 0;
	}

	//printf("\n");

}
//�û��ӿڣ���ȡIEEE MAC��ַ8�ֽ�
void Board_Mac_Get(uint8_t mac[8]) {

	for (int i = 0; i < 8; i++) {
		mac[i] = deviceMacAddr[i];
	}
}

uint8_t* Board_Mac_Get_Ptr() {
	return deviceMacAddr;
}

//��Flash��д�����ݣ�dataΪ����ָ�룬sizeΪд�����ݵĴ�С����С������sizeof(uint32_t)�ı�����
void Board_Flash_Write(uint32_t *data, uint32_t size) {
	Board_Flash_Init();
	for (int i = 0; i < (size / sizeof(uint32_t)); i++) {
		//FLASH_ProgramWord�����VoltageRange_3ƥ�䣻
		//��VoltageRange_3ʱ��ֻ��ʹ��FLASH_ProgramWord
		FLASH_ProgramWord(USER_FLASH_BASE + i * 4, data[i]);
	}
	FLASH_Lock();
}

//���豸��Ϣд��Flash
void Board_DeviceInfo_Write(Device_Info_t *info) {
	Board_Flash_Write((uint32_t *) (info), sizeof(Device_Info_t));
}

//���豸��Ϣ����Flash��������info
void Board_DeviceInfo_Read(Device_Info_t *info) {
#if 0
	uint32_t *ptr=(uint32_t *)info;
	uint32_t *pflash=(uint32_t *)(USER_FLASH_BASE);
	for(uint32_t i=0;i<sizeof(Device_Info_t)/sizeof(uint32_t);i++)
	{
		ptr[i]=pflash[i];
	}
#else
	memcpy(info, (uint32_t *) (USER_FLASH_BASE), sizeof(Device_Info_t));
#endif
}
//��ȡ�����豸��Ϣ
Device_Info_t * Board_DeviceInfo_Get() {
	return (Device_Info_t *) (USER_FLASH_BASE);
}
//��ӡ�����豸��Ϣ
void Board_DeviceInfo_Print() {
	//���Դ�ӡ����Ŵ򿪴�ӡ
	if (debugFlag) {
		Device_Info_t *ptr = Board_DeviceInfo_Get(); //��ȡ�豸��Ϣ

		printf("---------------DeviceInfo-----------------\n");
		//��ӡMAC��ַ
		printf("--MAC Address: ");
		for (uint8_t i = 0; i < sizeof(ptr->mac); i++) {
			printf("%02X", ptr->mac[i]);
		}
		printf("\n");

		//��ӡSN
		printf("--SN: %08d\n", ptr->sn);

		//��ӡ�汾��
		printf("--VER FW: ");
		Board_Version_Print(ptr->ver_fw);
		printf("--VER SW: ");
		Board_Version_Print(ptr->ver_sw);

		//��ӡ�������ò���
		printf("--NetWork Setting \n");
		printf("----|_type: %04x\n", ptr->sets.type);
		printf("----|_idle: %d\n", ptr->sets.idle);
		printf("----|_tx: %d\n", ptr->sets.tx);
		printf("----|_heart: %d\n", ptr->sets.heart);

		//��ӡ����ģʽ
		printf("--DEBUG: %s\n", (ptr->debug == 1) ? "ON" : "OFF");

#if (BOOT_LOADER || RZG)	//BOOT_LOADER��Ҫ�Զ�ʶ���豸RZB��ZG
		//ֻ��ZG���������ַ��ӡ
		if (Board_Is_RZB() == 0) {
			//��ӡIP
			printf("--IP Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip); i++) {
				printf("%d.", ptr->udp.ip[i]);
			}
			printf("\n");

			//��ӡ��������
			printf("--MASK Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip_mask); i++) {
				printf("%d.", ptr->udp.ip_mask[i]);
			}
			printf("\n");

			//��ӡ���ص�ַ
			printf("--GATEWAY Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip_gateway); i++) {
				printf("%d.", ptr->udp.ip_gateway[i]);
			}
			printf("\n");

			//Ŀ���IP��ַ
			printf("--TARGET Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip_target); i++) {
				printf("%d.", ptr->udp.ip_target[i]);
			}
			printf("\n");
			//��ӡ����˿�
			printf("--IP SEND PORT: %d\n", ptr->udp.send_port);
			printf("--IP RECV PORT: %d\n", ptr->udp.recv_port);
		}
#endif
		printf("---------------DeviceInfo END-------------\n");
	}
}

//�ж���ǰ��������Ƿ���Ч�������Ч�򱣴沢������Ч,0--����ʧ�ܣ�1--���óɹ�
uint8_t Board_Network_Set(Network_setting_t *settings) {
	Device_Info_t info_org;
	uint8_t idle_valid = 0, tx_valid = 0, heart_valid = 0;
	uint8_t ret = 0;
	//�Ƿ�Ĭ��ֵ������Ч
	if ((settings->type) & (TYPE_MASK_SET_DEFAULT)) {
		//��ԭ��Ĭ��ֵ����ֱ���˳�
		Board_Network_SetDefault();
		//ZigBee��Ϣ������Ч
		Network_Config();
		return 1; //�������óɹ�
	}

	Board_DeviceInfo_Read(&info_org); //��ȡ�������в���
	if (((settings->type) & (TYPE_MASK_IDLE_TIME))
			&& (IS_VALID_IDLE_TIME(settings->idle))) { //����ʱ����Ч

		//�޸�ԭʼ����
		info_org.sets.idle = settings->idle;

		idle_valid = 1;
	}
	if (((settings->type) & (TYPE_MASK_TX_TIMEOUT))
			&& (IS_VALID_TX_TIMEOUT(settings->tx))) { //���з��ͳ�ʱ��Ч

		//��֤���յ��Ĳ�����������tx<heart��
		//������յ��Ĳ�����Чλ��heart��Ч����ʹ�ý��յ���heartֵ���бȽ�
		//����ʹ��ԭʼ��¼��heartֵ���бȽϣ�ֻ�����������ű��棬���򲻴���
		if ((settings->type) & (TYPE_MASK_HEART_TIMEOUT)) { //����������ЧλΪ1
			if (IS_VALID_HEART_TIMEOUT(settings->heart,settings->tx)) {
				//������Ч
				tx_valid = 1;
			}
		} else { //����������ЧλΪ0������ԭʼ�����������бȽ�
			if (IS_VALID_HEART_TIMEOUT(info_org.sets.heart,settings->tx)) {
				//������Ч
				tx_valid = 1;
			}
		}

		if (tx_valid) {
			//�޸�ԭʼ����
			info_org.sets.tx = settings->tx;
		}
	}
	//heart������tx����֮��
	if ((settings->type) & (TYPE_MASK_HEART_TIMEOUT)) { //����������ʱ��Ч

		if (IS_VALID_HEART_TIMEOUT(settings->heart,info_org.sets.tx)) //������ʱʱ��>ԭʼ�ķ��ͳ�ʱ
		{
			//�޸�ԭʼ����
			info_org.sets.heart = settings->heart;
			heart_valid = 1;
		}

	}

	//д��Flash
	if (idle_valid || tx_valid || heart_valid) {
		info_org.sets.type = settings->type; //��������ֵ

		Board_DeviceInfo_Write(&info_org);

		//�������磬ʹ�ò�����Ч���ú���������Flashд��֮�����
		Network_Config();
		ret = 1; //���óɹ�

		//��ӡ
		Dprintf("NetSetting Idle=%d\n", info_org.sets.idle);
		Dprintf("NetSetting Tx=%d\n", info_org.sets.tx);
		Dprintf("NetSetting Heart=%d\n", info_org.sets.heart);
		Dprintf("����������óɹ�!\n");

	}

	return ret;
}

//ZigBee������ز�����ԭ��Ĭ��ֵ��������UDP��������
void Board_Network_SetDefault() {
	Device_Info_t info;
	uint8_t *mac = Board_Mac_Get_Ptr();
	Board_Mac_AutoGen(); //����MAC��ַ

	info.flag = DEVICE_INFO_VALID; //��Ϊ��Ч
	//����MAC��ַ
	for (uint8_t i = 0; i < sizeof(info.mac); i++) {
		info.mac[i] = mac[i];
	}
	//����SN��
	info.sn = DEVICE_SN;
	//���ù̼��汾��
	Device_Info_t info_org;
	Board_DeviceInfo_Read(&info_org); //��ȡ�������в���
	info.ver_fw = info_org.ver_fw;
	//���ð汾��
	info.ver_sw = Board_Version_Get();

	//�����������ΪĬ��ֵ
	info.sets.type = DEFAULT_VALUE_SET_TYPE;
	info.sets.idle = DEFAULT_VALUE_SET_IDLE;
	info.sets.tx = DEFAULT_VALUE_SET_TX;
	info.sets.heart = DEFAULT_VALUE_SET_HEART;
	//д��Flash
	Board_DeviceInfo_Write(&info);
}

//���豸��Ϣ����ΪĬ��ֵ
void Board_DeviceInfo_SetDefault() {
	Device_Info_t info;
	uint8_t *mac = Board_Mac_Get_Ptr();
	Board_Mac_AutoGen(); //����MAC��ַ

	info.flag = DEVICE_INFO_VALID; //��Ϊ��Ч
	//����MAC��ַ
	for (uint8_t i = 0; i < sizeof(info.mac); i++) {
		info.mac[i] = mac[i];
	}
	//����SN��
	info.sn = DEVICE_SN;

	//���ð汾��
#if BOOT_LOADER
	//���ù̼��汾��
	info.ver_fw = Board_Version_Get();
	//��������汾��
	Device_Info_t info_org;
	Board_DeviceInfo_Read(&info_org);//��ȡ�������в���
	info.ver_sw=info_org.ver_sw;
#else
	//��������汾��
	info.ver_sw = Board_Version_Get();
	//���ù̼��汾��
	Device_Info_t info_org;
	Board_DeviceInfo_Read(&info_org); //��ȡ�������в���
	info.ver_fw = info_org.ver_fw;
#endif

	//�����������ΪĬ��ֵ
	info.sets.type = DEFAULT_VALUE_SET_TYPE;
	info.sets.idle = DEFAULT_VALUE_SET_IDLE;
	info.sets.tx = DEFAULT_VALUE_SET_TX;
	info.sets.heart = DEFAULT_VALUE_SET_HEART;

	//����ģʽ
	info.debug = 0; //Ĭ�ϲ�����
#if (BOOT_LOADER || RZG)	//BOOT_LOADER��Ҫ�Զ�ʶ���豸RZB��ZG
			//ֻ��ZG���������ַ��ӡ
			if (Board_Is_RZB() == 0) {

				//����IP��ַ
				info.udp.ip[0] = BOARD_IP_0;
				info.udp.ip[1] = BOARD_IP_1;
				info.udp.ip[2] = BOARD_IP_2;
				info.udp.ip[3] = BOARD_IP_3;

				//������������
				info.udp.ip_mask[0] = BOARD_NETMASK_0;
				info.udp.ip_mask[1] = BOARD_NETMASK_1;
				info.udp.ip_mask[2] = BOARD_NETMASK_2;
				info.udp.ip_mask[3] = BOARD_NETMASK_3;

				//�������ص�ַ
				info.udp.ip_gateway[0] = BOARD_GW_0;
				info.udp.ip_gateway[1] = BOARD_GW_1;
				info.udp.ip_gateway[2] = BOARD_GW_2;
				info.udp.ip_gateway[3] = BOARD_GW_3;

				//���÷�����IP��ַ
				info.udp.ip_target[0] = BOARD_IP_SERVER_0;
				info.udp.ip_target[1] = BOARD_IP_SERVER_1;
				info.udp.ip_target[2] = BOARD_IP_SERVER_2;
				info.udp.ip_target[3] = BOARD_IP_SERVER_3;

				//��������˿�
				info.udp.send_port = UDP_SEND_PORT;
				info.udp.recv_port = UDP_RECV_PORT;
			}
#endif
	//д��Flash
	Board_DeviceInfo_Write(&info);
}

//��ʼ���豸��Ϣ�����Flash���豸��Ϣ��д�롣
void Board_DeviceInfo_Init() {
	Device_Info_t *ptr = Board_DeviceInfo_Get();
	if (ptr->flag != DEVICE_INFO_VALID) { //δ��ʼ��
		//����ΪĬ��ֵ
		Board_DeviceInfo_SetDefault();
	}
	debugFlag = ptr->debug; //�����������
#if BOOT_LOADER
			if (ptr->ver_fw != Board_Version_Get()) { //�̼��汾��δ����
				Device_Info_t info;
				Board_DeviceInfo_Read(&info);//��ȡ�������в���
				info.ver_fw = Board_Version_Get();//д��̼��汾��
				Board_DeviceInfo_Write(&info);
			}
#else

	if (ptr->ver_sw != Board_Version_Get()) { //����汾��δ����
		Device_Info_t info;
		Board_DeviceInfo_Read(&info); //��ȡ�������в���
		info.ver_sw = Board_Version_Get(); //д������汾��
		Board_DeviceInfo_Write(&info);
	}
#endif

}
//�����λ
void Board_Soft_Reset() {
	Dprintf("ϵͳ����������\n");
	__set_FAULTMASK(1); //�ر������жϣ�core_cmFunc.h
	NVIC_SystemReset(); //��λ����
}
//�رջ��ԵĴ�������
uint32_t Board_COM2_Gets(uint8_t * buffP, uint32_t length) {
	static uint32_t bytes_read = 0;
	uint8_t c = 0;

	if (xUSART2_ValidInput()) {
		c = xUSART2_getchar();
		if (c == '\r') {
			//xUSART2_putchar('\n');
			//xUSART2_putchar('\r');
			buffP[bytes_read] = '\0';
			bytes_read = 0;

			return 1;
		}

		if (c == '\b') /* Backspace */
		{
			if (bytes_read > 0) {
				//xUSART2_putchar('\b');
				//xUSART2_putchar(' ');
				//xUSART2_putchar('\b');
				bytes_read--;
			}
			return 0;
		}
		//if (bytes_read >= (CMD_STRING_SIZE))
		if (bytes_read >= (length)) {
			//printf("Command string size overflow\r\n");
			bytes_read = 0;
			return 0;
		}
		if (c >= 0x20 && c <= 0x7E) {
			buffP[bytes_read] = c;
			bytes_read++;
			//xUSART2_putchar(c);
		}
	}

	return 0;
}

#if BOOT_LOADER

void Board_Shell() {
	uint8_t comKey[10];
	while (1) {
		printf("\nDEBUG>");
		//�����Ե����룬������ʽ
		xgets(comKey, sizeof(comKey));

		//����ģʽ����
		if (strcmp(comKey, COM_KEY_SHELL_DEBUGON) == 0) {
			printf("\n����ģʽ����.\n");
			Device_Info_t info_org;
			Board_DeviceInfo_Read(&info_org); //��ȡ�������в���
			info_org.debug=1;//����ģʽ��Ϊ1

			//�������
			Board_DeviceInfo_Write(&info_org);
			//break;
		}
		//����ģʽ�ر�
		if (strcmp(comKey, COM_KEY_SHELL_DEBUGOFF) == 0) {
			printf("\n����ģʽ�ر�.\n");
			Device_Info_t info_org;
			Board_DeviceInfo_Read(&info_org); //��ȡ�������в���
			info_org.debug=0;//����ģʽ��Ϊ1

			//�������
			Board_DeviceInfo_Write(&info_org);
			//break;
		}

		//�ָ���������
		if (strcmp(comKey, COM_KEY_SHELL_RESET) == 0) {

			printf("\n����ִ�лָ��������ã�[ȷ��]������Y��������ִ�У�\n");

			xgets(comKey, sizeof(comKey));
			if ((strcmp(comKey, "Y") == 0) || (strcmp(comKey, "y") == 0)) {
				printf("����ִ�лָ��������á�\n");
				Board_DeviceInfo_SetDefault();
			} else {
				printf("�������ò���ȡ����\n");
			}
			//break;
		}

		//�˳�Shell
		if (strcmp(comKey, COM_KEY_SHELL_EXIT) == 0) {
			break;
		}

		//Xmodem
		if (strcmp(comKey, COM_KEY_SHELL_XUPDATE) == 0) {
			printf("\n��ʼ����Xmodem 1K���������������������κ��ַ��������ɹ����Զ����г���\n");
			if (IAP_Service(0) == 0) { //IAP��дʧ�ܣ���Ҫ����
				printf("��������ʧ�ܣ�\n");
				//���˳�while
			} else {
				printf("���������ɹ�����������Ӧ�ó���\n");
				break;
			}
		}
		//ǿ�Ʋ���Ӧ�ó���
		if (strcmp(comKey, COM_KEY_SHELL_ERASE) == 0) {
			printf("�����棡����ǿ�Ʋ���Ӧ�ó��򣡣�ȷ��������Y��������ִ�С�\n");
			xgets(comKey, sizeof(comKey));

			if ((strcmp(comKey, "Y") == 0) || (strcmp(comKey, "y") == 0)) {
				IAP_Program_Erase();
			}
			//break;
		}

		//��ӡ�豸��Ϣ
		if (strcmp(comKey, COM_KEY_SHELL_PRINTINFO) == 0) {
			Board_DeviceInfo_Init();
			debugFlag=1; //������ӡ��־
			Board_DeviceInfo_Print();
			debugFlag=0;//�رմ�ӡ��־
			//break;
		}

		//�����豸��Ϣ
		if (strcmp(comKey, COM_KEY_SHELL_SETINFO) == 0) {
			printf("�豸��Ϣ���ù�����ʱ��֧��\n");
			//break;
		}

		//��ӡ����
		if (strcmp(comKey, COM_KEY_SHELL_HELP) == 0) {
			printf(COM_KEY_SHELL_HELP_INFO);
			//break;
		}
	}
}

//����SHELL��wait--�û��ȴ���־�����wait=0��ʾ���ȴ����̽���SHELL������ȴ�һ��ʱ���û�����������ȷ�����shell
void Board_Shell_Entry(uint8_t wait) {
	//ʹ�ܴ�������
	Board_COM2_Input_Enable();
	if (wait) { //wait��Ч��־��Ҫ�ȴ�������ȴ���ʱ���˳�

		//�ȴ�5s�еȴ��û�����
#define COM_INPUT_TIMEOUT		5000
		uint32_t timeout = GetTimeMS();
		//��������"232start"�ſ��Խ������û���
		uint8_t comKey[10];

		printf("ϵͳ�����У���ȴ�%d��.\n���ؽ��ȣ�  ", COM_INPUT_TIMEOUT / 1000);
		while ((GetTimeMS() - timeout) < COM_INPUT_TIMEOUT) {
			//�������Ե�����
			Board_COM2_Gets(comKey, sizeof(comKey));

			if (strcmp(comKey, COM_KEY_SHELL) == 0) {
				Board_Shell();
				//�˳�while
				break;
			}

			uint32_t delta = (GetTimeMS() - timeout);
			if (delta % (COM_INPUT_TIMEOUT / 10) == 0) {
				printf("\b\b%2d%", delta * 100 / COM_INPUT_TIMEOUT); //�˸�
			}
			//�ӳ�һ��ʱ�䣬ʱ��ֵ
			delayMS(1);
		}
		printf("\b\b100\n");
	} else {
		Board_Shell();
	}
	//���ô�������
	Board_COM2_Input_Disable();
}

#endif //ENDOF BOOT_LOADER
//Flash���򱣻�����ֹ��оƬ�ж�ȡ����
void Board_Flash_Protect() {
	if (FLASH_OB_GetRDP() == RESET) { //FLASHδ������
		FLASH_Unlock();
		FLASH_OB_Unlock();
		FLASH_OB_RDPConfig(OB_RDP_Level_1); //д����
		FLASH_OB_Launch(); //������Ч
		FLASH_OB_Lock();
		FLASH_Lock();
	}
}

//RAWԭʼ���ݻ�ȡ��������������ʽ��ȡ�������ݣ����سɹ���ȡ�ֽ���
uint32_t Board_COM_Nowait_RAW_Gets(uint8_t *key, uint32_t len) {
	static uint32_t bytes = 0;
	if (xUSART2_ValidInput()) {
		key[bytes++] = xUSART2_getchar();
		if (bytes >= len) {
			bytes = 0;
			return len;
		}
	}
	return bytes;
}

#if RZB	//���¹���ΪRZB���У����ڶ�дRFID����
//RFID�ӿڲ���UART5 ������115200
void Board_Rfid_Init() {

	/*----��ʼ��IF��Interface�ӿڣ�-----*/
	RFID_IF_CLK_EN();
	//����RX/TX����
	GPIO_PinAFConfig(RFID_IF_RX_PORT, RFID_IF_RX_SRC, RFID_IF_AF);
	GPIO_PinAFConfig(RFID_IF_TX_PORT, RFID_IF_TX_SRC, RFID_IF_AF);

	//��ʼ�����IO����
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

	/*----��ʼ��RX-----*/

	RFID_IF_RX_CLK_EN();
	GPIO_InitStructure.GPIO_Pin = RFID_IF_RX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RFID_IF_RX_PORT, &GPIO_InitStructure);
	//��ʼ��TX
	RFID_IF_TX_CLK_EN();
	GPIO_InitStructure.GPIO_Pin = RFID_IF_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RFID_IF_TX_PORT, &GPIO_InitStructure);

	/*----���ò�����-----*/
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = 115200; //115200 230400 460800��UART5 APB1 30MHZ���ʹ��460800���ٸ߳������ݸ���>1%��
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(RFID_IF, &USART_InitStructure);
	USART_Cmd(RFID_IF, ENABLE);
}

//��������ʽ��ȡ�������ݣ����سɹ���ȡ�ֽ���
uint16_t Board_Rfid_Nowait_Gets(uint8_t *key, uint16_t len) {
	static uint16_t bytes = 0;
	while (USART_GetFlagStatus(RFID_IF, USART_FLAG_RXNE) != RESET) {
		key[bytes++] = (uint8_t)RFID_IF->DR & 0xFF;
		if (bytes >= len) {
			bytes = 0;
			return len;
		}
	}
	return bytes;
}
#endif //endof RZB
//ʶ��ǰӲ���Ƿ���RZB��1--��RZB ��0--��ZG
uint8_t Board_Is_RZB() {
	uint8_t state = 0; //1--��RZB ��0--��ZG
#define RZG_CHECK_PIN			GPIO_Pin_0	//PE0
#define RZB_CHECK_PORT			GPIOE		//PE
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	/*----��ʼ��PE0����-----*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = RZG_CHECK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RZB_CHECK_PORT, &GPIO_InitStructure);

	//RZB PE0Ϊ����״̬���ڲ��������ȡ��ֵӦ��Ϊ�ߵ�ƽ
	//ZG PE0Ϊ�ӵ�״̬����ȡ��ֵӦ��Ϊ�͵�ƽ
	for (int i = 0; i < 100; i++) { //�ظ�100�ζ�ȡ
		state = GPIO_ReadInputDataBit(RZB_CHECK_PORT, RZG_CHECK_PIN);
	}
	return state;
}

int aoti(char *str) {
	int ret = 0;
	while (*str != 0) {
		ret = ret * 10;
		ret += *str - '0';
		str++;
	}
	return ret;
}
//���ɰ汾��
void Board_Version_Init() {
	const uint8_t month_table[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	uint8_t str_year[5], str_month[4], str_day[3]; //��ʵ��2016Ϊ4�ֽڻ���һ������������Ϊ5�ֽ�
	uint8_t str_date[] = __DATE__;

	memcpy(str_month, str_date, (sizeof(str_month) - 1));
	str_month[(sizeof(str_month) - 1)] = 0; //׷�ӽ�����'\0'
	//�����·�
	uint8_t month = 0;
	for (int i = 0; i < (sizeof(month_table) / 4); i++) {
		if (strcmp(month_table[i], str_month) == 0) {
			month = i + 1;
			break;
		}
	}

	memcpy(str_day, str_date + 4, (sizeof(str_day) - 1));
	str_day[(sizeof(str_day) - 1)] = 0; //׷�ӽ�����'\0'

	memcpy(str_year, str_date + 7, (sizeof(str_year) - 1));
	str_year[(sizeof(str_year) - 1)] = 0; //׷�ӽ�����'\0'

	int year;
	year = aoti(str_year);

	int day;
	day = aoti(str_day);

	//��8λΪDAY��������4λΪmonth���ӵ�12λ��ʼΪ��,��ռ����12λ
	board_SW_Version = (year << 12) | (month << 8) | day;

}
void Board_Version_Print(uint32_t ver) {
	int year, month, day;
	year = (ver >> 12) & 0xfff;
	month = (ver >> 8) & 0xf;
	day = ver & 0xff;

	printf("%d%d%d\n", year, month, day);
}

uint32_t Board_Version_Get() {
	return board_SW_Version;
}
//���汾���Ƿ�Ϸ���1--�Ϸ���0--���Ϸ�
uint8_t Board_Version_Check(uint32_t ver) {

	uint32_t tmp=(ver & BOARD_VERSION_MASK);
	if ((tmp> 0) && (tmp<BOARD_VERSION_MASK))
		return 1;
	else
		return 0;
}
