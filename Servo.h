#ifndef _SERVO_H_
#define _SERVO_H_
#include <stdint.h>

#define SERVO_PORT B
#define SERVO_PIN 11	

void ServoInit(void);
void servoPosition(uint32_t angle);
uint32_t mapValue(uint32_t minIn, uint32_t maxIn, uint32_t minOut, uint32_t maxOut, uint32_t value);


#endif

