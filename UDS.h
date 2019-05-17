#ifndef _UDS_H_
#define _UDS_H_

#define UDS_TRIG_PORT B
#define UDS_TRIG_PIN 15
#define UDS_ECHO_PORT B
#define UDS_ECHO_PIN 14

//Pin assignment
//PB15 -- ULTRA_TRIG --  TIM1_CH3N
//PB14 -- ULTRA_ECHO --	 TIM15_CH1

void UDSInit(void);
void UDSTrig(void);
void UDSEcho(void);
void TIM15_IRQHandler(void);
void beeperControl(void);
uint32_t walkingAverage(uint32_t current_value);

#endif
