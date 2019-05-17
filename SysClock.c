#include "SysClock.h"
#include "utilities.h"

//******************************************************************************************
// Switch the PLL source from MSI to HSI, and select the PLL as SYSCLK source.
//******************************************************************************************
//void System_Clock_Init(void){
//	
//	uint32_t HSITrim;

//	// To correctly read data from FLASH memory, the number of wait states (LATENCY)
//  // must be correctly programmed according to the frequency of the CPU clock
//  // (HCLK) and the supply voltage of the device.		
//	FLASH->ACR &= ~FLASH_ACR_LATENCY;
//	FLASH->ACR |=  FLASH_ACR_LATENCY_2;
//		
//	// Enable the Internal High Speed oscillator (HSI)
//	RCC->CR |= RCC_CR_HSION;
//	while((RCC->CR & RCC_CR_HSIRDY) == 0);
//	// Adjusts the Internal High Speed oscillator (HSI) calibration value
//	// RC oscillator frequencies are factory calibrated by ST for 1 % accuracy at 25oC
//	// After reset, the factory calibration value is loaded in HSICAL[7:0] of RCC_ICSCR	
//	HSITrim = 16; // user-programmable trimming value that is added to HSICAL[7:0] in ICSCR.
//	RCC->CR &= ~RCC_CR_HSITRIM;
//	RCC->CR |= HSITrim << 24;
//	
//	RCC->CR    &= ~RCC_CR_PLLON; 
//	while((RCC->CR & RCC_CR_PLLRDY) == RCC_CR_PLLRDY);
//	
//	// Select clock source to PLL
//	RCC->CFGR &= ~RCC_CFGR_PLLSRC;
//	RCC->CFGR |= RCC_CFGR_PLLSRC_HSI_PREDIV; // select HSI as PLL input
//	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV1; //select no divide for HSI input to PLL
//	
//	// Make PLL as 72 MHz
//	RCC->CFGR |= RCC_CFGR_PLLNODIV; //dont divide PLL output by 2
//	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PLLMUL) | RCC_CFGR_PLLMUL9; // PLL multiplier value

//	RCC->CR   |= RCC_CR_PLLON; 
//	while((RCC->CR & RCC_CR_PLLRDY) == 0);
//	
//	// Select PLL selected as system clock
//	RCC->CFGR &= ~RCC_CFGR_SW;
//	RCC->CFGR |= RCC_CFGR_SW_PLL; // 00: MSI, 01:HSI, 10: HSE, 11: PLL
//	
//	// Wait until System Clock has been selected
//	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
//	
//	// The maximum frequency of the AHB, the APB1 and the APB2 domains is 80 MHz.
//	RCC->CFGR &= ~RCC_CFGR_HPRE;  // AHB prescaler = 1; SYSCLK not divided
//	RCC->CFGR &= ~RCC_CFGR_PPRE1; // APB high-speed prescaler (APB1) = 1, HCLK not divided
//	RCC->CFGR &= ~RCC_CFGR_PPRE2; // APB high-speed prescaler (APB2) = 1, HCLK not divided
//}

void System_Clock_Init(void){
uint32_t HSITrim;

// To correctly read data from FLASH memory, the number of wait states (LATENCY)
// must be correctly programmed according to the frequency of the CPU clock
// (HCLK) and the supply voltage of the device. 
FLASH->ACR &= ~FLASH_ACR_LATENCY;
FLASH->ACR |= FLASH_ACR_LATENCY_2;

SET_BITS(RCC->CR, RCC_CR_HSEON);
while((RCC->CR & RCC_CR_HSERDY) == 0);

/* Reset CFGR register */
RCC->CFGR &= 0xF87FC00C;

/* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE bits */
RCC->CFGR &= (uint32_t)0xFF80FFFF;

/* Reset PREDIV1[3:0] bits */
RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

CLR_BITS(RCC->CR, RCC_CR_PLLON); /// PLL off
while((RCC->CR & RCC_CR_PLLRDY) == RCC_CR_PLLRDY); // wait for PLL off

FORCE_BITS(RCC->CFGR, RCC_CFGR_PLLMUL, 0x07 << 18); // set PLL multiplier to 9
CLR_BITS(RCC->CFGR, RCC_CFGR_PLLXTPRE); // SET_BITS HSE PLL input clock div to 1
FORCE_BITS(RCC->CFGR, RCC_CFGR_PLLSRC, (RCC_CFGR_PLLSRC_HSE_PREDIV)); // use HSE as PLL input; prediv = 1

// turn on PLL and wait for it to be ready
SET_BITS(RCC->CR, RCC_CR_PLLON); 
while((RCC->CR & RCC_CR_PLLRDY) == 0);

FORCE_BITS(RCC->CFGR, RCC_CFGR_SW, 0x02); // switch SYSCLK to PLL

// Wait until System Clock has been selected
while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

SET_BITS(RCC->CR, RCC_CR_CSSON); // enable clock security

// The maximum frequency of the AHB, the APB1 and the APB2 domains is 80 MHz.
RCC->CFGR &= ~RCC_CFGR_HPRE; // AHB prescaler = 1; SYSCLK not divided
RCC->CFGR &= ~RCC_CFGR_PPRE1; // APB high-speed prescaler (APB1) = 1, HCLK not divided
RCC->CFGR &= ~RCC_CFGR_PPRE2; // APB high-speed prescaler (APB2) = 1, HCLK not divided
}

