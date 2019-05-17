#include "Encoder.h"
#include "utilities.h"
#include "stm32f303xe.h"
#include "SysTick.h"
#include "LCD.h"
#define PRESCALER_NUM 99
#define ARR_NUM 199
#define LEFT_SPD_CMPS 197.352													//multiplier to calculate the left encoder speed
#define RIGHT_SPD_CMPS 197.352												//multiplier to calculate the right encoder speed
volatile uint32_t left_period_width = 1000;
static volatile uint32_t left_last_captured = 0;

volatile uint32_t right_period_width = 1000;
static volatile uint32_t right_last_captured = 0;


void EncoderInit(void){
/***************************************Configure PA6 & PA7 as alternate function***********************************/
//Configure PA6	
	EN_CLK_PORT(ENC_LEFT_PORT);																			//1. Enable encoder left port
	
	GPIO_PIN_MODE(ENC_LEFT_PORT, ENC_LEFT_PIN, AF);									//2. Set pin PA6 as alternative function 1 (TIM16_CH1).
	
	FORCE_BITS(GPIO(ENC_LEFT_PORT)->AFR[0], GPIO_AFRL_AFRL6, (1UL<<(4*6)));

//Configure PA7	
	EN_CLK_PORT(ENC_RIGHT_PORT);																			//1. Enable encoder right port
	
	GPIO_PIN_MODE(ENC_RIGHT_PORT, ENC_RIGHT_PIN, AF);									//2. Set pin PA7 as alternative function 1 (TIM17_CH1).
	
	FORCE_BITS(GPIO(ENC_RIGHT_PORT)->AFR[0], GPIO_AFRL_AFRL7, (1UL<<(4*7)));	
	
/***************************************Configure TIM16_CH1*********************************************************/
	SET_BITS(RCC->APB2ENR, RCC_APB2ENR_TIM16EN);								//1. Enable the clock of timer 16 
	
	TIM16->PSC = PRESCALER_NUM;															//2. set pre-scaler. 
																															//   72MHz/(99+1) = 720KHz
	
	TIM16->ARR = ARR_NUM;																		//set counter period
																																//   720KHz/(7199+1) = 100Hz (10 ms period)
	
	FORCE_BITS(TIM16->CCMR1, TIM_CCMR1_CC1S, TIM_CCMR1_CC1S_0);  //3. Select the active input
																															
	CLR_BITS(TIM16->CCMR1, TIM_CCMR1_IC1F);											//4. Program the input filter duration. Disable input fileter function
																															
	CLR_BITS(TIM16->CCER, (TIM_CCER_CC1NP|TIM_CCER_CC1P));
																															//5. Select the edge of the active transition
																															//   CC1NP = 0 and CC1P = 0 for raising edge only
	CLR_BITS(TIM16->CCMR1, TIM_CCMR1_IC1PSC);										//6. input prescaler set to 0.

  SET_BITS(TIM16->CCER, TIM_CCER_CC1E);												//7. Enable capture from the counter. 
	
	SET_BITS(TIM16->DIER, (TIM_DIER_CC1IE|TIM_DIER_TIE|TIM_DIER_UIE));											  
																															//8. Capture/compare 1 interrupt enable. 
																															//   Trigger interrupt enable.
																															//	 Update interrupt enable.																								
	
	SET_BITS(TIM16->CR1, TIM_CR1_CEN);														//9. Enable the counter. 
	
/***************************************Configure TIM17_CH1*********************************************************/
	SET_BITS(RCC->APB2ENR, RCC_APB2ENR_TIM17EN);								//1. Enable the clock of timer 17 
	
	TIM17->PSC = PRESCALER_NUM;																	//2. set pre-scaler. 
																															//   72MHz/(99+1) = 720KHz
	
	TIM17->ARR = ARR_NUM;																				//set counter period
																																//   720KHz/(199+1) = 3600Hz (0.2778 ms period)
																																//   to avoid aliasing, this frequency needs to be larger than 3200Hz. 720KHz/(224+1) = 3200Hz
	
	FORCE_BITS(TIM17->CCMR1, TIM_CCMR1_CC1S, TIM_CCMR1_CC1S_0);  //3. Select the active input
																															
	CLR_BITS(TIM17->CCMR1, TIM_CCMR1_IC1F);											//4. Program the input filter duration. Disable input fileter function
																															
	CLR_BITS(TIM17->CCER, (TIM_CCER_CC1NP|TIM_CCER_CC1P));
																															//5. Select the edge of the active transition
																															//   CC1NP = 0 and CC1P = 0 for raising edge only
	CLR_BITS(TIM17->CCMR1, TIM_CCMR1_IC1PSC);										//6. input prescaler set to 0.

  SET_BITS(TIM17->CCER, TIM_CCER_CC1E);												//7. Enable capture from the counter. 
	
	SET_BITS(TIM17->DIER, (TIM_DIER_CC1IE|TIM_DIER_TIE|TIM_DIER_UIE));											  
																															//8. Capture/compare 1 interrupt enable. 
																															//   Trigger interrupt enable.
																															//	 Update interrupt enable.																								
	
	SET_BITS(TIM17->CR1, TIM_CR1_CEN);														//9. Enable the counter. 
	
/***************************************Enable ISRs for the two timer***********************************************/
	
	NVIC_SetPriority(TIM16_IRQn, 2);														//1. Set the interrupt priority of TIM16_IRQn to the least urgency
	
	NVIC_EnableIRQ(TIM16_IRQn);																	//2. Enable the interrupt TIM16_IRQn
	
	NVIC_SetPriority(TIM17_IRQn, 2);														//3. Set the interrupt priority of TIM17_IRQn to the least urgency
	
	NVIC_EnableIRQ(TIM17_IRQn);																	//4. Enable the interrupt TIM17_IRQn
	
}

void TIM16_IRQHandler(void){
	
	uint32_t left_current_captured;
	
	static uint32_t overflow_count;
	static uint32_t LCD_overflow_count;
	
	if((TIM16->SR & TIM_SR_UIF) != 0){														//Check if overflow has taken place
		
		overflow_count++;																					//increment the echo overflow counter
		LCD_overflow_count++;
		TIM16->SR &= ~TIM_SR_UIF;																	//Clear UIF flag to prevent re-entering
	}
	
	if((TIM16->SR & TIM_SR_CC1IF) != 0){													//Check interrupt flag is set
		
		left_current_captured = TIM16->CCR1;														//Reading CCR1 clears CC1IF interrupt flag
		
		left_period_width = (left_current_captured - left_last_captured)+(ARR_NUM + 1)*overflow_count; 				//calculate pulse width. pulse width is an integer
					
		overflow_count = 0;																				//reset overflow counter													
		
		left_last_captured = left_current_captured;													//store last capture
	}	

//	if(LCD_overflow_count%1000 == 0){
//		LCDclear();
//		if(left_period_width > 900)
//			LCDprintf("Left stopped.\n");
//		else
//			LCDprintf("Left:%.4fm/s\n", LEFT_SPD_CMPS/left_period_width);
//		
//		if(right_period_width > 900)
//			LCDprintf("Right stopped.");
//		else
//			LCDprintf("Right:%.4fm/s", RIGHT_SPD_CMPS/right_period_width);
//		
//		LCD_overflow_count = 0;
//	}
}

void TIM17_IRQHandler(void){
	
	uint32_t right_current_captured;
	
	static uint32_t overflow_count;
	
	if((TIM17->SR & TIM_SR_UIF) != 0){														//Check if overflow has taken place
		
		overflow_count++;																					//increment the echo overflow counter
		
		TIM17->SR &= ~TIM_SR_UIF;																	//Clear UIF flag to prevent re-entering
	}
	
	if((TIM17->SR & TIM_SR_CC1IF) != 0){													//Check interrupt flag is set
		
		right_current_captured = TIM17->CCR1;														//Reading CCR1 clears CC1IF interrupt flag
		
		right_period_width = (right_current_captured - right_last_captured)+(ARR_NUM + 1)*overflow_count; 				//calculate pulse width. pulse width is an integer
					
		overflow_count = 0;																				//reset overflow counter													
		
		right_last_captured = right_current_captured;													//store last capture
	}		
}

