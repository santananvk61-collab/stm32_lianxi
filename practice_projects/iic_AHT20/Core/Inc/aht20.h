/*
 * aht20.h
 *
 *  Created on: Aug 20, 2026
 *      Author: zj
 */

#ifndef INC_AHT20_H_
#define INC_AHT20_H_

#include "i2c.h"

void AHT20_Init();

void AHT20_Read(float *humidity, float *temperature);

#endif /* INC_AHT20_H_ */
