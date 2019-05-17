#include "LED_Flash.h"
#include "utilities.h"
#include "stm32f303xe.h"
#include "SysTick.h"
#define delay 2000000UL

void flashLEDInit(void){

	/**********************************initate the LED port and pin*****************************/
	SET_BITS(RCC->AHBENR, RCC_AHBENR_GPIOAEN);					//Enable Port A clock. Reference manual page 148 
	
	GPIO_PIN_MODE(LED_PORT, LED_PIN, OUT);							//set LED port and pin as output
	
	GPIO_PIN_DRV_TYPE(LED_PORT, LED_PIN, PP);						//SET_BITS output type as push-pull
	
}

void flashLED(void){
	
	SET_BITS(GPIO(LED_PORT)->ODR, 1UL<<LED_PIN);				//Turn on LED
	Delay_ms(100);																			//Delay 500ms
	CLR_BITS(GPIO(LED_PORT)->ODR, 1UL<<LED_PIN);				//Turn off LED
	Delay_ms(100);																			//Delay 500ms

}
