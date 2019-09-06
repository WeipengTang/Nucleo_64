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
	
	//Hardware initiation
	System_Clock_Init();																	//Initiate system clock
	SysTick_Initialize (719);															// interrup interval is 10us. 10us * 72MHz - 1 = 719. if clock is 68MHz, this number should be 679.
	//flashLEDInit();																				//Initiate the flashing LED at PA5	
	LCDinit();																						//Initate LCD display at PB12, PB13, PB4, PB5, PB6, PB7
	LCDclear();
	LCDprintf("Initializing...");
	
	StepperMotorInit();																		//Initate Stepper Motor
	ServoInit();																					//Initate RC Servo
	DCMotorInit();																				//Initiate DC Motor
	UDSInit();																						//Initiate ultrasonic distance sensor
	EncoderInit();																					//Initiate Encoders
	UARTInit();																						//Initiate the RS232 communication
	limitSwitchInit();																		//Inititate the limit switches
	beeperInit();																					//Initiate the beeper
	
	//Constants Initiation
	char input_buffer[MAX_UART_BUFSIZ+1];									//buffer for RS232 communication
	servoPosition(60);																		//default servo position
	current_instructions.stepper_target = 90;							//default stepper motor target
	current_instructions.stepper_speed = 2;								//default stepper motor speed
	current_instructions.DCM_Left_DIR = 1;								//default left DC motor direction (1 is forward, -1 is backward)
	current_instructions.DCM_Left_SPD = 0;               //default left DC motor speed (duty cycle)
	current_instructions.DCM_Right_DIR = 1;								//default right DC motor direction
	current_instructions.DCM_Right_SPD = 0;							//default right DC motor speed
	
	uint8_t camera_flag = FALSE;
	uint8_t motor_flag = FALSE;
	uint8_t servo_flag = FALSE;
	uint8_t stepper_flag = FALSE;
	uint8_t control_stepper_flag = FALSE;
	
	LCDclear();
	LCDprintf("Hello World!");
	Delay_ms(2000);

//************************************main while loop**********************************		
	while(1){
			//print out main menu
			main_menu(input_buffer);
			switch(atoi(input_buffer)){
				case 1: //Camera module
					while(camera_flag != TRUE){
						camera_menu(input_buffer);
						switch(atoi(input_buffer)){
							case 1: //Servo
								while(servo_flag != TRUE){
									servo_control_menu(input_buffer);
									if(input_boolean_judge(input_buffer[0], 'y'))
										servo_flag = TRUE;
								}
								servo_flag = FALSE;
								break;		
							case 2: //Stepper motor
								while(stepper_flag != TRUE){
									stepper_motor_menu(input_buffer);
									switch(atoi(input_buffer)){
										case 1://Set stepper motor to specific angle
											while(control_stepper_flag != TRUE){
												stepper_motor_control_menu(input_buffer, &current_instructions);
												if(input_boolean_judge(input_buffer[0], 'y'))
													control_stepper_flag = TRUE; 
											}
											control_stepper_flag = FALSE; 
											break;
										case 2://Recenter stepper motor
											stepper_recenter_menu();
											break;
										case 3://Go back
											stepper_flag = TRUE;
											break;
										default://Invalid command
											UARTprintf("Invalid command, please try again.\n");
											break;
									}
								}
								stepper_flag = FALSE; 
								break;
							case 3: //Go back
								camera_flag = TRUE;
								break;
							default://Invalid command
								UARTprintf("Invalid command, please try again.\n");
								break;
							
						}
					}
					camera_flag = FALSE;
					break;
				case 2: //DC motor module
					while(motor_flag != TRUE){
					dc_motor_control_menu(input_buffer, &current_instructions);
					if(input_boolean_judge(input_buffer[0], 'y'))
						motor_flag = TRUE; 
					}
					motor_flag = FALSE; //reset flag
					break;
				default://Invalid command
					UARTprintf("Invalid command, please try again.\n");
					break;
					
			}
		
			UARTprintf("\n");
		
	}//End of dead while loop
	
}//End of main

