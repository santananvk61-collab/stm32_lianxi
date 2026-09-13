

#ifndef INC_NTC_H_
#define INC_NTC_H_

#include "main.h"
#include <math.h>

float ADC2Resistance(uint32_t adcValue);
float resistance2Temperature(float R1);

#endif /* INC_NTC_H_ */
