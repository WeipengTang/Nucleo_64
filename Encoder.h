#ifndef _ENCODER_H_
#define _ENCODER_H_

#define ENC_LEFT_PORT A
#define ENC_LEFT_PIN 6
#define ENC_RIGHT_PORT A
#define ENC_RIGHT_PIN 7

#define MAX_PULSE_WIDTH 100000

//Pin assignment
//PA6 - TIM3_CH1(AF2) or TIM16_CH1(AF1) - ENC_A_GPIO
//PA7 - TIM3_CH2(AF2) or TIM17_CH1(AF1) - ENC_B_GPIO


void EncoderInit(void);
void TIM16_IRQHandler(void);
void TIM17_IRQHandler(void);




#endif

