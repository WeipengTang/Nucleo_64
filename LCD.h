/******************************************************************************
*LCD.c -- Inplementation of setting up LCD functions and printf on the LCD display
*By Weipeng Tang
*FEB 1, 2019
********************************************************************************/
#ifndef _LCD_H
#define _LCD_H
#include "stm32f303xe.h"

#define MAX_LCD_BUFSIZ 80
#define FIRST_ROW_ADDR 0x00
#define SECOND_ROW_ADDR 0x40

#define LCD_PORT_MAP B
#define LCD_E_PIN 12
#define LCD_RS_PIN 13
#define LCD_DB4 4
#define LCD_DB5 5
#define LCD_DB6 6
#define LCD_DB7 7

//LCD port
#define LCD_PORT GPIO(LCD_PORT_MAP)->ODR

//LCD enable(clock)
#define LCD_E_BIT GPIO_ODR_(LCD_E_PIN)

//LCD rs bit
#define LCD_RS_BIT GPIO_ODR_(LCD_RS_PIN)

//data bus
#define LCD_BUS_BIT (GPIO_ODR_(LCD_DB4)|GPIO_ODR_(LCD_DB5)|GPIO_ODR_(LCD_DB6)|GPIO_ODR_(LCD_DB7))

//low nybble of a value
#define LO_NYBBLE(value) ((value)&0x0F)

//high nybble of a value
#define HI_NYBBLE(value) (((value)>>4)&0x0F)

//set LCD enable high
#define LCD_E_HI SET_BITS(LCD_PORT, LCD_E_BIT)

//set LCD enable low
#define LCD_E_LO CLR_BITS(LCD_PORT, LCD_E_BIT)

//write 4 bit value to LCD bus
#define LCD_BUS(nybble) FORCE_BITS(LCD_PORT, LCD_BUS_BIT, (nybble)<<4)

//set LCD RS to instruction mode
#define LCD_RS_IR CLR_BITS(LCD_PORT, LCD_RS_BIT)

//set LCD RS to data mode
#define LCD_RS_DR SET_BITS(LCD_PORT, LCD_RS_BIT)



//functions for LCD
void LCDinit(void);
void LCDdata(uint8_t value);
void LCDcmd(uint8_t command);
void LCDclear(void);
void LCDputc(char c);
void LCDputs(char* string);
void LCDprintf(char* fmt, ...);
void LCDClearRow1(void);
void LCDClearRow2(void);

#endif
