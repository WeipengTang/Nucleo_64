/******************************************************************************
*UART.c -- Inteface of setting up UART function and send a string to the port
*By Weipeng Tang
*Jan 17, 2019
********************************************************************************/
#ifndef _UART_H
#define _UART_H
#define MAX_UART_BUFSIZ 50
#include "stm32f303xe.h"

#define RS232_TXD_PORT C
#define RS232_TXD_PIN 4
#define RS232_RXD_PORT C
#define RS232_RXD_PIN 5
#define RS232_CTS_PORT A
#define RS232_CTS_PIN 11
#define RS232_RTS_PORT A
#define RS232_RTS_PIN 12

//Pin assignment
//PC4 - USART1_TX(AF7) - RS232_TxD
//PC5 - USART1_RX(AF7) - RS232_RxD
//PA11 - USART1_CTS(AF7) - RS232_CTS
//PA12 - USART1_RTS(AF7) -RS232_RTS

void UARTprintf(char *fmt, ...);
void UARTputs(char *str);
void UARTputc(char cx);			 //push one character to the transmission register
void UARTInit(void);         //initiate the UART1
void USART1_IRQHandler(void);
void receive(USART_TypeDef *USARTx, uint8_t *buffer, volatile uint32_t *pCounter);
int8_t UARTDequeue(void);
int8_t UARTNotEmpty(void);
int8_t UARTCheckEnter(void);
void UARTString(char *cx);


#endif
