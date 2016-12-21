#include "delay.h"
static __IO uint32_t uTickMs; //MCU Tick��

void TimingInc() {
	uTickMs++;
}
//��ϵͳTick��ֵת��usΪ��λ��ʱ��ֵ
uint32_t GetUsFromTicks(uint32_t ticks) {
	uint32_t ret;

	if (120000000 == SystemCoreClock) { //�Ż�����
		ret = ticks / 120;
	} else { //���ܼ���������
		ret = 1000 * ticks / SYSTICK_MAX_VALUE; //us
	}
	return ret;
}
//��ȡusΪ��λ��ʱ��ֵ������ʾ2^32/1000/1000��
uint32_t GetTimeUS() {
	uint32_t tms, ticks; //����msֵ
	ticks = GetTick();
	tms = uTickMs;

	return GetUsFromTicks(ticks) + tms * 1000;
}
//��ȡmsΪ��λ��ʱ��ֵ
uint32_t GetTimeMS() {
	return uTickMs;
}

//ϵͳTick��������/(SYSTICK_MAX_VALUE)��Ϊms��
uint32_t GetTick() {
	return SYSTICK_MAX_VALUE-SysTick->VAL;//VALΪ������
}

//ʹ��STM32��SysTick���о�ȷʱ���ӳ�
void SysTick_Init() {
	//���ó�1msһ��Tick
	SysTick_Config(SYSTICK_MAX_VALUE);
	//��ʼ��uTick
	uTickMs = 0;
}

//�ӳٺ��룬�����ظ����ã�������ʽ�����ɷ��ж���ִ��
void delayMS(uint32_t ms) {
	uint32_t timingdelay;
	timingdelay = uTickMs + ms;
	while (uTickMs < timingdelay) {
	}
}

//ϵͳʱ���жϷ�������ж����ȼ����
void SysTick_Handler(void) {
	TimingInc();
}
//�ӳ�US������Ƕ�׵��ã�������ʽ�����ɷ����ж�
void delayUS(uint32_t us){
	uint32_t t;
	t=GetTimeUS()+us;
	while(GetTimeUS()<t){

	}
}
