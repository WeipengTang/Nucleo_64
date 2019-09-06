#ifndef _UTILITIES_H
#define _UTILITIES_H
#include "stm32f303xe.h"

//mask value with the lower four bits
#define LOW(value) ((value)&0xFF)

//set arbitrary pattern of bits
#define SET_BITS(port, mask) ((port)|=(mask))

//clear arbitrary pattern of bits
#define CLR_BITS(port, mask) ((port)&=(~(mask)))

//flip arbitrary pattern of bits
#define FLIP_BITS(port, mask) ((port)^=(mask))

//force a subset of bits in a port to a value
#define FORCE_BITS(port, mask, value) (port)=((port&(~(mask)))|((value)&(mask)))

//macros for STM32L476VG
//set pin function
#define GPIO_PIN_MODE(port, pin, mode) FORCE_BITS(GPIO(port)->MODER, (3UL<<(pin*2)), ((mode)<<(pin*2)))
#define IN 0 		 //input
#define OUT 1		 //output
#define AF 2		 //alternate function
#define ANALOG 3 //analog

//set pin output type
#define GPIO_PIN_DRV_TYPE(port, pin, type) FORCE_BITS(GPIO(port)->OTYPER, (1UL<<(pin)), ((type)<<(pin)))
#define PP 0 //pull-up/pull-down
#define OD 1 //open-drain

//set pin input/output type
#define GPIO_PIN_PP_TYPE(port, pin, type) FORCE_BITS(GPIO(port)->PUPDR, (3UL<<(pin*2)), ((type)<<(pin*2)))
#define NPP 0 //no pull-up, no pull-down
#define PU 1	//pull-up
#define PD 2	//pull-down
#define RS 3	//reserved - don't use

//set port name to target port 
#define GPIO(port) GPIOx(port)
#define GPIOx(port) GPIO##port

//choosing the output pin
#define GPIO_ODR_(pin) GPIO_ODR_x(pin)
#define GPIO_ODR_x(pin) GPIO_ODR_##pin

//select the clock output port
#define EN_CLK_PORT(port) RCC->AHBENR |= RCC_AHBENR_GPIO(port)
#define RCC_AHBENR_GPIO(port) RCC_AHBENR_GPIO##port##EN

//select alternate function
//#define GPIO_AF_TYPE(port, pin, type) FORCE_BITS(GPIO(port)->AFR[(pin)], (0xFUL<<(4*(pin&0b0111))), ((type)<<(4*(pin&0b0111)))
//#define AFR(pin) AFR##(pin>>3)



struct instruction{
	uint32_t stepper_target;
	uint8_t stepper_speed;
	int8_t DCM_Left_DIR;
	uint32_t DCM_Left_SPD;
	int8_t DCM_Right_DIR;
	uint32_t DCM_Right_SPD;
	
};

typedef struct instruction Instruction;

void receive_cmd(char *buffer);
uint8_t input_boolean_judge(char input, char expected_letter);
void main_menu(char *buffer);
void camera_menu(char *buffer);
void servo_control_menu(char *buffer);
void stepper_motor_menu(char *buffer);
void stepper_motor_control_menu(char *buffer, Instruction* current_instructions);
void stepper_recenter_menu(void);
void dc_motor_control_menu(char *input_buffer, Instruction* current_instructions);
#endif
