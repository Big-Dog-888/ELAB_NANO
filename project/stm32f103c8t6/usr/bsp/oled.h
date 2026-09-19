#ifndef __OLED_H__                          /* 防止头文件被重复包含的宏守卫开始 */
#define __OLED_H__                          /* 宏守卫定义 __OLED_H__ */

#include "stm32f1xx_hal.h"                  /* 引入 STM32F1 HAL 库头文件，提供基础类型定义 */

/* OLED I2C 7bit 地址 */
#define OLED_ADDR       0x3C                /* SSD1306 OLED 模块的 I2C 7位地址（写地址为 0x78，读地址为 0x79） */

/* OLED 屏幕尺寸 (SSD1306 128x64) */
#define OLED_WIDTH      128                 /* OLED 屏幕横向像素点数 128 */
#define OLED_HEIGHT     64                  /* OLED 屏幕纵向像素点数 64 */
#define OLED_PAGES      (OLED_HEIGHT / 8)   /* SSD1306 按页寻址，每页 8 像素高，共 64/8=8 页 */

/* 基础接口 */
void OLED_Init(void);                       /* OLED 初始化函数，发送 SSD1306 初始化命令序列 */
void OLED_WriteCmd(uint8_t cmd);             /* 向 OLED 写一条命令（控制字节 0x00 + 命令字节） */
void OLED_WriteData(uint8_t *data, uint16_t len); /* 向 OLED 写显示数据（控制字节 0x40 + 数据流，自动分片发送） */

/* 绘图辅助 */
void OLED_Clear(void);                      /* 清屏，将全部显存写 0x00（熄灭） */
void OLED_FillAll(uint8_t value);           /* 全屏填充指定字节值，0xFF 全亮，0x00 全灭 */

#endif                                      /* 宏守卫结束 */