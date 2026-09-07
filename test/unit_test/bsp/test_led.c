#include "unity.h"
#include "Mockstm32f1xx_hal.h"
#include "led.h"

void setUp(void)
{
    Mockstm32f1xx_hal_Init();
}

void tearDown(void)
{
    Mockstm32f1xx_hal_Verify();
}

void test_led_init_should_call_hal_gpio_init(void)
{
    HAL_GPIO_Init_ExpectAnyArgs();
    LED_Init();
}

void test_led_ctl_led1_on_should_set_pin11_reset(void)
{
    HAL_GPIO_WritePin_Expect(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
    LED_Ctl(LED_1, LED_ON);
}

void test_led_ctl_led1_off_should_set_pin11_set(void)
{
    HAL_GPIO_WritePin_Expect(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
    LED_Ctl(LED_1, LED_OFF);
}

void test_led_ctl_led2_on_should_set_pin12_reset(void)
{
    HAL_GPIO_WritePin_Expect(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    LED_Ctl(LED_2, LED_ON);
}

void test_led_ctl_led2_off_should_set_pin12_set(void)
{
    HAL_GPIO_WritePin_Expect(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
    LED_Ctl(LED_2, LED_OFF);
}

void test_led_ctl_invalid_led_should_not_call_hal(void)
{
    LED_Ctl((LED_t)99, LED_ON);
}