/******************************************************************************
*beeper.c -- Inplementation of setting up the PWM waveform output and beeper functions
*By Weipeng Tang
*FEB 7, 2019
********************************************************************************/

#include "DCMotor.h"
#include "utilities.h"
#include "SysClock.h"
#include "Servo.h"
#include "LCD.h"
#include "SysTick.h"
#define PRESCALER_NUM 899
#define ARR_NUM 7999
#define PRESCALER_TIM3 799																	
#define ARR_TIM3 35999
#define MCU_CLK 72000000
#define MAX_VANE_FREQ 1620
#define MIN_VANE_FREQ 1030
//#define FEEDBACK_SCALE_FACTOR 16777216
//#define SENSOR_GAIN 1716
//#define STUPID_SPEED_ERROR 100
//#define MIN_DRIVE_VALUE -100
//#define MAX_DRIVE_VALUE 100
//#define P_GAIN 1
//#define I_GAIN 1
//#define GAIN_DIVISOR 2

extern Instruction current_instructions;
extern volatile uint32_t left_period_width;			//location to read the left encoder period width
extern volatile uint32_t right_period_width;		//location to read the right encoder period width

void DCMotorInit(void){
	//pin assignment
	//DCM1_PWM	--PA8	--TIM1_CH1
	//DCM1_DIRA	--PA0
	//DCM1_DIRB	--PA1
	//DCM2_PWM	--PA9	--TIM2_CH2
	//DCM2_DIRA	--PB0
	//DCM2_DIRB	--PB1
	
	//Enable Peripheral Clocks via RCC Registers

	EN_CLK_PORT(MOTOR_PWM_LEFT_PORT);							//1. Enable motor port clocks
	EN_CLK_PORT(MOTOR_DIRA_LEFT_PORT);
	EN_CLK_PORT(MOTOR_DIRB_LEFT_PORT);
	EN_CLK_PORT(MOTOR_PWM_RIGHT_PORT);
	EN_CLK_PORT(MOTOR_DIRA_RIGHT_PORT);
	EN_CLK_PORT(MOTOR_DIRB_RIGHT_PORT);
	
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;					//2. enable timer 1 clock
	
	//Configure GPIO 
	
	GPIO_PIN_MODE(MOTOR_PWM_LEFT_PORT, MOTOR_PWM_LEFT_PIN, AF);									//1. set beeper pin as alternate function mode
	GPIO_PIN_MODE(MOTOR_DIRA_LEFT_PORT, MOTOR_DIRA_LEFT_PIN, OUT);
	GPIO_PIN_MODE(MOTOR_DIRB_LEFT_PORT, MOTOR_DIRB_LEFT_PIN, OUT);
	GPIO_PIN_MODE(MOTOR_PWM_RIGHT_PORT, MOTOR_PWM_RIGHT_PIN, AF);
	GPIO_PIN_MODE(MOTOR_DIRA_RIGHT_PORT, MOTOR_DIRA_RIGHT_PIN, OUT);
	GPIO_PIN_MODE(MOTOR_DIRB_RIGHT_PORT, MOTOR_DIRB_RIGHT_PIN, OUT);
	
	FORCE_BITS(GPIO(MOTOR_PWM_LEFT_PORT)->AFR[1], GPIO_AFRH_AFRH0, 6UL);				//2. select alternate function 6 (TIM1_CH1) for PA8. ARF[0] for pin 0-7, ARF[1] for pin 8-15
	FORCE_BITS(GPIO(MOTOR_PWM_LEFT_PORT)->AFR[1], GPIO_AFRH_AFRH1, 6UL<<(1*4));//  select alternate function 6 (TIM1_CH2) for PA9.
				
	GPIO_PIN_DRV_TYPE(MOTOR_DIRA_LEFT_PORT, MOTOR_DIRA_LEFT_PIN, PP);						//3. Set all direction control pin as pull-up/pull-down
	GPIO_PIN_DRV_TYPE(MOTOR_DIRB_LEFT_PORT, MOTOR_DIRB_LEFT_PIN, PP);
	GPIO_PIN_DRV_TYPE(MOTOR_DIRA_RIGHT_PORT, MOTOR_DIRA_RIGHT_PIN, PP);
	GPIO_PIN_DRV_TYPE(MOTOR_DIRB_RIGHT_PORT, MOTOR_DIRB_RIGHT_PIN, PP);
				
	GPIO_PIN_PP_TYPE(MOTOR_PWM_LEFT_PORT, MOTOR_PWM_LEFT_PIN, NPP);								//4. set no pull-up, no pull-down
	GPIO_PIN_PP_TYPE(MOTOR_DIRA_LEFT_PORT, MOTOR_DIRA_LEFT_PIN, NPP);
	GPIO_PIN_PP_TYPE(MOTOR_DIRB_LEFT_PORT, MOTOR_DIRB_LEFT_PIN, NPP);
	GPIO_PIN_PP_TYPE(MOTOR_PWM_RIGHT_PORT, MOTOR_PWM_RIGHT_PIN, NPP);
	GPIO_PIN_PP_TYPE(MOTOR_DIRA_RIGHT_PORT, MOTOR_DIRA_RIGHT_PIN, NPP);
	GPIO_PIN_PP_TYPE(MOTOR_DIRB_RIGHT_PORT, MOTOR_DIRB_RIGHT_PIN, NPP);
	
	//*********************************************Configure TIM1 for PWM output******************************************************
	//Configure TIM1, channel 1 and 2
	
	TIM1->CR1 &= ~TIM_CR1_DIR;																										//1. Counting direction: 0 = up-counting, 1 = down-counting
	
	TIM1->PSC = PRESCALER_NUM; 															  										//2. Clock prescaler (16 bits, up to 65,535). Counter frequency = Timer frequency/(1+PSC)
																																								//	 72MHz/(89+1) = 800kHz
	
	TIM1->ARR = ARR_NUM-1;  																											//3. Auto-reload value. PWM frequency = Counter frequency/(1+ARR)
																																								//   800kHz/(799+1) = 300Hz
	
	FORCE_BITS(TIM1->CCMR1, TIM_CCMR1_OC1M, (TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1M_2));	//4. Select PWM mode 1 output on channel 1 and 2(OC1M = 0110, OC2M = 0110)
	FORCE_BITS(TIM1->CCMR1, TIM_CCMR1_OC2M, (TIM_CCMR1_OC2M_1|TIM_CCMR1_OC2M_2));
	
	TIM1->CCMR1 |= TIM_CCMR1_OC1PE;																								//5. Output 1 preload enable
	TIM1->CCMR1 |= TIM_CCMR1_OC2PE;																								//   Output 2 preload enable
	
	TIM1->BDTR |= TIM_BDTR_MOE;																										//6. Main output enable(MOE): 0 = Disable, 1 = Enable
	
	TIM1->CCER |= TIM_CCER_CC1E;	 																								//7. Enable output for channel 1 output
	TIM1->CCER |= TIM_CCER_CC2E;																									//   Enable output for channel 2 output
	
	TIM1->CCER &= ~TIM_CCER_CC1P; 																								//8. Select output polarity: 0 = active high, 1 = active low
	TIM1->CCER &= ~TIM_CCER_CC2P;																									//		
	
	TIM1->CCR1 = 0.5*ARR_NUM;		  	 																							//9. Output Compare Register for channel 1 & 2
	TIM1->CCR2 = 0.5*ARR_NUM;																											//   Initial duty cycle 50%
																 
	TIM1->CR1 |= TIM_CR1_CEN;			 																								//10. Enable counter;
	
	//********************************************Configure TIM3 for DC Motor speed control******************************************/
	//***********************************Enable TIM3 for interrup service**************************
	//Enable the clock of timer 3 
	SET_BITS(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
	
	//set pre-scaler.72MHz/(899+1) = 80KHz.
	TIM3->PSC = PRESCALER_TIM3;
	
	//set counter period.80KHz/(7999+1) = 10Hz.																																					 
	TIM3->ARR = ARR_TIM3;
	
	//Select Channel 1 as output																													   
	CLR_BITS(TIM3->CCMR1, TIM_CCMR1_CC1S);  									
													
	//Select active mode. When TIM3_CNT = TIM3_CCR1, CC1IF is set high.
  FORCE_BITS(TIM3->CCMR1, TIM_CCMR1_OC1M, TIM_CCMR1_OC1M_0);																															

	//Duty cycle. May not be important here.
	TIM3->CCR1 = 500;
	
	//Main output enable
	TIM3->BDTR |= TIM_BDTR_MOE;
	
	//Select output polarity: 0 = active high, 1 = active low
	TIM3->CCER &= ~TIM_CCER_CC1P;
	
	//Enable output for channel 1 output
	TIM3->CCER |= TIM_CCER_CC1E;	 
	
	//Enable capture/compare 1 interrupt
	SET_BITS(TIM3->DIER, TIM_DIER_CC1IE);
	
	//Enable timer 4
	TIM3->CR1 |= TIM_CR1_CEN;
	/********************************Enable Interrupt****************************************/
	//Set the interrupt priority of TIM3_IRQn 
	NVIC_SetPriority(TIM3_IRQn, 2);															
	
	//Enable the interrupt TIM3_IRQn
	NVIC_EnableIRQ(TIM3_IRQn);																	
	

	
}

void runDCMotor(int8_t DIR_L, uint32_t SPD_L, int8_t DIR_R, uint32_t SPD_R){
	//stop the two motor during operation mode change
	CLR_BITS(GPIO(MOTOR_DIRA_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_LEFT_PIN));
	CLR_BITS(GPIO(MOTOR_DIRB_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_LEFT_PIN));
	CLR_BITS(GPIO(MOTOR_DIRA_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_RIGHT_PIN));
	CLR_BITS(GPIO(MOTOR_DIRB_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_RIGHT_PIN));
	
	//Determine the PWM signal to the two motors
	TIM1->CCR1 = SPD_L*ARR_NUM/100;
	TIM1->CCR2 = SPD_R*ARR_NUM/100;
	
	//Determine the direction of the two motors
	if(DIR_L > 0){																																//Left Motor Forward
		SET_BITS(GPIO(MOTOR_DIRA_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_LEFT_PIN));
		CLR_BITS(GPIO(MOTOR_DIRB_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_LEFT_PIN));
	}
	else if(DIR_L < 0){																														//Left Motor Backward
		CLR_BITS(GPIO(MOTOR_DIRA_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_LEFT_PIN));
		SET_BITS(GPIO(MOTOR_DIRB_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_LEFT_PIN));
	}
	else{																																					//Left Motor Stops
		CLR_BITS(GPIO(MOTOR_DIRA_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_LEFT_PIN));
		CLR_BITS(GPIO(MOTOR_DIRB_LEFT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_LEFT_PIN));
		
	}
	
	if(DIR_R > 0){																																//Right Motor Forward
		SET_BITS(GPIO(MOTOR_DIRA_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_RIGHT_PIN));
		CLR_BITS(GPIO(MOTOR_DIRB_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_RIGHT_PIN));
	}
	else if(DIR_R < 0){																														//Right Motor Backward
		CLR_BITS(GPIO(MOTOR_DIRA_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_RIGHT_PIN));
		SET_BITS(GPIO(MOTOR_DIRB_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_RIGHT_PIN));
	}
	else{																																					//Right Motor Stops
		CLR_BITS(GPIO(MOTOR_DIRA_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRA_RIGHT_PIN));
		CLR_BITS(GPIO(MOTOR_DIRB_RIGHT_PORT)->ODR, GPIO_ODR_(MOTOR_DIRB_RIGHT_PIN));
	}
	
}


void TIM3_IRQHandler(void){
	
	if(((TIM3->SR & TIM_SR_CC1IF) != 0)){											//Check if the CC1 interrupt flag is set
	static uint32_t left_PWM;					//range from 0 to 100
	static uint32_t right_PWM;


	left_PWM = speedControl(current_instructions.DCM_Left_SPD, left_PWM, left_period_width);
	right_PWM = speedControl(current_instructions.DCM_Right_SPD, right_PWM, right_period_width);

	runDCMotor(current_instructions.DCM_Left_DIR, left_PWM, current_instructions.DCM_Right_DIR, right_PWM);
	
		
		
	CLR_BITS(TIM3->SR, TIM_SR_CC1IF);												//clear the flag	
	}
}
uint32_t speedControl(uint32_t target_speed, uint32_t PWM, int32_t current_period_ticks){
	
	if(target_speed > 0){
		int32_t vane_frequency = 720000/current_period_ticks;
		
		if(vane_frequency <= MIN_VANE_FREQ)
			vane_frequency = MIN_VANE_FREQ;
		if(vane_frequency >= MAX_VANE_FREQ)
			vane_frequency = MAX_VANE_FREQ;
		
		int32_t current_speed = mapValue(MIN_VANE_FREQ, MAX_VANE_FREQ, 0, 100, vane_frequency);
		
		int32_t current_error = target_speed - current_speed;

		if(current_error > 100)
			current_error = 100;
		if(current_error < -100)
			current_error = -100;
		
		if((PWM+current_error)<=100)
			PWM += current_error;
		else if((PWM+current_error)>100)
			PWM = 100;
		else
			PWM = 0;
	}else
	
	PWM = 0;
	
	return PWM;
	
}




//void TIM3_IRQHandler(void){
//	
//	static int32_t leftSpeedErrorIntegral;
//	static int32_t rightSpeedErrorIntegral;
//	static uint32_t left_PWM;
//	static uint32_t right_PWM;
//	
//	if(((TIM3->SR & TIM_SR_CC1IF) != 0)){											//Check if the CC1 interrupt flag is set
//	
//	//left_PWM = controlLawFunction(leftSpeedErrorIntegral, left_PWM, current_instructions.DCM_Left_SPD, left_period_width);
//	//right_PWM = controlLawFunction(rightSpeedErrorIntegral, right_PWM, current_instructions.DCM_Right_SPD, right_period_width);
//	
//	runDCMotor(current_instructions.DCM_Left_DIR, left_PWM, current_instructions.DCM_Right_DIR, right_PWM);
//	CLR_BITS(TIM3->SR, TIM_SR_CC1IF);												//clear the flag	
//	}
//}
//uint32_t controlLawFunction(int32_t speedErrorIntegral, uint32_t PWM, uint32_t setSpeed, uint32_t encoderVaneWidth){
//	 
//	
//	
//	
//	 //calculate error terms
//	 int32_t speedError = setSpeed*SENSOR_GAIN - FEEDBACK_SCALE_FACTOR/encoderVaneWidth*720000;
//	 int32_t driveValue;
//	 
//	 	//Check for stupid speed error
//	if((speedError < STUPID_SPEED_ERROR) && (speedError > -STUPID_SPEED_ERROR)){
//		
//		//update integral term but only if drive is not on the rail
//		if(((PWM == MIN_DRIVE_VALUE) && (speedError > 0)) || ((PWM == MAX_DRIVE_VALUE) && (speedError < 0)) || ((PWM > MIN_DRIVE_VALUE) && (PWM < MAX_DRIVE_VALUE)))
//			speedErrorIntegral += speedError;
//		
//		//Calculate control law
//		driveValue = ((speedError * P_GAIN) + (speedErrorIntegral * I_GAIN))/GAIN_DIVISOR;
//		
//		//Limit the controller output to range of PWM
//		if(driveValue > MAX_DRIVE_VALUE)
//			driveValue = MAX_DRIVE_VALUE;
//		else if(driveValue < MIN_DRIVE_VALUE)
//			driveValue = MIN_DRIVE_VALUE;
//		
//		return (PWM + driveValue);
//	}
//	else //if stupid error happens, keep the current PWM
//		return PWM;
//	 
// }

