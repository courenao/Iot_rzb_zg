
#ifndef DELAY_H
#define DELAY_H

#include "stm32f2xx.h"


#define SYSTICK_MAX_VALUE 120000 //ϵͳTick�����������ֵ������SystemCoreClock / SYSTEM_CLK_PRESCALE
//ϵͳTickƵ��ֵ��1000Ϊ1000Hz����Ϊ1msһ�μ�ʱ��
#define SYSTEM_CLK_PRESCALE 1000

#define DELTA_TICKS(now,pre)   (now>pre? now-pre : now+120000-pre)

void TimingInc();
void delayMS(uint32_t ms);
uint32_t GetTick();
uint32_t GetUsFromTicks(uint32_t ticks);
uint32_t GetTimeUS();




#endif //ENDOF DELAY_H

