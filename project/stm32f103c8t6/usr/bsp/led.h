#ifndef __LED_H__
#define __LED_H__

#include "stm32f1xx_hal.h"

typedef enum
{
    LED_ON = 0,
    LED_OFF = 1,
} LED_Status_t;

typedef enum
{
    LED_1 = 0,
    LED_2 = 1,
} LED_t;

void LED_Init(void);

void LED_Ctl(LED_t led, LED_Status_t status);

#endif
