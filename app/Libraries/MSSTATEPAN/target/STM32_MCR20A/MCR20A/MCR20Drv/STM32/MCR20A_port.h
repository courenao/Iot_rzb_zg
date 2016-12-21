/*******************************************************************************
 * @�ļ�    SPI.c
 * @����  LiXianmao
 * @�汾 V0.0.1
 * @����    2016.9.13
 * @����   mcr20a SPI�豸�����ļ�
 * @��ע  �ӿ���instanceָ����ΪSPI�˿ڣ�����MCR20A�̶�ΪSPI1���instance����Ҫ��
 *        ����Ϊ�˱�֤��ֲ�ļ��ݣ�instance���Ǳ����ˣ�����Ҫ�������κδ���
 *******************************************************************************/

#ifndef __XCVR_SPI_H__
#define __XCVR_SPI_H__

/*! *********************************************************************************
 *************************************************************************************
 * Include
 *************************************************************************************
 ********************************************************************************** */

#include "EmbeddedTypes.h"
#include "spi.h"

void spi_master_init(uint32_t instance);
void spi_master_configure_speed(uint32_t instance, uint32_t freq);
void spi_master_transfer(uint32_t instance, uint8_t * sendBuffer,
		uint8_t * receiveBuffer, int16_t transferByteCount);

void spi_cs_enable();
void spi_cs_disable();

void MCR20A_IRQ_Init();
void MCR20A_IRQ_Disable();
void MCR20A_IRQ_Enable();

/*----------------SPI�ӿڶ���--------------------------------*/
//NRF24L01 SPI�ӿ�CS�ź�
#define SPI_MCR20A_CS_PIN			GPIO_Pin_4	//PA4,���
#define SPI_MCR20A_CS_PORT			GPIOA
#define SPI_MCR20A_CS_PIN_EN()		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define SPI_MCR20A_CS_LOW() 		GPIO_ResetBits(SPI_MCR20A_CS_PORT,SPI_MCR20A_CS_PIN)
#define SPI_MCR20A_CS_HIGH() 		GPIO_SetBits(SPI_MCR20A_CS_PORT,SPI_MCR20A_CS_PIN)

//��д
#define SPI_MCR20A_WR(x)			SPIx_WriteReadByte(SPI1,x)
#define SPI_MCR20A_RD()   			SPIx_Read(SPI1)
#define SPI_MCR20A_Init()			SPI1_Init()
#define SPI_MCR20A_Speed(x)			SPIx_MasterSpeed(SPI1,x)

#define SPI_MCR20A_CR1_SET(x) 		SPIx_MasterSetCR(SPI1,x)
#define SPI_MCR20A_CR1_GET()  		SPIx_MasterGetCR(SPI1)

/*----------------IRQ�жϽӿڶ���--------------------------------*/

#define MCR20A_IRQ_PRIO_EMPTION		1	//�����ȼ�����ռ���ȼ�
#define MCR20A_IRQ_PRIO_SUB			0	//�����ȼ���

#if RZG
#define MCR20A_IRQ_PIN				GPIO_Pin_9	//PC9������
#define MCR20A_IRQ_PORT				GPIOC
#define MCR20A_IRQ_PIN_EN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE)

#define MCR20A_IRQ_EXTI_LINE		EXTI_Line9
#define MCR20A_IRQ_EXTI_PORT		GPIOC
#define MCR20A_IRQ_EXTI_IRQn		EXTI9_5_IRQn
#define MCR20A_IRQ_LINE_CFG()		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, GPIO_PinSource9)



#define MCR20A_IRQ_Entry			EXTI9_5_IRQHandler 	//ֱ�ӹ�����stm32f207xx.s�ļ����жϴ�����
#define MCR20A_IRQ_CLEAR()			EXTI_ClearITPendingBit(MCR20A_IRQ_EXTI_LINE)


#define MCR20A_RESET_PIN			GPIO_Pin_15//PB15
#define MCR20A_RESET_PORT			GPIOB
#define MCR20A_RESET_PIN_EN()       RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE)
#define MCR20A_RESET_ASSERT()		GPIO_ResetBits(MCR20A_RESET_PORT,MCR20A_RESET_PIN)	//�͵�ƽ��λ��Ч
#define MCR20A_RESET_DEASSERT()		GPIO_SetBits(MCR20A_RESET_PORT,MCR20A_RESET_PIN)
#else

#define MCR20A_IRQ_PIN				GPIO_Pin_1	//PA1������
#define MCR20A_IRQ_PORT				GPIOA
#define MCR20A_IRQ_PIN_EN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)

#define MCR20A_IRQ_EXTI_LINE		EXTI_Line1
#define MCR20A_IRQ_EXTI_PORT		GPIOA
#define MCR20A_IRQ_EXTI_IRQn		EXTI1_IRQn
#define MCR20A_IRQ_LINE_CFG()		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, GPIO_PinSource1)


#define MCR20A_IRQ_Entry			EXTI1_IRQHandler 	//ֱ�ӹ�����stm32f207xx.s�ļ����жϴ�����
#define MCR20A_IRQ_CLEAR()			EXTI_ClearITPendingBit(MCR20A_IRQ_EXTI_LINE)


#define MCR20A_RESET_PIN			GPIO_Pin_2//PA2
#define MCR20A_RESET_PORT			GPIOA
#define MCR20A_RESET_PIN_EN()       RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define MCR20A_RESET_ASSERT()		GPIO_ResetBits(MCR20A_RESET_PORT,MCR20A_RESET_PIN)	//�͵�ƽ��λ��Ч
#define MCR20A_RESET_DEASSERT()		GPIO_SetBits(MCR20A_RESET_PORT,MCR20A_RESET_PIN)

#endif

/*----------------RNG���������������--------------------------------*/

#define MCR20A_MCU_RNG_ENABLE  		1 	//ʹ��MCU�Դ���RNG�����������

#ifdef MCR20A_MCU_RNG_ENABLE
void MCR20A_MCU_RNG_Init();
uint8_t MCR20A_MCU_GetRNG();
#endif

#endif /* __XCVR_SPI_H__ */
