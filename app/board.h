/**
 *@file board.c
 *@brief ��Ӳ���ӿڶ���(MCU-STM32F207ZG)
 *@author lxm
 *@date 2016.10.13
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX��˾
 *@par �޸���־
 *      0.1.0 xx 2016.10.11 �����ļ�
 *@details ��ϸӲ���嵥���board_config.h
 **/
#ifndef BOARD_H
#define BOARD_H

#include "stm32f2xx.h"
#include "board_config.h"

/*------------------COM(���Դ���)����--------------------------*/
typedef enum {
	COM1 = 0, COM2 = 1, COMn //���һ�����ڼ����ܸ����������޸ģ����ұ�֤ENUM�е�һ��ֵΪ0
} COM_TypeDef;

/**
 * @brief Definition for COM port1, connected to USART3
 */
#define EVAL_COM1                        USART1
#define EVAL_COM1_STR                    "USART1"
#define EVAL_COM1_CLK                    RCC_APB2Periph_USART1
#define EVAL_COM1_TX_PIN                 GPIO_Pin_9
#define EVAL_COM1_TX_GPIO_PORT           GPIOA
#define EVAL_COM1_TX_GPIO_CLK            RCC_AHB1Periph_GPIOA
#define EVAL_COM1_TX_SOURCE              GPIO_PinSource9
#define EVAL_COM1_TX_AF                  GPIO_AF_USART1
#define EVAL_COM1_RX_PIN                 GPIO_Pin_10
#define EVAL_COM1_RX_GPIO_PORT           GPIOA
#define EVAL_COM1_RX_GPIO_CLK            RCC_AHB1Periph_GPIOA
#define EVAL_COM1_RX_SOURCE              GPIO_PinSource10
#define EVAL_COM1_RX_AF                  GPIO_AF_USART1
#define EVAL_COM1_IRQn                   USART1_IRQn

/**
 * @brief Definition for COM port2, connected to USART3
 */

#if (RZG||RZB||BOOT_LOADER)		//RZG/RZB��ʹ��  COM2--USART4��Ϊ���ڴ�ӡ
#define EVAL_COM2                        UART4
#define EVAL_COM2_STR                    "UART4"
#define EVAL_COM2_CLK                    RCC_APB1Periph_UART4
#define EVAL_COM2_TX_PIN                 GPIO_Pin_10	//PC10
#define EVAL_COM2_TX_GPIO_PORT           GPIOC
#define EVAL_COM2_TX_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define EVAL_COM2_TX_SOURCE              GPIO_PinSource10
#define EVAL_COM2_TX_AF                  GPIO_AF_UART4
#define EVAL_COM2_RX_PIN                 GPIO_Pin_11	//PC11
#define EVAL_COM2_RX_GPIO_PORT           GPIOC
#define EVAL_COM2_RX_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define EVAL_COM2_RX_SOURCE              GPIO_PinSource11
#define EVAL_COM2_RX_AF                  GPIO_AF_UART4
#define EVAL_COM2_IRQn                   UART4_IRQn

//����ʹ������
#define EVAL_COM2_EN_PIN				GPIO_Pin_9	//PA9
#define EVAL_COM2_EN_PORT				GPIOA
#define EVAL_COM2_EN_CLKEN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)

#else									//COM2--USART3
#define EVAL_COM2                        USART3
#define EVAL_COM2_STR                    "USART3"
#define EVAL_COM2_CLK                    RCC_APB1Periph_USART3
#define EVAL_COM2_TX_PIN                 GPIO_Pin_10
#define EVAL_COM2_TX_GPIO_PORT           GPIOB
#define EVAL_COM2_TX_GPIO_CLK            RCC_AHB1Periph_GPIOB
#define EVAL_COM2_TX_SOURCE              GPIO_PinSource10
#define EVAL_COM2_TX_AF                  GPIO_AF_USART3
#define EVAL_COM2_RX_PIN                 GPIO_Pin_11
#define EVAL_COM2_RX_GPIO_PORT           GPIOB
#define EVAL_COM2_RX_GPIO_CLK            RCC_AHB1Periph_GPIOB
#define EVAL_COM2_RX_SOURCE              GPIO_PinSource11
#define EVAL_COM2_RX_AF                  GPIO_AF_USART3
#define EVAL_COM2_IRQn                   USART3_IRQn
#endif

/*------------------LED����--------------------------*/
typedef enum {
	ALARM = 0, //�澯ָʾ��
	RUN, //ϵͳ����ָʾ��

	LEDn //���һ�����ڼ���LED�ܸ����������޸ģ����ұ�֤ENUM�е�һ��ֵΪ0
} Led_TypeDef;

#define LED_ALARM_PIN                    GPIO_Pin_7	//PB7
#define LED_ALARM_PORT                   GPIOB
#define LED_ALARM_CLK                    RCC_AHB1Periph_GPIOB

#define LED_RUN_PIN						 GPIO_Pin_8	//PB8
#define LED_RUN_PORT					 GPIOB
#define LED_RUN_CLK						 RCC_AHB1Periph_GPIOB

/*------------------����������--------------------------*/
#define PWM_PIN 						GPIO_Pin_9	//PE9
#define PWM_PORT 						GPIOE
#define PWM_CLK							RCC_AHB1Periph_GPIOE
#define PWM_PIN_SOURCE                  GPIO_PinSource9

#define PWM_LOW()  						GPIO_ResetBits(PWM_PORT, PWM_PIN)
#define PWM_HIGH()  					GPIO_SetBits(PWM_PORT, PWM_PIN)

#define PWM_USE_TIM  					1	//ʹ�ö�ʱ��TIM1����PWM����������
#define PWM_TIM_CLK_EN()     			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)// PE9Ӳ����TIM1_CH1��
#define PWM_TIM_PIN_CONNECT()			GPIO_PinAFConfig(PWM_PORT, GPIO_PinSource9, GPIO_AF_TIM1)
#define PWM_TIM  						TIM2
#define PWM_TIM_IRQn					TIM2_IRQn
#define PWM_TIM_IRQ_PRIO_EMPTION 		2
#define PWM_TIM_IRQ_PRIO_SUB			1
#define Board_PWM_IRQHandler			TIM2_IRQHandler

/*------------------RTC����--------------------------*/
#define RTC_IRQ_LINE_TS				EXTI_Line21	//RTC Tamper��Stamper�ж�������Ϊ12
#define RTC_IRQ_CHANNEL_TS			TAMP_STAMP_IRQn //RTC LIN12�ж�ͨ��
#define RTC_IRQHandler_TS			TAMP_STAMP_IRQHandler//RTC Tamper��STamper�ж�
#define RTC_IRQ_PRIO_EMPTION_TS   	5	//�����ȼ�,��ռ
#define RTC_IRQ_PRIO_SUB_TS			6	//�����ȼ�,ʱ��Ƭ
#define RTC_IRQ_LINE_ALARM			EXTI_Line17 //RTC Alarm�ж���������17
#define RTC_IRQ_CHANNEL_ALARM		RTC_Alarm_IRQn//RTC ALARM�ж�ͨ��
#define RTC_IRQHandler_ALARM		RTC_Alarm_IRQHandler//RTC Tamper��STamper�ж�
#define RTC_IRQ_PRIO_EMPTION_ALARM  5	//�����ȼ�,��ռ
#define RTC_IRQ_PRIO_SUB_ALARM		7	//�����ȼ�,ʱ��Ƭ
/*------------------�ڲ�Flash����--------------------------*/
/*------FLASH Sector �� ���ܹ�512K��-----*/
#define FLASH_BASE_SECTOR_0		    (FLASH_BASE+0X0)		//16k
#define FLASH_BASE_SECTOR_1		    (FLASH_BASE+0X4000)		//16k
#define FLASH_BASE_SECTOR_2		    (FLASH_BASE+0X8000)		//16k
#define FLASH_BASE_SECTOR_3		    (FLASH_BASE+0Xc000)		//16k
#define FLASH_BASE_SECTOR_4		    (FLASH_BASE+0X10000)	//64k
#define FLASH_BASE_SECTOR_5		    (FLASH_BASE+0X20000)	//128k
#define FLASH_BASE_SECTOR_6		    (FLASH_BASE+0X40000)	//128k
#define FLASH_BASE_SECTOR_7		    (FLASH_BASE+0X60000)	//128k

//��ַ���뱣֤���������洢��ַ�н���,��ȡ0x40000=256*1024=256k��
//��ַ�����sector�ο�оƬ�ֲ�
#define USER_FLASH_BASE  			(FLASH_BASE+0x40000)  //sector6
#define USER_FLASH_SIZE 			0x20000				//sector6 ��СΪ128K
#define USER_FLASH_SECTOR			FLASH_Sector_6

#define USER_FLASH_MAC_BASE			USER_FLASH_BASE		//MAC��ַ���׵�ַ
/*------------------MCU�豸ID----------------------*/
#define MCU_ID_BASE					0x1FFF7A10			//ID����ַ
/*------------------��������--------------------------*/
void Board_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);
void Board_LEDInit(Led_TypeDef Led);
void Board_LedOn(Led_TypeDef Led);
void Board_LedOff(Led_TypeDef Led);
void Board_PWM_Init();
void Board_PWM_On();
#if PWM_USE_TIM
void Board_PWM_Off();
ErrorStatus Board_PWM_SetCount(uint8_t c); //������Ĵ���1~N����ʾ���1s�켸��
#endif //PWM_USE_TIM
#define FOREVER()		while(1)

/*-----------�豸��Ϣ���������ö���------------*/
typedef struct {
	uint16_t type; //��������
	uint16_t idle; //IDLE_TIME������ʱ��
	uint16_t tx; //TX_TIMEOUT���ͳ�ʱ
	uint16_t heart; //HEART_BEART_TIMEOUT������ʱ
} Network_setting_t;
#define TYPE_MASK_IDLE_TIME						(1<<12)	//IDLE_TIME
#define TYPE_MASK_TX_TIMEOUT					(1<<8)	//TX_TIMEOUT
#define TYPE_MASK_HEART_TIMEOUT					(1<<4)	//HEART_BEAT_TIMEOUT
#define TYPE_MASK_SET_DEFAULT					(1<<0)	//��ԭ��Ĭ��ֵ
#define IS_VALID_IDLE_TIME(x)  					((x)>=20)	//IDLE_TIME >=20��������
#define IS_VALID_TX_TIMEOUT(x)					((x)>1000)//���ͳ�ʱ����>1000
#define IS_VALID_HEART_TIMEOUT(heart,tx) 		((heart)>(tx)) 	//��������>���ͳ�ʱ
//�������Ĭ��ֵ
#define DEFAULT_VALUE_SET_TYPE						0x1110		//Ĭ�����ж���Ч
#define DEFAULT_VALUE_SET_IDLE						2000		//2000ms
#define DEFAULT_VALUE_SET_TX						2000		//2000ms
#define DEFAULT_VALUE_SET_HEART						5000		//5000ms
#if (USE_INTERNET||BOOT_LOADER)
typedef struct {
	uint8_t ip[4]; //��̫��IP��ַ
	uint8_t ip_mask[4];//��������
	uint8_t ip_gateway[4];//���ص�ַ
	uint8_t ip_target[4];//Ŀ��IP��ַ
	uint16_t send_port;//Ŀ��˿�
	uint16_t recv_port;//���ն˿�
}UDP_Net_Config_t;
#endif


//���뱣֤32λ��4�ֽڣ����룬��Ϊflashд������32λΪ��λ
typedef struct {
	uint32_t flag; //��ʼ����־	��0x0001--��ʾ��ʼ��������������Ч��0xffffffff--��ʾδ��ʼ������������Ч
	uint8_t mac[8]; //zigbee IEEE��ַ8�ֽڣ���̫��MACȡǰ6���ֽ�
	uint32_t sn; //��ƷSN���к�
	uint32_t ver_fw;	//�̼��汾��
	uint32_t ver_sw; 	//����汾�ţ�101��ʾ1.0.1�汾
	Network_setting_t sets; //��������
	uint32_t debug;		//�Ƿ�������ģʽ
#if (USE_INTERNET||BOOT_LOADER)//ֻ��RZG������̫��
	UDP_Net_Config_t udp;//UDP��������
#endif //endof RZG
} Device_Info_t;

#define DEVICE_INFO_VALID		0x01					//�豸��Ϣ������Ч
#define DEVICE_INFO_INVALID		(!DEVICE_INFO_VALID)	//�豸��Ϣ������Ч
/*-----------SHELL�û�����------------*/
#define COM_KEY_RESET				"232start"
#define COM_KEY_SHELL				"232start"	//�������ô������룬ֻ��������������ͨ�����ڻָ�����ֵ����Сд����
#define COM_KEY_SHELL_DEBUGON		"-v"		//����ģʽ����ӡ���е�����Ϣ
#define COM_KEY_SHELL_DEBUGOFF		"-n"		//�رյ���ģʽ����������
#define COM_KEY_SHELL_RESET 		"-r"		//�ָ���������
#define COM_KEY_SHELL_EXIT			"-q"		//�˳�SHELL
#define COM_KEY_SHELL_HELP			"-h"		//����
#define COM_KEY_SHELL_XUPDATE		"-u"		//ʹ��Xmodem��������
#define COM_KEY_SHELL_ERASE			"-e"	//ǿ�Ʋ���Ӧ�ó���
#define COM_KEY_SHELL_PRINTINFO		"-p"		//��ӡ�豸��Ϣ
#define COM_KEY_SHELL_SETINFO		"-s"		//�����豸��Ϣ

#define COM_KEY_SHELL_HELP_INFO 	"-v ����ģʽ\n\
-n �رյ���ģʽ\n\
-r �ָ���������\n\
-q �˳�DEBUG\n\
-u ʹ��Xmodem 1K��������\n\
-e ǿ�Ʋ���Ӧ�ó���\n\
-p ��ӡ�豸��Ϣ\n\
-s �����豸��Ϣ\n\
-h ��ӡ������Ϣ\n"


extern uint8_t debugFlag; //���Կ���

#define DPRINT(flag,format,args...)  do {if(flag) printf(format,##args);}while(0)
#define Dprintf(args...) DPRINT(debugFlag,##args)

/*-----------SHELL�û�����------------*/
#if RZB //����ΪRZB����
//RFID_51822�������ӿ� UART5 ,RX--PD2 ,TX--PC12
#define RFID_IF						UART5
#define RFID_IF_AF					GPIO_AF_UART5
#define RFID_IF_CLK_EN()			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE)

#define RFID_IF_RX_PIN				GPIO_Pin_2	//PD2
#define RFID_IF_RX_PORT				GPIOD
#define RFID_IF_RX_CLK_EN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE)
#define RFID_IF_RX_CLK				RCC_AHB1Periph_GPIOD
#define RFID_IF_RX_SRC				GPIO_PinSource2

#define RFID_IF_TX_PIN				GPIO_Pin_12	//PC12
#define RFID_IF_TX_PORT				GPIOC
#define RFID_IF_TX_CLK_EN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE)
#define RFID_IF_TX_CLK				RCC_AHB1Periph_GPIOC
#define RFID_IF_TX_SRC				GPIO_PinSource12
#define RFID_IF_IRQ					USART5_IRQn



void Board_Rfid_Init();
uint16_t Board_Rfid_Nowait_Gets(uint8_t *key, uint16_t len);

#endif //endof if RZB

uint8_t Board_Is_RZB();
int aoti(char *str);
void Board_Version_Init();
void Board_Version_Print(uint32_t ver);
uint32_t Board_Version_Get();

#define BOARD_VERSION_MASK		((uint32_t)0x7FFF1F)	//�����գ��Ӻ���ǰ1F--�գ����31����F-�£����12����7FF-�꣨���2046��

#endif //endof BOARD_H
