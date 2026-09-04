#include "led.h"
#include "../../../../elab/common/elab_export.h"
GPIO_InitTypeDef GPIO_InitStruct = {0};
void LED_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
INIT_EXPORT(LED_Init, EXPORT_DEVICE);

void LED_Ctl(LED_t led, LED_Status_t status)
{
    switch (led)
    {
        case LED_1:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, (GPIO_PinState)status);
            break;
        case LED_2:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, (GPIO_PinState)status);
            break;
        default: 
            break;
    }
}
