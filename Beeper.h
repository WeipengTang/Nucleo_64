#ifndef _BEEPER_H_
#define _BEEPER_H_

#define BEEPER_PORT A
#define BEEPER_PIN 5

void beeperInit(void);
void TIM8_CC_IRQHandler(void);



#endif
