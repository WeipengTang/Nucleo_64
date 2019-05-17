/**************************************************************************************
*StepperMotor.h -- interface of initiating ports for stepper motor and run the stepper motor
*By Weipeng Tang
*Jan 24, 2019
***************************************************************************************/
#ifndef _STEPPERMOTOR_H
#define _STEPPERMOTOR_H

#define STM_PORT C
#define STM_A 6
#define STM_ANOT 7
#define STM_B 8
#define STM_BNOT 9
#define STM_SLEEP_PORT A
#define STM_SLEEP_PIN 10

#define LMSW_LEFT_PORT B
#define LMSW_LEFT_PIN 2
#define LMSW_RIGHT_PORT B
#define LMSW_RIGHT_PIN 10



#define STM_WAKE SET_BITS(GPIO(STM_SLEEP_PORT)->ODR, GPIO_ODR_(STM_SLEEP_PIN))
#define STM_SLEEP CLR_BITS(GPIO(STM_SLEEP_PORT)->ODR, GPIO_ODR_(STM_SLEEP_PIN))

void StepperMotorInit(void);
void runStepperMotor(int input); //Only pass in 0, -2, -1, 1 and 1.
void TIM4_IRQHandler(void);
void stepperExitParallel(void);

void limitSwitchInit(void);
void stepperHoming(void);

#endif
