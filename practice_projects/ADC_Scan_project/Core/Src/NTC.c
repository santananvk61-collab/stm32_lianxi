#include "NTC.h"

float ADC2Resistance(uint32_t adcValue)
{
    float resistance;
    resistance = (float)(adcValue * 10000.0f / (float)(4095.0f - adcValue));
    return resistance;
}

float resistance2Temperature(float R1)
{
    float B = 3950.0f;
    float R2 = 10000.0f;
    float T2 = 25.0F;
    return (1.0 / ((1.0f / B) * log(R1 / R2) + (1.0 / (T2 + 273.15))) - 273.15f);
}

