#include "Beeper.h"
#include "utilities.h"
#include "SysTick.h"
#define PRESCALER_NUM 89
#define ARR_NUM 799
#define BEEP_LENGTH 50

volatile uint32_t beep_cycle = 5000;


void beeperInit(void){
	
	//***********************Configure Pin PA5 and timer****************************
	//Enable port clock
	EN_CLK_PORT(BEEPER_PORT);
	
	//Enable Timer 8
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
	
	//Set PA5 as output
	GPIO_PIN_MODE(BEEPER_PORT, BEEPER_PIN, OUT);
	
	//Set PA5 as pull-up/pull-down
	GPIO_PIN_DRV_TYPE(BEEPER_PORT, BEEPER_PIN, PP);
	
	//Set PA5 pull-up/pull-down register as no pull-up/pull-down
	GPIO_PIN_PP_TYPE(BEEPER_PORT, BEEPER_PIN, NPP);
	
	//***********************Configure TIM8 to toggle PA5****************************
	//Configure TIM1, channel 1 and 2
	
	TIM8->CR1 &= ~TIM_CR1_DIR;																										//1. Counting direction: 0 = up-counting, 1 = down-counting
	
	TIM8->PSC = PRESCALER_NUM; 															  										//2. Clock prescaler (16 bits, up to 65,535). Counter frequency = Timer frequency/(1+PSC)
																																								//	 72MHz/(89+1) = 800kHz
	
	TIM8->ARR = ARR_NUM-1;  																											//3. Auto-reload value. PWM frequency = Counter frequency/(1+ARR)
																																								//   800kHz/(799+1) = 1000Hz
	
	CLR_BITS(TIM8->CCMR1, TIM_CCMR1_CC1S);																				//4. Select Channel 1 as output
	
	FORCE_BITS(TIM4->CCMR1, TIM_CCMR1_OC1M, TIM_CCMR1_OC1M_0);										//5. Select active mode. When TIM8_CNT = TIM8_CCR1, CC1IF is set high.
	
	TIM8->CCMR1 |= TIM_CCMR1_OC1PE;																								//6. Output 1 preload enable
	
	
	TIM8->BDTR |= TIM_BDTR_MOE;																										//7. Main output enable(MOE): 0 = Disable, 1 = Enable
	
	TIM8->CCER |= TIM_CCER_CC1E;	 																								//8. Enable output for channel 1 output
	
	TIM8->CCER &= ~TIM_CCER_CC1P; 																								//9. Select output polarity: 0 = active high, 1 = active low		
	
	TIM8->CCR1 = 0.5*ARR_NUM;		  	 																							//10. Output Compare Register for channel 1 & 2
																																								//   Initial duty cycle 50%	
	SET_BITS(TIM8->DIER, (TIM_DIER_CC1IE|TIM_DIER_UIE));                          //11. Enable capture and overflow interrupt

	TIM8->CR1 |= TIM_CR1_CEN;			 																								//12. Enable counter;
	
	//*****************************Enable Interrupt*********************************
	//set interrrupt priority
	NVIC_SetPriority(TIM8_CC_IRQn, 2);
	
	//Enable interrupt
	NVIC_EnableIRQ(TIM8_CC_IRQn);
	
}

void TIM8_CC_IRQHandler(void){
	
	static uint8_t signal_polarity;
	static uint8_t signal_enable;
	static uint32_t overflow_count;
	static uint8_t beep_cycle_flag;
	static uint32_t beep_cycle_temp;
	if((TIM8->SR & TIM_SR_UIF) != 0){   //Check if overflow flag is set
		overflow_count++;
		CLR_BITS(TIM8->SR, TIM_SR_UIF);												//clear the flag
	}
	if(beep_cycle_flag == 0){          //capture current beep_cycle if the flag is clear
		beep_cycle_temp = beep_cycle;
		beep_cycle_flag = 1;
	}
	
	if(overflow_count == beep_cycle_temp){
		signal_enable = 1;
	}
		
	if(overflow_count == (beep_cycle_temp + BEEP_LENGTH)){
		signal_enable = 0;
		overflow_count = 0;
		beep_cycle_flag = 0;
	}
		
	if((TIM8->SR & TIM_SR_CC1IF) != 0){ //Check if the compare 1 interrupt flag is set
		
		if(signal_enable == 1){																						//if signal enable is set, allow beeper to beep
			if(signal_polarity == 1)
				SET_BITS(GPIO(BEEPER_PORT)->ODR, GPIO_ODR_(BEEPER_PIN));
			else
				CLR_BITS(GPIO(BEEPER_PORT)->ODR, GPIO_ODR_(BEEPER_PIN));
				
			signal_polarity = 1 - signal_polarity;	//toggle signal polarity
		}else																															//stop the beeper			
			CLR_BITS(GPIO(BEEPER_PORT)->ODR, GPIO_ODR_(BEEPER_PIN));
		
		CLR_BITS(TIM8->SR, TIM_SR_CC1IF);												//clear the flag
	}
	
}
