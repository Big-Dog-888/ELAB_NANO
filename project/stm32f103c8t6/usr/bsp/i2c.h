#ifndef __I2C_H__
#define __I2C_H__

#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c1;   /* 全局句柄 */

/* ==================== 通用 I2C 收发接口 ==================== */
/* DevAddress 必须是 7bit 地址，HAL 内部自动左移并加 R/W 位 */
HAL_StatusTypeDef I2C1_Send(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef I2C1_Recv(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif