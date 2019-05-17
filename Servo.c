#include "utilities.h"
#include "SysClock.h"
#include "Servo.h"
#define PRESCALER_NUM 89
#define ARR_NUM 15999
#define MCU_CLK 72000000
#define MAX_SERVO_ANGLE 180
#define MIN_SERVO_ANGLE 75

void ServoInit(void){
	
	//Pin assignment 
	//PB11
	
	//Enable Peripheral Clocks via RCC Registers

	EN_CLK_PORT(SERVO_PORT);							//1. Enable beeper port clock
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		//2. enable timer 2 clock
	
	//Configure GPIO (Pin PB11)
	
	GPIO_PIN_MODE(SERVO_PORT, SERVO_PIN, AF);																	//1. set beeper pin as alternate function mode
	
	FORCE_BITS(GPIO(SERVO_PORT)->AFR[1], GPIO_AFRH_AFRH3, 1UL<<(4*3));				//2. select alternate function 1 (TIM2_CH4). ARF[0] for pin 0-7, ARF[1] for pin 8-15
	
	GPIO_PIN_PP_TYPE(SERVO_PORT, SERVO_PIN, NPP);															//3. set no pull-up, no pull-down
	
	//Configure GPIO (Pin PB11)
	
	TIM2->CR1 &= ~TIM_CR1_DIR;																										//1. Counting direction: 0 = up-counting, 1 = down-counting
	
	TIM2->PSC = PRESCALER_NUM; 															  										//2. Clock prescaler (16 bits, up to 65,535). Counter frequency = Timer frequency/(1+PSC)
																																								//	 72MHz/(84+1) = 800kHz
	
	TIM2->ARR = ARR_NUM;  																											  //3. Auto-reload value. PWM frequency = Counter frequency/(1+ARR)
																																								//   800kHz/(15999+1) = 50Hz
	
	FORCE_BITS(TIM2->CCMR2, TIM_CCMR2_OC4M, (TIM_CCMR2_OC4M_1|TIM_CCMR2_OC4M_2));	//4. Select PWM mode 1 output on channel 4 (OC4M = 0110) Reference manual page 658 and 661
	
	TIM2->CCMR2 |= TIM_CCMR2_OC4PE;																								//5. Output 1 preload enable
	
	TIM2->BDTR |= TIM_BDTR_MOE;																										//6. Main output enable(MOE): 0 = Disable, 1 = Enable
	
	TIM2->CCER |= TIM_CCER_CC4E;	 																								//7. Enable output for channel 4 output. Reference manual page 662.
	
	TIM2->CCER &= ~TIM_CCER_CC4P; 																								//8. Select output polarity: 0 = active high, 1 = active low
	
	TIM2->CCR4 = 0.075*ARR_NUM;		  	 																						//9. Output Compare Register for channel 4. Reference manual page 666.
																																								//   Initial duty cycle 7.5%
																 
	TIM2->CR1 |= TIM_CR1_CEN;			 																								//10. Enable counter;
	
	
}

void servoPosition(uint32_t angle){
			
	//input angle should be range from 0 to 180 degree
	//Servo position map: 0   degree - 0.6ms(3%   duty cycle)
	//									  90  degree - 1.5ms(7.5% duty cycle)
	//										180 degree - 2.4ms(12%  duty cycle)
	//formula: duty cycle = angle/2000 + 0.03
	
	
	//control input to acceptable limit
	if(angle > MAX_SERVO_ANGLE)
		angle = MAX_SERVO_ANGLE;
	if(angle < MIN_SERVO_ANGLE)
		angle = MIN_SERVO_ANGLE;
	
	
	uint32_t temp = ((angle/2 + 30)*ARR_NUM);
	
	TIM2->CCR4 = temp/1000;																												//Change PWM duty cycle. Set duty cycle based on the required angle
	
}
uint32_t mapValue(uint32_t minIn, uint32_t maxIn, uint32_t minOut, uint32_t maxOut, uint32_t value){
	
	//to ensure input not exceed input limits
	if(value < minIn) value = minIn;
	if(value > maxIn) value = maxIn;
	
	return ((value - minIn)*(maxOut - minOut)/(maxIn - minIn) + minOut);
	
}

