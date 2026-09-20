#ifndef __I2C_H__                           /* 防止头文件被重复包含的宏守卫开始 */
#define __I2C_H__                           /* 宏守卫定义 __I2C_H__ */

#include "stm32f1xx_hal.h"                  /* 引入 STM32F1 HAL 库头文件，提供 I2C_HandleTypeDef 等类型 */

extern I2C_HandleTypeDef hi2c1;             /* 声明全局 I2C1 句柄，在 i2c.c 中定义，其他文件可直接使用 */

/* ==================== 通用 I2C 收发接口 ==================== */
/* DevAddress 必须是 7bit 地址，HAL 内部自动左移并加 R/W 位 */
HAL_StatusTypeDef I2C1_Send(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout); /* I2C1 主机发送数据，参数：从机7位地址、数据缓冲区指针、数据长度、超时(ms) */
HAL_StatusTypeDef I2C1_Recv(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout); /* I2C1 主机接收数据，参数同上 */
void I2C1_BusUnlock(void);                                                                          /* 9-SCL-clock 解锁被锁死的 I2C 总线（STM32F1 硬件 bug  workaround） */

#endif                                      /* 宏守卫结束 */