#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f1xx_hal.h"

/* OLED I2C 7bit 地址 */
#define OLED_ADDR       0x3C

/* OLED 屏幕尺寸 (SSD1306 128x64) */
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGES      (OLED_HEIGHT / 8)   /* 8 页，每页 8 像素高 */

/* 基础接口 */
void OLED_Init(void);
void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t *data, uint16_t len);

/* 绘图辅助 */
void OLED_Clear(void);
void OLED_FillAll(uint8_t value);

#endif