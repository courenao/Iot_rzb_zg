#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/**
 *@file board_rzg.c
 *@brief RZG��Ӳ���ӿڶ���(MCU-STM32F207ZG)
 *@author lxm
 *@date 2016.10.13
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX��˾
 *@par �޸���־
 *      0.1.0 xx 2016.10.11 �����ļ�
 **/

/*---------------------RZGӲ������-------------------------
 *@details RZGӲ���嵥��
 *@details 1.UART4-��Ϊ���Դ���(���xprintf.h)
 *@details   TX  --PC10
 *@details   RX  --PC11
 *@details 2.SPI1-��MCR20A��ƵоƬͨѶ�ӿ�(���spi.h MCR20A_port.h)
 *@details   SCK --PA5
 *@details   MOSI--PB5
 *@details   MISO--PA6
 *@details   CS  --PA4
 *@details 3.EXTI_Line9--MCR20A��ƵоƬ�жϽӿ�(���MCR20A_port.h)
 *@details   IRQ --PC9
 *@details   EXTI9_5_IRQn
 *@details   EXTI9_5_IRQHandler
 *@details 4.RST -MCR20A��ƵоƬ��λ����
 *@details   RST --PB15���͵�ƽ��Ч���ߵ�ƽ��Ч��
 *@details 5.LED-ָʾ��
 *@details   ALARM-PB7  �澯��
 *@details   RUN --PB8  ϵͳ����ָʾ��
 *@details 6.GPIOA9--RS232_EN����ʹ������
 *@details   RS232_EN--PA9
 *@details 7.ETH-��̫���ӿ�
 *@details   MDIO--PA2
 *@details   MDC--PC1
 *@details   TX_CLK--PC3
 *@details   TX_EN--PB11
 *@details   TXD_0--PB12
 *@details   TXD_1--PB13
 *@details   TXD_2--PC2
 *@details   TXD_3--PE2
 *@details   RX_CLK--PA1
 *@details   RX_DV--PA7
 *@details   CRS--PA0
 *@details   RX_ER--PB10
 *@details   COL--PA3
 *@details   RXD_0--PC4
 *@details   RXD_1--PC5
 *@details   RXD_2--PB0
 *@details   RXD_3--PB1
 *@details   RST_CPU--PB15
 *@details   RESET--PC6
 *@details   POWR_DOWN--PB14
 *@details
 *---------------------RZBӲ������-------------------------
 *@details RZBӲ���嵥��
 *@details 1.UART4-��Ϊ���Դ���(���xprintf.h)
 *@details   TX  --PC10
 *@details   RX  --PC11
 *@details 2.SPI1-��MCR20A��ƵоƬͨѶ�ӿ�(���spi.h MCR20A_port.h)
 *@details   SCK --PA5
 *@details   MOSI--PA7
 *@details   MISO--PA6
 *@details   CS  --PA1
 *@details 3.EXTI_Line1--MCR20A��ƵоƬ�жϽӿ�(���MCR20A_port.h)
 *@details   IRQ --PC9
 *@details   EXTI1_IRQn
 *@details   EXTI1_IRQHandler
 *@details 4.RST -MCR20A��ƵоƬ��λ����
 *@details   RST --PA2���͵�ƽ��Ч���ߵ�ƽ��Ч��
 *@details 5.LED-ָʾ��
 *@details   ALARM-PB7  �澯��
 *@details   RUN --PB8  ϵͳ����ָʾ��
 *@details 6.SPI2--��nRF24L01ͨѶ�ӿ�
 *@details   SCK --PB13
 *@details   MOSI--PB14
 *@details   MISO--PB15
 *@details   CSN  --PB12
 *@details 7.CE --nRF24L01ʹ�ܿ�������
 *@details 	 CE  --PA3
 *@details 8.GPIOA8--nRF24L01�жϣ�����Ҫ���жϣ�ֻ����ͨ����IO��
 *@details   IRQ����-PA8
 *@details 9.GPIOA9--RS232_EN����ʹ������
 *@details   RS232_EN--PA9
 *@details 10.TIM2--��������PWM����ʱ��
 *@details    PWM���--PE9
 *@details 11.UART5--nRF518822 RFID������
 *@details    RX--- PD2
 *@details    TX--- PC12
 */

//#define RZG 			1	//0--RZG�壬1--RZB��

//#define BOOT_LOADER		1 	//1--��ʾ����������0--Ӧ�ó���

#if BOOT_LOADER
#else
#if !RZG	//��Ϊ�ڵ�
/*---------RZB��Ϊ�ڵ�--------*/
#define RZB 			1
//�ڵ��־������ֻ�����ҽ���һ��Ϊ1
#define RFD_0 			1		//�ڵ�0
#define RFD_1 			0		//�ڵ�1
#define RFD_2 			0		//�ڵ�2
#define RFD_3 			0		//�ڵ�3
#else 	//��Ϊ����
#define USE_INTERNET 	1
#define RZB 0
#endif //ENDOF !RZG

#endif //ENDOF BOOT_LOADER


/*---����ʹ��---*/
#define TEST 			1
/*----�豸ZigBee/��̫��ģ�������ַ����̫����ַȡZigBee��ַ��ǰ6���ֽڣ�  ---*/

#define USE_AUTO_MAC 	1		//ʹ���Զ�����MAC��ַ

#if RZG
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x00
#else
#if RFD_0	//RFD0
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x01
#elif RFD_1 	//RFD1
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x02
#elif RFD_2
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x03
#else //RFD3
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x67
#endif

#endif //if RZG

/*------���ݼ��ܿ���-------*/
#define DATA_ENCRYPTION_ENABLE     0 //1--ʹ�����ݼ��ܣ�0--�������ݼ���
/*-------��̫������---*/

#define BOARD_IP_0				192
#define BOARD_IP_1 				168
#define BOARD_IP_2				10
#define BOARD_IP_3				111

#define BOARD_NETMASK_0			255
#define BOARD_NETMASK_1			255
#define BOARD_NETMASK_2			254
#define BOARD_NETMASK_3			0

#define BOARD_GW_0				192
#define BOARD_GW_1				168
#define BOARD_GW_2				10
#define BOARD_GW_3				1

#define BOARD_IP_SERVER_0		192
#define BOARD_IP_SERVER_1		168
#define BOARD_IP_SERVER_2		10
#define BOARD_IP_SERVER_3		10

#define UDP_SEND_PORT     		1031
#define UDP_RECV_PORT			1030

/*--------����汾----------*/

#define DEVICE_SN				0x00000001	//�豸���к�
#if BOOT_LOADER
#define VER_FW_DF				201			//�̼��汾��,
#define VER_SW_DF				000			//����汾�ţ�Ĭ��ֵ

#define BOOT_MD5_CHECK			1			//����MD5����У��
#else
#define VER_FW_DF				000			//�̼��汾��,Ĭ��ֵ
#define VER_SW_DF				212			//����汾��
#endif

#define DEBUG_PRINT				1			//���Դ�ӡ

#define DEBUG_SHELL				0			//����SHELL�򿪿��أ�1--����Shell���ܣ�0--�ر�Shell����

#endif//  BOARD_CONFIG_H
