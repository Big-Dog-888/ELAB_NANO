#ifndef STM32F1XX_HAL_H
#define STM32F1XX_HAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

typedef enum
{
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT_PP,
    GPIO_MODE_OUTPUT_OD,
    GPIO_MODE_AF_PP,
    GPIO_MODE_AF_OD,
    GPIO_MODE_ANALOG
} GPIO_Mode;

typedef enum
{
    GPIO_NOPULL = 0,
    GPIO_PULLUP,
    GPIO_PULLDOWN
} GPIO_Pull;

typedef enum
{
    GPIO_SPEED_FREQ_LOW = 0,
    GPIO_SPEED_FREQ_MEDIUM,
    GPIO_SPEED_FREQ_HIGH,
    GPIO_SPEED_FREQ_VERY_HIGH
} GPIO_Speed;

typedef struct
{
    uint32_t Pin;
    GPIO_Mode Mode;
    GPIO_Pull Pull;
    GPIO_Speed Speed;
} GPIO_InitTypeDef;

typedef struct
{
    uint32_t CRL;
    uint32_t CRH;
    uint32_t IDR;
    uint32_t ODR;
    uint32_t BSRR;
    uint32_t BRR;
    uint32_t LCKR;
} GPIO_TypeDef;

#define GPIOA   ((GPIO_TypeDef *)0x40010800)
#define GPIOB   ((GPIO_TypeDef *)0x40010C00)
#define GPIOC   ((GPIO_TypeDef *)0x4001100)
#define GPIOD   ((GPIO_TypeDef *)0x40011400)
#define GPIOE   ((GPIO_TypeDef *)0x40011800)

#define GPIO_PIN_0      ((uint16_t)0x0001)
#define GPIO_PIN_1      ((uint16_t)0x0002)
#define GPIO_PIN_2      ((uint16_t)0x0004)
#define GPIO_PIN_3      ((uint16_t)0x0008)
#define GPIO_PIN_4      ((uint16_t)0x0010)
#define GPIO_PIN_5      ((uint16_t)0x0020)
#define GPIO_PIN_6      ((uint16_t)0x0040)
#define GPIO_PIN_7      ((uint16_t)0x0080)
#define GPIO_PIN_8      ((uint16_t)0x0100)
#define GPIO_PIN_9      ((uint16_t)0x0200)
#define GPIO_PIN_10     ((uint16_t)0x0400)
#define GPIO_PIN_11     ((uint16_t)0x0800)
#define GPIO_PIN_12     ((uint16_t)0x1000)
#define GPIO_PIN_13     ((uint16_t)0x2000)
#define GPIO_PIN_14     ((uint16_t)0x4000)
#define GPIO_PIN_15     ((uint16_t)0x8000)

#define HAL_OK          0
#define HAL_ERROR       1
#define HAL_BUSY        2
#define HAL_TIMEOUT     3

#define HAL_RCC_GPIOA_CLK_ENABLE()   do {} while (0)
#define __HAL_RCC_GPIOA_CLK_ENABLE() do {} while (0)

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t Delay);

void __disable_irq(void);
void __enable_irq(void);

#ifdef __cplusplus
}
#endif

#endif