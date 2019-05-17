#include <stdlib.h>
#include "stm32f303xe.h"
#include "SysClock.h"
#include "LED_Flash.h"
#include "SysTick.h"
#include "LCD.h"
#include "StepperMotor.h"
#include "Servo.h"
#include "DCMotor.h"
#include "UDS.h"
#include "Encoder.h"
#include "UART.h"
#include "utilities.h"
#include "Beeper.h"

#define EnableInterrupts __asm__("ISB ; CPSIE I") 
#define DisableInterrupts __asm__("CPSID I") 
#define TRUE 1
#define FALSE 0

#define PULSE_WIDTH_CM 0.0236111		//(Free runner counter prescaler+1)/system clock frequency*speed of sound/2 * (meter/centimeter)
																		//100/72000000*340/2*100 =0.0236111
																		//if the clock is 68MHz, the multiplier here should be 0.025
#define UDS_MAX_DIS 24480						//the maximum return pulse width is 34ms, so UDS_MAX_DIS = 0.034s*72000000/100 = 24480
																		//if the clock is 68MHz, the multiplier here should be 23120
extern volatile int32_t UDS_pulse_width;			//location to check the ultrasonic distance sensor pulse width value

#define LEFT_SPD_CMPS 197.352													//multiplier to calculate the left encoder speed
extern volatile uint32_t left_period_width;			//location to read the left encoder period width

#define RIGHT_SPD_CMPS 197.352												//multiplier to calculate the right encoder speed
extern volatile uint32_t right_period_width;		//location to read the right encoder period width

Instruction current_instructions;								//declare global instruction set for all actions


int main(void){
	
	System_Clock_Init();																	//Initiate system clock
	SysTick_Initialize (719);															// interrup interval is 10us. 10us * 72MHz - 1 = 719. if clock is 68MHz, this number should be 679.
//  flashLEDInit();																				//Initiate the flashing LED at PA5	
	LCDinit();																						//Initate LCD display at PB12, PB13, PB4, PB5, PB6, PB7
	LCDclear();
//	uint32_t counter = 0;
	StepperMotorInit();																		//Initate Stepper Motor
	ServoInit();																					//Initate RC Servo
	DCMotorInit();																				//Initiate DC Motor
	UDSInit();																						//Initiate ultrasonic distance sensor
	EncoderInit();																					//Initiate Encoders
	UARTInit();																						//Initiate the RS232 communication
	limitSwitchInit();																		//Inititate the limit switches
	beeperInit();																					//Initiate the beeper
	char input_buffer[MAX_UART_BUFSIZ+1];									//buffer for RS232 communication
	servoPosition(100);																		//default servo position
	current_instructions.stepper_target = 90;							//default stepper motor target
	current_instructions.stepper_speed = 2;								//default stepper motor speed
	current_instructions.DCM_Left_DIR = 1;								//default left DC motor direction (1 is forward, -1 is backward)
	current_instructions.DCM_Left_SPD = 0;               //default left DC motor speed (duty cycle)
	current_instructions.DCM_Right_DIR = 1;								//default right DC motor direction
	current_instructions.DCM_Right_SPD = 0;							//default right DC motor speed
	
	

	uint8_t camera_flag = FALSE;
	uint8_t motor_flag = FALSE;
	uint8_t left_motor_dir_flag = FALSE;
	uint8_t right_motor_dir_flag = FALSE;
	uint8_t servo_flag = FALSE;
	uint8_t stepper_flag = FALSE;
	uint8_t control_stepper_flag = FALSE;
	
	
	int8_t left_motor_dir_temp;
	uint32_t left_motor_spd_temp;
	int8_t right_motor_dir_temp;
	uint32_t right_motor_spd_temp;
	
	while(1){
//************************************main menu**********************************
		

			
			UARTprintf("*****Main Menu*****\n");
			UARTprintf("1 - Camera module\n");
			UARTprintf("2 - DC motor module\n");
			UARTprintf("Choose one of above options: ");
			while(UARTCheckEnter() != 1);  //wait for user input
			UARTString(input_buffer);			
			switch(atoi(input_buffer)){
				case 1:
					while(camera_flag != TRUE){
						UARTprintf("\n*****Camera module*****\n");
						UARTprintf("1 - Servo\n");
						UARTprintf("2 - Stepper Motor\n");
						UARTprintf("3 - Back to previous menu\n");
						UARTprintf("Choose one of above options: ");
						while(UARTCheckEnter() != 1);  //wait for user input
						UARTString(input_buffer);
						switch(atoi(input_buffer)){
							case 1:
								while(servo_flag != TRUE){
									UARTprintf("\n*****Servo*****\n");
									UARTprintf("Enter Servo angle:(75 to 180)\n");
									while(UARTCheckEnter() != 1);  //wait for user input
									UARTString(input_buffer);
									servoPosition(atoi(input_buffer));
									UARTprintf("Back to previous menu?[Y/N]\n");
									while(UARTCheckEnter() != 1);  //wait for user input
									UARTString(input_buffer);
									if(input_buffer[0] == 'Y' || input_buffer[0] == 'y')
										servo_flag = TRUE; 
								}
								servo_flag = FALSE;
								break;		
							case 2:
								while(stepper_flag != TRUE){
									UARTprintf("\n*****Stepper Motor*****\n");
									UARTprintf("1 - Control stepper motor\n");
									UARTprintf("2 - Recenter stepper motor\n");
									UARTprintf("3 - Back to previous menu\n");
									UARTprintf("Choose one of above options: ");
									while(UARTCheckEnter() != 1);  //wait for user input
									UARTString(input_buffer);
									switch(atoi(input_buffer)){
										case 1:
											while(control_stepper_flag != TRUE){
												UARTprintf("\n*****Control stepper motor****\n");
												UARTprintf("Enter stepper speed:(1 or 2) \n");
												while(UARTCheckEnter() != 1);  //wait for user input
												UARTString(input_buffer);
												current_instructions.stepper_speed = atoi(input_buffer);
												UARTprintf("Enter stepper angle:(0 to 180)\n");
												while(UARTCheckEnter() != 1);  //wait for user input
												UARTString(input_buffer);
												current_instructions.stepper_target = atoi(input_buffer);
												UARTprintf("Back to previous menu?[Y/N]\n");
												while(UARTCheckEnter() != 1);  //wait for user input
												UARTString(input_buffer);
												if(input_buffer[0] == 'Y' || input_buffer[0] == 'y')
													control_stepper_flag = TRUE; 
											}
											control_stepper_flag = FALSE; 
											break;
										case 2:
											UARTprintf("Start stepper motor recenter sequence.\n");
											stepperHoming();
											UARTprintf("Recentering finish.\n");
											break;
										case 3:
											stepper_flag = TRUE;
											break;
										default:
											UARTprintf("Invalid command, please try again.\n");
											break;
									}
								}
								stepper_flag = FALSE; 
								break;
							case 3:
								camera_flag = TRUE;
								break;
							default:
								UARTprintf("Invalid command, please try again.\n");
								break;
							
						}
					}
					camera_flag = FALSE;
					break;
				case 2:
					while(motor_flag != TRUE){
					UARTprintf("\n*****DC Motor*****\n");
					//get left motor direction command
						while(left_motor_dir_flag != TRUE){
						UARTprintf("Left motor direction:(F or B) \n");
						while(UARTCheckEnter() != 1);  //wait for user input
						UARTString(input_buffer);
						if(input_buffer[0] == 'F' || input_buffer[0] == 'f'){
							left_motor_dir_temp = 1;
							left_motor_dir_flag = TRUE;
						}
						else if(input_buffer[0] == 'B' || input_buffer[0] == 'b'){
							left_motor_dir_temp = -1;
							left_motor_dir_flag = TRUE;
						}
						else
							UARTprintf("Invalid command, please try again.\n");
						}
						left_motor_dir_flag = FALSE;
						
					//get right motor direction command
						while(right_motor_dir_flag != TRUE){
						UARTprintf("Right motor direction:(F or B) \n");
						while(UARTCheckEnter() != 1);  //wait for user input
						UARTString(input_buffer);
						if(input_buffer[0] == 'F' || input_buffer[0] == 'f'){
							right_motor_dir_temp = 1;
							right_motor_dir_flag = TRUE;
						}
						else if(input_buffer[0] == 'B' || input_buffer[0] == 'b'){
							right_motor_dir_temp = -1;
							right_motor_dir_flag = TRUE;
						}
						else
							UARTprintf("Invalid command, please try again.\n");
						}
						right_motor_dir_flag = FALSE;
						
					//get left motor speed command
					UARTprintf("Left motor speed:(0 to 100) \n");
					while(UARTCheckEnter() != 1);  //wait for user input
					UARTString(input_buffer);
					left_motor_spd_temp = atoi(input_buffer);	
						
					//get right motor speed command
					UARTprintf("Right motor speed:(0 to 100) \n");
					while(UARTCheckEnter() != 1);  //wait for user input
					UARTString(input_buffer);
					right_motor_spd_temp = atoi(input_buffer);	
					
					//write the motor commands
					DisableInterrupts;
					current_instructions.DCM_Left_DIR = left_motor_dir_temp;
					current_instructions.DCM_Left_SPD = left_motor_spd_temp;
					current_instructions.DCM_Right_DIR = right_motor_dir_temp;
					current_instructions.DCM_Right_SPD = right_motor_spd_temp;
					EnableInterrupts;
					
					//ask user whether want to back to previous menu or stay
					UARTprintf("Back to previous menu?[Y/N]\n");
					while(UARTCheckEnter() != 1);  //wait for user input
					UARTString(input_buffer);
					if(input_buffer[0] == 'Y' || input_buffer[0] == 'y')
						motor_flag = TRUE; 
					}
					motor_flag = FALSE; //reset flag
					break;
				default:
					UARTprintf("Invalid command, please try again.\n");
					break;
					
			}
		
			UARTprintf("\n");
		
		
//*************************************control DC motor with RS232***************

//		//runDCMotor(1, 100, 1, 100);
//		UARTprintf("Left speed target:(0 to 100) ");
//		while(UARTCheckEnter() != 1);  //wait for user input
//		UARTString(input_buffer);			 //convert the input to a string and store in the buffer
//		


//		
//		
//		DisableInterrupts;
//		current_instructions.DCM_Left_SPD = atoi(input_buffer);
//		EnableInterrupts;
//		
//		while(UARTCheckEnter()!=1){
//			UARTprintf("%d\n", left_period_width);
//			Delay_10us(1);
//		}
//		
//		UARTprintf("Right speed target:(0 to 100) ");
////		while(UARTCheckEnter() != 1);  //wait for user input
////		UARTString(input_buffer);			 //convert the input to a string and store in the buffer
////		
//		DisableInterrupts;
////		current_instructions.DCM_Right_SPD = atoi(input_buffer);
//		current_right_speed = mapValue(0, 24480, 0, 100, pulse_width);
//		EnableInterrupts;
//		LCDprintf("\nRight speed: %d", current_right_speed);
//		Delay_ms(10);
//		
//		LCDclear();
//				
//	
//**************************************test homing function*********************
//		
//		//DisableInterrupts;
//		UARTprintf("Start homing function?[Y/N]");
//		//EnableInterrupts;
//		
//		while(UARTCheckEnter() != 1);  //wait for user input
//		UARTString(input_buffer);			 //convert the input to a string and store in the input buffer
//		
//		//if the input is yes
//		if(input_buffer[0] == 'Y' || input_buffer[0] == 'y'){
//			LCDprintf("Start homing..");
//			stepperHoming();							//start the homing sequence
//		}
//		LCDclear();
//		LCDprintf("homing finish.");
//		
//**************************************control servo with RS232*****************		
//		if(UARTCheckEnter() == 1){
//				UARTString(input_buffer);
//				LCDprintf("%s", input_buffer);
//		}
//			
//		UARTprintf("Stepper target: ");
//		while(UARTCheckEnter() != 1);  //wait for user input
//		UARTString(input_buffer);
//		current_instructions.stepper_target = mapValue(0, 180, stepper_min_angle, stepper_max_angle, atoi(input_buffer));
//		
//		LCDclear();
//		
//**************************************control servo with RS232*****************		
//		if(UARTCheckEnter() == 1){
//				UARTString(input_buffer);
//				LCDprintf("%s", input_buffer);
//		}
//			
//		UARTprintf("Servo target: ");
//		while(UARTCheckEnter() != 1);  //wait for user input
//		UARTString(input_buffer);
//		servoPosition(atoi(input_buffer));
//		
//	//	LCDclear();
//	
/***********************************Encoders************************************/		
//		LCDprintf("L_SPD:%.4fm/s\nR_SPD:%.4fm/s", LEFT_SPD_CMPS/left_period_width, RIGHT_SPD_CMPS/right_period_width);
//		
//		Delay_ms(100);
//		
//		LCDclear();
//		
/***********************************Ultrasonic Distance Sensor******************/		
//	if(pulse_width >= UDS_MAX_DIS)			//if pulse width is larger than the maximum value, print no echo
//		LCDprintf("No Echo");
//	else
//		LCDprintf("Distance:\n%.2f cm", pulse_width*PULSE_WIDTH_CM);
//		
//	Delay_ms(100);
//	
//	LCDclear();  

/***********************************DC Motor************************************/		

//		runDCMotor(1, 100, 1, 100);
//		Delay_ms(100);
/***********************************RC Servo************************************/		
//		servoPosition(0);
//		Delay_ms(1000);
//		servoPosition(180);
//		Delay_ms(1000);
/***********************************Stepper Motor*******************************/		
//		counter &= 0xFFFF;
//	if(counter <= 300){
//		runStepperMotor(1);
//		counter++;
//	}
//	else
//		runStepperMotor(0);
//	
//	Delay_ms(50);
/***********************************LCD Display********************************/	
//		counter &= 0xFFFF;
//		LCDprintf("Weipeng Tang\nCount: 0x%.4X", counter);
//		Delay_ms(50);
//		LCDclear();
//		counter++;
/***********************************flash LED**********************************/
//		flashLED();
		
		
		
	}//End of dead while loop
	
}//End of main
