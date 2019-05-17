/**************************************************************************************
*StepperMotor.c -- implementation of initiating ports for stepper motor and run the stepper motor
*By Weipeng Tang
*Jan 24, 2019
***************************************************************************************/
#include "stm32f303xe.h"
#include "StepperMotor.h"
#include "utilities.h"
#include "SysTick.h"
#include "Servo.h"
#define PRESCALER_NUM 89
#define ARR_NUM 9999

static uint32_t stepperValue[8] = {0x8, 0xA, 0x2, 0x6, 0x4, 0x5, 0x1, 0x9};
static uint32_t current_position = 185;
extern Instruction current_instructions;
static uint32_t stepper_max_angle = 370;
static uint32_t stepper_min_angle = 0;

void StepperMotorInit(void){

	//Enable Port C clock
	EN_CLK_PORT(STM_PORT);
	EN_CLK_PORT(STM_SLEEP_PORT);
	
	//Set Pin PC6, PC7, PC8, PC9, PA10 as output
	GPIO_PIN_MODE(STM_PORT, STM_A, OUT);
	GPIO_PIN_MODE(STM_PORT, STM_ANOT, OUT);
	GPIO_PIN_MODE(STM_PORT, STM_B, OUT);
	GPIO_PIN_MODE(STM_PORT, STM_BNOT, OUT);
	GPIO_PIN_MODE(STM_SLEEP_PORT, STM_SLEEP_PIN, OUT);
	
	//Set Pin PC6, PC7, PC8, PC9, PA10 as push-pull mode
	GPIO_PIN_DRV_TYPE(STM_PORT, STM_A, PP);
	GPIO_PIN_DRV_TYPE(STM_PORT, STM_ANOT, PP);
	GPIO_PIN_DRV_TYPE(STM_PORT, STM_B, PP);
	GPIO_PIN_DRV_TYPE(STM_PORT, STM_BNOT, PP);
	GPIO_PIN_DRV_TYPE(STM_SLEEP_PORT, STM_SLEEP_PIN, PP);
	
	//Set Pin PC6, PC7, PC8, PC9, PA10 as no-push/no-pull
	GPIO_PIN_PP_TYPE(STM_PORT, STM_A, NPP);
	GPIO_PIN_PP_TYPE(STM_PORT, STM_ANOT, NPP);
	GPIO_PIN_PP_TYPE(STM_PORT, STM_B, NPP);
	GPIO_PIN_PP_TYPE(STM_PORT, STM_BNOT, NPP);
	GPIO_PIN_PP_TYPE(STM_SLEEP_PORT, STM_SLEEP_PIN, NPP);
	
	stepperExitParallel();
	
	
	//***********************************Enable TIM4 for interrup service**************************
	//Enable the clock of timer 4 
	SET_BITS(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
	
	//set pre-scaler.72MHz/(89+1) = 800KHz.
	TIM4->PSC = PRESCALER_NUM;
	
	//set counter period. 800KHz/(9999+1) = 80Hz.																													 
	TIM4->ARR = ARR_NUM;
	
	//Select Channel 1 as output																													   
	CLR_BITS(TIM4->CCMR1, TIM_CCMR1_CC1S);  									
													
	//Select active mode. When TIM4_CNT = TIM4_CCR1, CC1IF is set high.
  FORCE_BITS(TIM4->CCMR1, TIM_CCMR1_OC1M, TIM_CCMR1_OC1M_0);																															

	//Duty cycle. May not be important here.
	TIM4->CCR1 = 500;
	
	//Main output enable
	TIM4->BDTR |= TIM_BDTR_MOE;
	
	//Select output polarity: 0 = active high, 1 = active low
	TIM4->CCER &= ~TIM_CCER_CC1P;
	
	//Enable output for channel 1 output
	TIM4->CCER |= TIM_CCER_CC1E;	 
	
	//Enable capture/compare 1 interrupt
	SET_BITS(TIM4->DIER, TIM_DIER_CC1IE);
	
	//Enable timer 4
	TIM4->CR1 |= TIM_CR1_CEN;
	/********************************Enable Interrupt****************************************/
	//Set the interrupt priority of TIM4_IRQn 
	NVIC_SetPriority(TIM4_IRQn, 2);															
	
	//Enable the interrupt TIM4_IRQn
	NVIC_EnableIRQ(TIM4_IRQn);																	
	

}//End of initStepperMotor()
void stepperExitParallel(void){
	SET_BITS(GPIO(STM_PORT)->ODR, GPIO_ODR_(STM_A));
	SET_BITS(GPIO(STM_PORT)->ODR, GPIO_ODR_(STM_ANOT));
		
	STM_SLEEP;
	Delay_ms(100);
	STM_WAKE; 						//wake up stepper motor driver
}


void limitSwitchInit(void){
	//*******************************configure pin PB2, PB10 as input*************************
	//Enable the port B
	EN_CLK_PORT(LMSW_LEFT_PORT);
	EN_CLK_PORT(LMSW_RIGHT_PORT);
	
	//Set PB2, PB10 as input
	GPIO_PIN_MODE(LMSW_LEFT_PORT, LMSW_LEFT_PIN, IN);
	GPIO_PIN_MODE(LMSW_RIGHT_PORT, LMSW_RIGHT_PIN, IN);
	
	//Set PB2, PB10 as no pull-up/pull-down
	GPIO_PIN_PP_TYPE(LMSW_LEFT_PORT, LMSW_LEFT_PIN, NPP);
	GPIO_PIN_PP_TYPE(LMSW_RIGHT_PORT, LMSW_RIGHT_PIN, NPP);
}

void stepperHoming(void){
	//disable capture/compare 1 interrupt
	CLR_BITS(TIM4->DIER, TIM_DIER_CC1IE);
	
	//keep turning ccw until hit left limit switch
	while(!(GPIO(LMSW_LEFT_PORT)->IDR & GPIO_IDR_2)){
		runStepperMotor(-1);
		Delay_ms(20);
	}	
	//stop the motor
	runStepperMotor(0);
	Delay_ms(10);
	
	//define current position as the minimum position
	current_position = stepper_min_angle;
	
	//turn cw until hit right limit switch
	while(!(GPIO(LMSW_RIGHT_PORT)->IDR & GPIO_IDR_10)){
		runStepperMotor(1);
		current_position++;
		Delay_ms(20);
	}
	//stop the motor
	runStepperMotor(0);
	Delay_ms(10);
	
	//define the maximum position as the current position
	stepper_max_angle = current_position;
	
	//recenter the stepper target
//	current_instructions.stepper_target = (stepper_min_angle + stepper_max_angle)/2;
	current_instructions.stepper_target = 90;
	
	//Enable capture/compare 1 interrupt
	SET_BITS(TIM4->DIER, TIM_DIER_CC1IE);
	
}



void runStepperMotor(int input){
	
	static uint32_t currentStepperIndex;
	
	if(currentStepperIndex > 7) currentStepperIndex = 0;								//set currentStpperIndex to 0 in case of unintiated value
	if(input == -2 || input == -1 || input == 1 || input == 2) {				//check if the input if one of the four require values
		
		currentStepperIndex = ((currentStepperIndex + input)&7UL);				//choose the next state. The 7UL mask is to ensure the index will be inside the range
	
		//write the value of the current stepper value into target regiester
		
		FORCE_BITS(GPIO(STM_PORT)->ODR, (GPIO_ODR_(STM_A)|GPIO_ODR_(STM_ANOT)|GPIO_ODR_(STM_B)|GPIO_ODR_(STM_BNOT)), stepperValue[currentStepperIndex]<<STM_A);
		
	}
	if(input == 0)
		FORCE_BITS(GPIO(STM_PORT)->ODR, (GPIO_ODR_(STM_A)|GPIO_ODR_(STM_ANOT)|GPIO_ODR_(STM_B)|GPIO_ODR_(STM_BNOT)), 0<<STM_A);
	
	
}//End of runStepperMotor()


void TIM4_IRQHandler(void){
	
	if(((TIM4->SR & TIM_SR_CC1IF) != 0)){											//Check if the CC1 interrupt flag is set
		
		//limit the input angle
		if(current_instructions.stepper_target > 180)	
			current_instructions.stepper_target = 180;
	
		
		//convert input angle to actual steps 
		uint32_t realTarget = mapValue(0, 180, stepper_min_angle, stepper_max_angle, current_instructions.stepper_target);
		
		//Ensure target won't go beyond the allowed range
		if(realTarget > stepper_max_angle)	
			realTarget = stepper_max_angle;
		if(realTarget < stepper_min_angle)
			realTarget = stepper_min_angle;
		
		
		//in case of the target cannot be reached with +2 or -2, use half-step motion to reach target
		if((current_position+1) == realTarget){
			runStepperMotor(1);																//Move a half step in position direction
			current_position++;																//increment the position tracker
		}
		if((current_position-1) == realTarget){
			runStepperMotor(-1);															//Move a half step in negative direction
			current_position--;																//decrement the position tracker
		}
		
		//start to determine the correct action
		if(current_position < realTarget){									//target is larger than current position
			
			if(current_instructions.stepper_speed == 2){																//Moving speed is 2(full step)
				if((current_position+2) <= stepper_max_angle){			//Check if the movement will exceed max angle
					runStepperMotor(2);																//Move a full step in position direction
					current_position += 2;														//increment the position tracker
				}
				else if((current_position+1) == stepper_max_angle){	//if the current position is 1 from max angle but require speed is 2, override the speed with 1. 
					runStepperMotor(1);																//Move a half step in position direction
					current_position++;																//increment the position tracker
				}
			}
			else{																									//Moving speed is 1(half step)
				if((current_position+1) <= stepper_max_angle){			//Check if the movement will exceed max angle
					runStepperMotor(1);																//Move a half step in position direction
					current_position++;																//increment the position tracker
				}
			}
		}
		else if(current_position > realTarget){							//target is smaller than current position
			if(current_instructions.stepper_speed == 2){																//Moving speed is 2(full step)
				if((current_position-2) >= stepper_min_angle){			//Check if the movement will exceed min angle
					runStepperMotor(-2);															//Move a full step in negative direction
					current_position -= 2;														//decrement the position tracker
				}
				else if((current_position-1) == stepper_min_angle){	//if the current position is 1 from min angle but require speed is 2, override the speed with 1. 
					runStepperMotor(-1);															//Move a half step in position direction
					current_position--;																//decrement the position tracker
				}
			}
			else{																									//Moving speed is 1(half step)
				if((current_position-1) >= stepper_min_angle){			//Check if the movement will exceed min angle
					runStepperMotor(-1);															//Move a half step in negative direction
					current_position--;																//decrement the position tracker
				}
			}
		}
		else																										//current position equals to the target
			runStepperMotor(0);																		//stop the stepper motor.
		CLR_BITS(TIM4->SR, TIM_SR_CC1IF);												//clear the flag
		
	}
}

