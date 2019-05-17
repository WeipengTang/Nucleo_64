#include "utilities.h"
#include "stm32f303xe.h"
#include "SysTick.h"
#include "UDS.h"
#include "Beeper.h"
#define EnableInterrupts __asm__("ISB ; CPSIE I") 
#define DisableInterrupts __asm__("CPSID I") 
#define PRESCALER_NUM_TRIG 99		//values based on 72MHz
#define ARR_NUM_TRIG 35999UL
#define PRESCALER_NUM_ECHO 99		//values based on 72MHz
#define ARR_NUM_ECHO 35999UL
#define DUTY_CYCLE_TRIG 0.0002
#define UDS_PULSE_WIDTH_CM 0.0236111		//(Free runner counter prescaler+1)/system clock frequency*speed of sound/2 * (meter/centimeter)
																		//100/72000000*340/2*100 =0.0236111

//These numbers are to balance the trigger output and echo input and to ensure the interrupt will not happen too frequently
//Since Trig and Echo share the same timer, the two prescalar and ARR have to be the same

volatile uint32_t UDS_pulse_width = 0;

extern volatile uint32_t beep_cycle;

void UDSInit(void){
	
	UDSEcho();																	//Enable input echo signal capture
	UDSTrig();																	//Enable output trigger signal
	
}

void UDSTrig(void){
/*******************************Configure Trigger waveform***************************************************************************************************************/
/*******************************the following section is covered by the UDSEcho() - start****************************/

// The trigger will be handled by the TIM15 interrupt service routine
	//Enable Peripheral Clocks via RCC Registers

//	EN_CLK_PORT(UDS_TRIG_PORT);							//1. Enable beeper port clock
	
//	SET_BITS(RCC->APB2ENR, RCC_APB2ENR_TIM15EN);		//2. enable timer 15 clock

/*******************************this above section is covered by the UDSEcho() - end******************************/	
	
//Configure GPIO (Pin PB15)
	
	GPIO_PIN_MODE(UDS_TRIG_PORT, UDS_TRIG_PIN, AF);								//1. set trigger pin as alternate function mode
	
	FORCE_BITS(GPIO(UDS_TRIG_PORT)->AFR[1], GPIO_AFRH_AFRH7, (1UL<<(4*7)));				//2. select alternate function 1 (TIM15_CH2). ARF[0] for pin 0-7, ARF[1] for pin 8-15.
	
	GPIO_PIN_PP_TYPE(UDS_TRIG_PORT, UDS_TRIG_PIN, NPP);						//3. set no pull-up, no pull-down
	
/*******************************the following section is covered by the UDSEcho() - start****************************/	
//	
//	//Configure GPIO (Pin PB15)
//	
//	TIM15->CR1 &= ~TIM_CR1_DIR;																										//1. Counting direction: 0 = up-counting, 1 = down-counting
//	
//	TIM15->PSC = PRESCALER_NUM_TRIG; 															  							//2. Clock prescaler (16 bits, up to 65,535). Counter frequency = Timer frequency/(1+PSC)
//																																								//	 72MHz/(99+1) = 720KHz
//	
//	TIM15->ARR = ARR_NUM_TRIG;  																									//3. Auto-reload value. PWM frequency = Counter frequency/(1+ARR)
//																																								//   720KHz/(35999+1) = 20Hz (50 ms period)
/*******************************the above section is covered by the UDSEcho() - end******************************/

	FORCE_BITS(TIM15->CCMR1, TIM_CCMR1_OC2M, (TIM_CCMR1_OC2M_1|TIM_CCMR1_OC2M_2));	//4. Select PWM mode 1 output on channel 2 (OC2M = 110)
	
	TIM15->CCMR1 |= TIM_CCMR1_OC2PE;																								//5. Output 2 preload enable
	
	TIM15->BDTR |= TIM_BDTR_MOE;																										//6. Main output enable(MOE): 0 = Disable, 1 = Enable
	
	TIM15->CCER |= TIM_CCER_CC2E;	 																								//7. Enable output for channel 2 output
	
	TIM15->CCER &= ~TIM_CCER_CC2P; 																								//8. Select output polarity: 0 = active high, 1 = active low
	
	TIM15->CCR2 = DUTY_CYCLE_TRIG*ARR_NUM_TRIG;		  	 														//9. Output Compare Register for channel 2

/*******************************the following section is covered by the UDSEcho() - start****************************/
//																 
//	TIM15->CR1 |= TIM_CR1_CEN;			 																								//10. Enable counter;
/*******************************the above section is covered by the UDSEcho() - end******************************/	
}

void UDSEcho(void){
	/********************************Configure Echo width measurement*****************************************************************/
	/*******************************Configure GPIO Pin*************************************/
	EN_CLK_PORT(UDS_ECHO_PORT);																			//1. Enable port for UDS port
	
	GPIO_PIN_MODE(UDS_ECHO_PORT, UDS_ECHO_PIN, AF);									//2. Set pin PB14 as alternative function 1 (TIM15_CH1).
	
	FORCE_BITS(GPIO(UDS_ECHO_PORT)->AFR[1], GPIO_AFRH_AFRH6, (1UL<<(4*6)));
	
	/********************************Configure Timer 15 Input Capture (Channel 1)***********/
	
	SET_BITS(RCC->APB2ENR, RCC_APB2ENR_TIM15EN);								//1. Enable the clock of timer 15 
	
	TIM15->PSC = PRESCALER_NUM_ECHO;															//2. set pre-scaler. 
																																//   72MHz/(99+1) = 720KHz
	
	TIM15->ARR = ARR_NUM_ECHO;																		//set counter period
																																//   720KHz/(35999+1) = 20Hz (50 ms period)
	
	FORCE_BITS(TIM15->CCMR1, TIM_CCMR1_CC1S, TIM_CCMR1_CC1S_0);  //3. Select the active input
																															//   refer to reference manual page 726
	CLR_BITS(TIM15->CCMR1, TIM_CCMR1_IC1F);											//4. Program the input filter duration. Disable input fileter function
																															//   refer to reference manual page 726
	FORCE_BITS(TIM15->CCER, (TIM_CCER_CC1NP|TIM_CCER_CC1P), (TIM_CCER_CC1NP|TIM_CCER_CC1P));
																															//5. Select the edge of the active transition
																															//   CC1NP = 1 and CC1P = 1 for both raising and falling edge detection. Page 730
	CLR_BITS(TIM15->CCMR1, TIM_CCMR1_IC1PSC);										//6. input prescaler set to 0. Page 726.

  SET_BITS(TIM15->CCER, TIM_CCER_CC1E);												//7. Enable capture from the counter. Page 731.
	
	SET_BITS(TIM15->DIER, (TIM_DIER_CC1IE|TIM_DIER_TIE|TIM_DIER_UIE));											  
																															//8. Capture/compare 1 interrupt enable. Page 723.
																															//   Trigger interrupt enable.
																															//	 Update interrupt enable.																								
	
	SET_BITS(TIM15->CR1, TIM_CR1_CEN);														//9. Enable the counter. Page 719.
	
	
	/********************************Enable Interrupt****************************************/
	
	NVIC_SetPriority(TIM15_IRQn, 2);															//1. Set the interrupt priority of TIM15_IRQn to the least urgency
	
	NVIC_EnableIRQ(TIM15_IRQn);																	//2. Enable the interrupt TIM15_IRQn
	
	
}
 uint32_t current_captured;
 volatile uint32_t last_captured = 0;
 volatile uint32_t signal_polarity = 0;
 uint32_t overflow_count_echo;
void TIM15_IRQHandler(void){
	
//	static uint32_t current_captured;
//	static volatile uint32_t last_captured = 0;
//	static volatile uint32_t signal_polarity = 0;
//	static uint32_t overflow_count_echo;
	
	
	if((TIM15->SR & TIM_SR_UIF) != 0){														//Check if overflow has taken place
		
		if(signal_polarity == 1)
			overflow_count_echo++;																					//increment the echo overflow counter
		
		TIM15->SR &= ~TIM_SR_UIF;																	//Clear UIF flag to prevent re-entering
	}

	if((TIM15->SR & TIM_SR_CC1IF) != 0){													//Check interrupt flag is set
		
		current_captured = TIM15->CCR1;														//Reading CCR1 clears CC1IF interrupt flag
		
		signal_polarity = 1 - signal_polarity;										//Toggle the polarity flag;
		
		if(signal_polarity == 0){																	//Calculate only when the current input is low
			
			if(current_captured > last_captured)
				UDS_pulse_width = (current_captured - last_captured)+(ARR_NUM_ECHO + 1)*overflow_count_echo; 				//calculate pulse width. pulse width is an integer
			else
				UDS_pulse_width = (ARR_NUM_ECHO + 1)*overflow_count_echo - (last_captured - current_captured);
		}
		
		overflow_count_echo = 0;																				//reset echo overflow counter													
		
		last_captured = current_captured;													//store last capture
		
		beeperControl();																					//update beeper control
	}

}
void beeperControl(void){
	
//	uint32_t average = walkingAverage(UDS_pulse_width);
	uint32_t average = UDS_pulse_width;
	
	DisableInterrupts;
	if((average*UDS_PULSE_WIDTH_CM) >= 100)
		beep_cycle = 5000;
	else if((average*UDS_PULSE_WIDTH_CM) >= 80)
		beep_cycle = 4000;
	else if((average*UDS_PULSE_WIDTH_CM) >= 60)
		beep_cycle = 3000;
	else if((average*UDS_PULSE_WIDTH_CM) >= 40)
		beep_cycle = 2000;
	else if((average*UDS_PULSE_WIDTH_CM) >= 20)
		beep_cycle = 1000;
	else if((average*UDS_PULSE_WIDTH_CM) >= 10)
		beep_cycle = 500;
	else if((average*UDS_PULSE_WIDTH_CM) >= 5)
		beep_cycle = 250;
	else
		beep_cycle = 100;
	EnableInterrupts;
}
uint32_t walkingAverage(uint32_t current_value){
	
	static uint32_t buffer[5];
	static uint32_t average = 24480;
	uint32_t i;
	for(i = 0; i < 5; i++){
		buffer[i+1] = buffer[i];
	}

//	if(current_value - buffer[1] <30000)
//		buffer[0] = current_value;
//	else
//		buffer[0] = buffer[0];
	
	for(i = 0; i < 5; i++){
		average+=buffer[i];
	}
	average = average/5;
	
	return average;
	
	
	
	
}















