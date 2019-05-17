/******************************************************************************
*LCD.c -- Inplementation of setting up LCD functions and printf on the LCD display
*By Weipeng Tang
*FEB 1, 2019
********************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include "LCD.h"
#include "utilities.h"
#include "stm32f303xe.h"
#include "SysTick.h"
#include "LCD command definitions.h"

void LCDinit(void){
	//Pin assignments
	//PB12 -- LCD_E
	//PB13 -- LCD_RS
	//PB4  -- LCD_DB4
	//PB5  -- LCD_DB5
	//PB6  -- LCD_DB6
	//PB7  -- LCD_DB7
	
	//initial ports
		RCC->AHBENR |= RCC_AHBENR_GPIOBEN;  // Enable clock of Port B
		
	//set PB12, PB13, PB4, PB5, PB6, PB7 as output
		GPIO_PIN_MODE(LCD_PORT_MAP, LCD_RS_PIN, OUT);	
		GPIO_PIN_MODE(LCD_PORT_MAP, LCD_E_PIN, OUT);
		GPIO_PIN_MODE(LCD_PORT_MAP, LCD_DB4, OUT);
		GPIO_PIN_MODE(LCD_PORT_MAP, LCD_DB5, OUT);
		GPIO_PIN_MODE(LCD_PORT_MAP, LCD_DB6, OUT);
		GPIO_PIN_MODE(LCD_PORT_MAP, LCD_DB7, OUT);
	
	
	//set PB12, PB13, PB4, PB5, PB6, PB7 as pull-up/pull-down mode
		GPIO_PIN_DRV_TYPE(LCD_PORT_MAP, LCD_RS_PIN, PP);
		GPIO_PIN_DRV_TYPE(LCD_PORT_MAP, LCD_E_PIN, PP);
		GPIO_PIN_DRV_TYPE(LCD_PORT_MAP, LCD_DB4, PP);
		GPIO_PIN_DRV_TYPE(LCD_PORT_MAP, LCD_DB5, PP);
		GPIO_PIN_DRV_TYPE(LCD_PORT_MAP, LCD_DB6, PP);
		GPIO_PIN_DRV_TYPE(LCD_PORT_MAP, LCD_DB7, PP);
		
	//set PB12, PB13, PB4, PB5, PB6, PB7 as no pull-up, no pull-down
		GPIO_PIN_PP_TYPE(LCD_PORT_MAP, LCD_RS_PIN, NPP);
		GPIO_PIN_PP_TYPE(LCD_PORT_MAP, LCD_E_PIN, NPP);
		GPIO_PIN_PP_TYPE(LCD_PORT_MAP, LCD_DB4, NPP);
		GPIO_PIN_PP_TYPE(LCD_PORT_MAP, LCD_DB5, NPP);
		GPIO_PIN_PP_TYPE(LCD_PORT_MAP, LCD_DB6, NPP);
		GPIO_PIN_PP_TYPE(LCD_PORT_MAP, LCD_DB7, NPP);
		
	
		
	
	// run through sync sequence from datasheet to start 4-bit interface    
    LCD_E_HI;
    LCD_BUS( 0x03 );      // wake up display & sync
    LCD_E_LO;
    
    Delay_ms( 5 );

    LCD_E_HI;
    LCD_BUS( 0x03 );      // wake up display & sync
    LCD_E_LO;

    Delay_ms( 1 );   
    
    LCD_E_HI;
    LCD_BUS( 0x03 );      // wake up display & sync
    LCD_E_LO;
    
    LCD_E_HI;
    LCD_BUS( 0x02 );      // wake up display & sync - go to 4-bit mode
    LCD_E_LO;

	  Delay_ms( 2 );

// now that we're sync'd and in 4-bit mode, issue commands to configure the display
    LCDcmd( LCD_CMD_FUNCTION | LCD_FUNCTION_4BIT | LCD_FUNCTION_2LINES | LCD_FUNCTION_5X8FONT );
		Delay_10us(4);			//delay 40us
    LCDcmd( LCD_CMD_DISPLAY | LCD_DISPLAY_OFF );
		Delay_10us(4);			//delay 40us
    LCDclear();		//clear display
    LCDcmd( LCD_CMD_ENTRY | LCD_ENTRY_MOVE_CURSOR | LCD_ENTRY_INC );
		Delay_10us(4);			//delay 40us
    LCDcmd( LCD_CMD_DISPLAY | LCD_DISPLAY_ON | LCD_DISPLAY_NOCURSOR | LCD_DISPLAY_NOBLINK );
		Delay_10us(4);			//delay 40us

}
void LCDclear(void){
	LCDcmd(LCD_CMD_CLEAR);	//command display to clear everthing
	Delay_ms(17);							//delay 16.4ms
}

void LCDputc(char c){
	static uint8_t line;
	if(c =='\n'){																		//check if the input character is a newline character
		if(line == 0)
			LCDcmd(LCD_CMD_SET_DDADDR|SECOND_ROW_ADDR);
			//LCDClearRow2();		//if the input is a newline character and at first line, start to write to second line address
//		else 
//			LCDcmd(LCD_CMD_SET_DDADDR|FIRST_ROW_ADDR);
//			//LCDClearRow1();		//if the input is a newline character and at second line, start to write to first line address
//		line = 1 - line;															//switch line
	}
	else{
		LCDdata(c);																		//send character data
	}
	Delay_10us(4);																				//Delay 40us
}
void LCDputs(char* str){
	while(*str) LCDputc(*str++);										//send all characters that are in the string
	
}
void LCDprintf(char *fmt, ...){										//LCDprintf() that uses similar format of parameters as a standard printf()
	va_list args;
	
	char buffer[MAX_LCD_BUFSIZ];
	
	va_start(args, fmt);
	vsnprintf(buffer, MAX_LCD_BUFSIZ, fmt, args);
	va_end(args);
	
	LCDputs(buffer);
}

//Internal functions
void LCDdata(uint8_t value){
	
	LCD_E_LO;											//pull the E low
	LCD_RS_DR;										//set RS to data register
	LCD_E_HI;											//pull the E high
	LCD_BUS(HI_NYBBLE(value));		//output upper 4 bits of data
	//Delay_10us(1);											//Delay in order to meet min. pulse width requirement
	LCD_E_LO;											//pull the E low
	LCD_E_HI;											//pull the E high
	LCD_BUS(LO_NYBBLE(value));		//output lower 4 bits of data
	//Delay_10us(1);											//Delay in order to meet min. pulse width requirement
	LCD_E_LO;											//pull the E low
}

void LCDcmd(uint8_t command){
	
	LCD_E_LO;											//pull the E low
	LCD_RS_IR;										//set RS to instruction register
	LCD_E_HI;											//pull the E high
	LCD_BUS(HI_NYBBLE(command));	//output upper 4 bits of data
	//Delay_10us(1);											//Delay in order to meet min. pulse width requirement
	LCD_E_LO;											//pull the E low
	LCD_E_HI;											//pull the E high
	LCD_BUS(LO_NYBBLE(command));	//output lower 4 bits of data
	//Delay_10us(1);											//Delay in order to meet min. pulse width requirement
	LCD_E_LO;											//pull the E low
}

void LCDClearRow1(void){
	LCDcmd(LCD_CMD_SET_DDADDR|FIRST_ROW_ADDR);
	for(uint8_t i = 0; i < 18; i++){
		LCDprintf(" ");
	}
	LCDcmd(LCD_CMD_SET_DDADDR|FIRST_ROW_ADDR);	
}

void LCDClearRow2(void){
	LCDcmd(LCD_CMD_SET_DDADDR|SECOND_ROW_ADDR);
	for(uint8_t i = 0; i < 18; i++){
		LCDprintf(" ");
	}
	LCDcmd(LCD_CMD_SET_DDADDR|SECOND_ROW_ADDR);	
}