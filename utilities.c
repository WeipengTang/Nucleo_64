#include <stdlib.h>
#include <ctype.h>
#include "stm32f303xe.h"
#include "SysClock.h"
#include "SysTick.h"
#include "utilities.h"
#include "LCD.h"
#include "UART.h"
#include "Servo.h"
#include "StepperMotor.h"
#include "DCMotor.h"
#include "Encoder.h"
#define TRUE 1
#define FALSE 0
#define EnableInterrupts __asm__("ISB ; CPSIE I") 
#define DisableInterrupts __asm__("CPSID I") 


void receive_cmd(char *buffer){
	while(UARTCheckEnter() != 1);  //wait for user input
	UARTString(buffer);
}
uint8_t input_boolean_judge(char input, char expected_letter){
	input = tolower(input);
	expected_letter = tolower(expected_letter);
	
	if(input == expected_letter)
		return TRUE;
	else
		return FALSE;
	
}
void main_menu(char *buffer){
	LCDclear();
	LCDprintf("*main menu*");
	Delay_ms(100);
	UARTprintf("*****Main Menu*****\n");
	UARTprintf("1 - Camera module\n");
	UARTprintf("2 - DC motor module\n");
	UARTprintf("Choose one of above options: ");	
	receive_cmd(buffer);
}

void camera_menu(char *buffer){
	LCDclear();
	LCDprintf("*Camera module*");
	Delay_ms(100);
	UARTprintf("\n*****Camera module*****\n");
	UARTprintf("1 - Servo\n");
	UARTprintf("2 - Stepper Motor\n");
	UARTprintf("3 - Back to previous menu\n");
	UARTprintf("Choose one of above options: ");
	receive_cmd(buffer);
}

void servo_control_menu(char *buffer){
	LCDclear();
	LCDprintf("*Servo*");
	Delay_ms(100);
	UARTprintf("\n*****Servo*****\n");
	UARTprintf("Enter Servo angle:(40 to 150)\n");
	receive_cmd(buffer);
	servoPosition(atoi(buffer));
	UARTprintf("Back to previous menu?[Y/N]\n");
	receive_cmd(buffer);
}
void stepper_motor_menu(char *buffer){
	LCDclear();
	LCDprintf("*Stepper Motor*");
	Delay_ms(100);
	UARTprintf("\n*****Stepper Motor*****\n");
	UARTprintf("1 - Control stepper motor\n");
	UARTprintf("2 - Recenter stepper motor\n");
	UARTprintf("3 - Back to previous menu\n");
	UARTprintf("Choose one of above options: ");
	receive_cmd(buffer);
}
void stepper_motor_control_menu(char *buffer, Instruction* current_instructions){
	LCDclear();
	LCDprintf("*Ctrl Stp*");
	Delay_ms(100);
	UARTprintf("\n*****Control stepper motor****\n");
	UARTprintf("Enter stepper speed:(1 or 2) \n");
	receive_cmd(buffer);
	(*current_instructions).stepper_speed = atoi(buffer);
	UARTprintf("Enter stepper angle:(0 to 180)\n");
	receive_cmd(buffer);
	(*current_instructions).stepper_target = atoi(buffer);
	UARTprintf("Back to previous menu?[Y/N]\n");
	receive_cmd(buffer);
	
}
void stepper_recenter_menu(void){
	LCDclear();
	LCDprintf("*recenter stp*");
	Delay_ms(100);
	UARTprintf("Start stepper motor recenter sequence.\n");
	stepperHoming();
	UARTprintf("Recentering finish.\n");
}

void dc_motor_control_menu(char *input_buffer, Instruction* current_instructions){
	uint8_t left_motor_dir_flag = FALSE;
	uint8_t right_motor_dir_flag = FALSE;
	
	int8_t left_motor_dir_temp;
	uint32_t left_motor_spd_temp;
	int8_t right_motor_dir_temp;
	uint32_t right_motor_spd_temp;
	
	LCDclear();
	LCDprintf("*DC Motor*");
	Delay_ms(100);
	UARTprintf("\n*****DC Motor*****\n");
//get left motor direction command
	while(left_motor_dir_flag != TRUE){
	UARTprintf("Left motor direction:(F or B) \n");
	receive_cmd(input_buffer);
	if(input_boolean_judge(input_buffer[0], 'f')){
		left_motor_dir_temp = -1;
		left_motor_dir_flag = TRUE;
	}
	else if(input_boolean_judge(input_buffer[0], 'b')){
		left_motor_dir_temp = 1;
		left_motor_dir_flag = TRUE;
	}
	else
		UARTprintf("Invalid command, please try again.\n");
	}
	left_motor_dir_flag = FALSE;
	
//get right motor direction command
	while(right_motor_dir_flag != TRUE){
	UARTprintf("Right motor direction:(F or B) \n");
	receive_cmd(input_buffer);
	if(input_boolean_judge(input_buffer[0], 'f')){
		right_motor_dir_temp = 1;
		right_motor_dir_flag = TRUE;
	}
	else if(input_boolean_judge(input_buffer[0], 'b')){
		right_motor_dir_temp = -1;
		right_motor_dir_flag = TRUE;
	}
	else
		UARTprintf("Invalid command, please try again.\n");
	}
	right_motor_dir_flag = FALSE;
	
//get left motor speed command
UARTprintf("Left motor speed:(0 to 100) \n");
receive_cmd(input_buffer);
left_motor_spd_temp = atoi(input_buffer);	
	
//get right motor speed command
UARTprintf("Right motor speed:(0 to 100) \n");
receive_cmd(input_buffer);
right_motor_spd_temp = atoi(input_buffer);	

//write the motor commands
DisableInterrupts;
(*current_instructions).DCM_Left_DIR = left_motor_dir_temp;
(*current_instructions).DCM_Left_SPD = left_motor_spd_temp;
(*current_instructions).DCM_Right_DIR = right_motor_dir_temp;
(*current_instructions).DCM_Right_SPD = right_motor_spd_temp;
EnableInterrupts;

//ask user whether want to back to previous menu or stay
UARTprintf("Back to previous menu?[Y/N]\n");
receive_cmd(input_buffer);
	
}