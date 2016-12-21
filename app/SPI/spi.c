/**
 *@file spi.c
 *@brief STM32 SPI��������
 *@details STM32 SPI ����
 *@author lxm
 *@date 2016.10.11
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX��˾
 *@par �޸���־
 *      0.1.0 xx 2016.10.11 �����ļ�
 **/

#include "spi.h"

//�洢SPIX�����ò���
static SPI_InitTypeDef SPI_InitStructure[3];

/**
 *@name SPI1_Init
 *@brief ��ʼ��SPI1,δ��CS��ʼ�����ã���Ҫ��������CS��
 *@detials 
 *@param[in] ��
 *@param[out] ��
 *@retval[OK] �ɹ�
 *@retval[ERROR] ʧ��
 **/
void SPI1_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	//����AHB_ENABLE
	SPI1_AHB_EN();

	//����CLK_ENABLE
	SPI1_CLK_EN();

	//����PORT

	GPIO_PinAFConfig(SPI1_SCK_PORT, SPI1_SCK_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI1_MISO_PORT, SPI1_MISO_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI1_MOSI_PORT, SPI1_MOSI_SOURCE, GPIO_AF_SPI1);

	//����PIN
	//SCK
	GPIO_InitStructure.GPIO_Pin = SPI1_SCK ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����;
	GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStructure);
	//MISO
	GPIO_InitStructure.GPIO_Pin = SPI1_MISO;
	GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStructure);
	//MOSI
	GPIO_InitStructure.GPIO_Pin = SPI1_MOSI;
	GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStructure);


	//����SPI1
	SPI_InitStructure[0].SPI_Direction = SPI_Direction_2Lines_FullDuplex; //����Ϊ����ȫ˫��
	SPI_InitStructure[0].SPI_Mode = SPI_Mode_Master; //����Ϊ��ģʽ
	SPI_InitStructure[0].SPI_DataSize = SPI_DataSize_8b; //SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure[0].SPI_CPOL = SPI_CPOL_Low; //����ʱ���ڲ�����ʱ��ʱ��Ϊ�͵�ƽ
	SPI_InitStructure[0].SPI_CPHA = SPI_CPHA_1Edge; //��һ��ʱ���ؿ�ʼ��������
	SPI_InitStructure[0].SPI_NSS = SPI_NSS_Soft; //NSS�ź��������ʹ��SSIλ������
	SPI_InitStructure[0].SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; //SPI������Ԥ��ƵֵΪ8
	SPI_InitStructure[0].SPI_FirstBit = SPI_FirstBit_MSB; //���ݴ����MSBλ��ʼ
	SPI_InitStructure[0].SPI_CRCPolynomial = 7; //CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure[0]); //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI2�Ĵ���

	//ʹ��SPI1
	SPI_Cmd(SPI1, ENABLE);

}

/**
 *@name SPI2_Init
 *@brief ��ʼ��SPI2
 *@detials 
 *@param[in] ��
 *@param[out] ��
 *@retval[OK] �ɹ�
 *@retval[ERROR] ʧ��
 **/
void SPI2_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	//����AHB_ENABLE
	SPI2_AHB_EN();

	//����CLK_ENABLE
	SPI2_CLK_EN();

	//����PORT
	GPIO_PinAFConfig(SPI2_GPIO_PORT, SPI2_SCK_SOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(SPI2_GPIO_PORT, SPI2_MISO_SOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(SPI2_GPIO_PORT, SPI2_MOSI_SOURCE, GPIO_AF_SPI2);

	//����PIN
	GPIO_InitStructure.GPIO_Pin = SPI2_SCK | SPI2_MISO | SPI2_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //GPIO_PuPd_NOPULL;
	GPIO_Init(SPI2_GPIO_PORT, &GPIO_InitStructure);

	//����SPI2
	SPI_InitStructure[1].SPI_Direction = SPI_Direction_2Lines_FullDuplex; //����Ϊ����ȫ˫��
	SPI_InitStructure[1].SPI_Mode = SPI_Mode_Master; //����Ϊ��ģʽ
	SPI_InitStructure[1].SPI_DataSize = SPI_DataSize_8b; //SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure[1].SPI_CPOL = SPI_CPOL_Low; //����ʱ���ڲ�����ʱ��ʱ��Ϊ�͵�ƽ
	SPI_InitStructure[1].SPI_CPHA = SPI_CPHA_1Edge; //��һ��ʱ���ؿ�ʼ��������
	SPI_InitStructure[1].SPI_NSS = SPI_NSS_Soft; //NSS�ź��������ʹ��SSIλ������
	SPI_InitStructure[1].SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; //SPI������Ԥ��ƵֵΪ8��SPI2����30MHZ
	SPI_InitStructure[1].SPI_FirstBit = SPI_FirstBit_MSB; //���ݴ����MSBλ��ʼ
	SPI_InitStructure[1].SPI_CRCPolynomial = 7; //CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure[1]); //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI2�Ĵ���

	//ʹ��SPI2����
	SPI_Cmd(SPI2, ENABLE);
}

/**
 *@name SPI2_WriteReadByte
 *@brief SPI2дһ���ֽ����ݺ��ȡһ���ֽ�����
 *@detials 
 *@param[in] ��
 *@param[out] ��
 *@retval[OK] �ɹ�
 *@retval[ERROR] ʧ��
 **/
#if 0
u8 SPIx_WriteReadByte(SPI_TypeDef* SPIx, u8 TxData) {
	u8 retry = 0;
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) //���ͻ����־λΪ��
	{
		retry++;
		if (retry > 200)
			return 0;
	}
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPIx, TxData); //����һ������
	retry = 0;
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) //���ջ����־λ��Ϊ��
	{
		retry++;
		if (retry > 200)
			return 0;
	}
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPIx); //���ؽ�������
}
#else
u8 SPIx_WriteReadByte(SPI_TypeDef* SPIx, u8 TxData) {
	u8 ret;
	while(RESET==(SPIx->SR &SPI_I2S_FLAG_TXE));
	SPI_I2S_SendData(SPIx, TxData); //����һ������

	while(RESET==(SPIx->SR &SPI_I2S_FLAG_RXNE));
	ret=SPI_I2S_ReceiveData(SPIx); //���ؽ�������
	return ret;
}
#endif

u8 SPIx_Read(SPI_TypeDef* SPIx) {
	u8 retry = 0;
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) //���ջ����־λ��Ϊ��
	{
		retry++;
		if (retry > 200)
			return 0;
	}
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPIx);
}

/**
 *@name SPIx_PowerOn
 *@brief ʹ��SPIx������CLK�Ӷ���SPIx
 *@detials
 *@param[in] ��
 *@param[out] ��
 *@retval[OK] �ɹ�
 *@retval[ERROR] ʧ��
 **/
void SPIx_PowerOn(SPI_TypeDef* SPIx) {
	if (SPIx == SPI1)
		SPI1_CLK_EN();
	if (SPIx == SPI2)
		SPI2_CLK_EN();
}

/**
 *@name SPIx_PowerOff
 *@brief ����SPIx������CLK�Ӷ��ر�SPIx
 *@detials
 *@param[in] ��
 *@param[out] ��
 *@retval[OK] �ɹ�
 *@retval[ERROR] ʧ��
 **/
void SPIx_PowerOff(SPI_TypeDef* SPIx) {
	if (SPIx == SPI1)
		SPI1_CLK_DEN();
	if (SPIx == SPI2)
		SPI2_CLK_DEN();
}
/**
 *@name SPIx_MasterSpeed
 *@brief ����SPIx����ʱ��Ƶ�ʣ�ֻ�޶�masterģʽ��Ч��
 *@detials  **STM32 SPIӲ��������SPIʱ��Ƶ��**
 *@detials  SPI1--����APB2���ߣ�APB2�������60MHz���ϣ�SPI1ʱ�����30MHz
 *@detials  SPI2/3--����APB1���ߣ�APB1�������30MHz���ϣ�SPI2/3ʱ�����15MHz
 *@detials  SPI1/2/3--��Ƶֵ���Ϊ2
 *@param[in] SPIx--SPI1/2/3
 *@param[in] prescaler--��Ƶֵ
 *@retval[0] �ɹ�
 *@retval[-1] ʧ��
 **/
int SPIx_MasterSpeed(SPI_TypeDef* SPIx, uint16_t prescaler) {
	SPI_InitTypeDef *spi_init;

	//�쳣��Ƶֱֵ���˳�
	if (!IS_SPI_BAUDRATE_PRESCALER(prescaler))
		return -1;

	if (SPI1 == SPIx)
		spi_init = &SPI_InitStructure[0];
	else if (SPI2 == SPIx) {
		spi_init = &SPI_InitStructure[1];
	} else if (SPI3 == SPIx) {
		spi_init = &SPI_InitStructure[2];
	} else {
		spi_init = 0;
	}
	//�쳣������
	if (0 != spi_init) {

		//����SPI2����
		SPI_Cmd(SPIx, DISABLE);

		//�޸�����
		spi_init->SPI_BaudRatePrescaler = prescaler; //SPI������Ԥ��ƵֵΪ8
		SPI_Init(SPIx, spi_init); //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI2�Ĵ���

		//ʹ��SPI2����
		SPI_Cmd(SPIx, ENABLE);
		return 0; //�ɹ�
	}

	return -1;
}
//ֱ���޸Ĳ����ʼĴ����ķ�ʽ����SPI���ٶ�
void SPIx_MasterSetCR(SPI_TypeDef* SPIx,uint16_t cr)
{
	SPIx->CR1=cr;
}
uint16_t SPIx_MasterGetCR(SPI_TypeDef* SPIx)
{
	return SPIx->CR1;
}
