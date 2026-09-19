#include "i2c.h"
#include "../../../../elab/common/elab_export.h"
#include "../../../../elab/common/elab_log.h"

ELAB_TAG("I2C");

I2C_HandleTypeDef hi2c1;

static void I2C1_MspInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void I2C1_Init(void)
{
    hi2c1.Instance              = I2C1;
    hi2c1.Init.ClockSpeed       = 100000;
    hi2c1.Init.DutyCycle        = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1      = 0;
    hi2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2      = 0;
    hi2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        elog_error("I2C1 init failed!");
        return;
    }

    elog_info("I2C1 init done. (PB6=SCL, PB7=SDA, 100kHz)");
}
INIT_EXPORT(I2C1_Init, EXPORT_DRVIVER);

HAL_StatusTypeDef I2C1_Send(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    return HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(DevAddress << 1), pData, Size, Timeout);
}

HAL_StatusTypeDef I2C1_Recv(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    return HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(DevAddress << 1), pData, Size, Timeout);
}

static void I2C1_MspDeInit(void)
{
    __HAL_RCC_I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        I2C1_MspInit();
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        I2C1_MspDeInit();
    }
}

static uint8_t scan_done = 0;

void I2C1_ScanPoll(void)
{
    if (scan_done) return;
    scan_done = 1;

    elog_info("=== I2C1 Scan Start ===");
    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 3, 10) == HAL_OK)
        {
            elog_info("  FOUND device at 0x%02X (7bit)", addr);
            found++;
        }
    }
    elog_info("=== Scan Done: %d device(s) found ===", found);

    if (found == 0)
    {
        elog_error("NO DEVICES FOUND! Check:");
        elog_error("  1. OLED is I2C version (4 pins)");
        elog_error("  2. SCL->PB6, SDA->PB7");
        elog_error("  3. 4.7k pull-up on SCL/SDA");
        elog_error("  4. OLED VCC=3.3V");
    }
}
POLL_EXPORT(I2C1_ScanPoll, 500);