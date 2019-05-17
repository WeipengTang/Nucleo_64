/******************************************************************************
*DCMotor.h -- Interface of drive two DC motors
*Weipeng
*Mar 28, 2019
*The code is modified from the beeper code
*Anywhere in the DCMotor.h and DCMotor.c, Motor_A = Motor_Left = Motor_L = Motor_1 
																					Motor_B = Motor_Right = Motor_R = Motor_2
********************************************************************************/
#ifndef _DCMOTOR_H
#define	_DCMOTOR_H
#include "stm32f303xe.h"

#define MOTOR_PWM_LEFT_PORT A
#define MOTOR_DIRA_LEFT_PORT A
#define MOTOR_DIRB_LEFT_PORT A
#define MOTOR_PWM_RIGHT_PORT A
#define MOTOR_DIRA_RIGHT_PORT B
#define MOTOR_DIRB_RIGHT_PORT B

#define MOTOR_PWM_LEFT_PIN 8
#define MOTOR_DIRA_LEFT_PIN 0
#define MOTOR_DIRB_LEFT_PIN 1
#define MOTOR_PWM_RIGHT_PIN 9
#define MOTOR_DIRA_RIGHT_PIN 0
#define MOTOR_DIRB_RIGHT_PIN 1

//pin assignment
//DCM1_PWM	--PA8	--TIM1_CH1
//DCM1_DIRA	--PA0
//DCM1_DIRB	--PA1
//DCM2_PWM	--PA9	--TIM2_CH2
//DCM2_DIRA	--PB0
//DCM2_DIRB	--PB1

void DCMotorInit(void);
void runDCMotor(int8_t DIR_L, uint32_t SPD_L, int8_t DIR_R, uint32_t SPD_R);	//setting either direction or speed to 0 can stop the motor, but better use speed for easier interrupt operation
//uint32_t controlLawFunction(int32_t speedErrorIntegral, uint32_t PWM, uint32_t setPoint, uint32_t encoderVanePeriod);
void TIM3_IRQHandler(void);
uint32_t speedControl(uint32_t target_speed, uint32_t PWM, int32_t current_period_ticks);
#endif
