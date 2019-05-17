#ifndef _SYSTICK_H
#define _SYSTICK_H
#include "stm32f303xe.h"

void SysTick_Initialize (uint32_t ticks);
void SysTick_Handler (void);
void Delay_10us (uint32_t nTime);
void Delay_ms(uint32_t nTime);

#endif
