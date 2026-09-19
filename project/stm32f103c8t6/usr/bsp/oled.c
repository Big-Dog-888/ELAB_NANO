#include "oled.h"
#include "i2c.h"
#include "../../../../elab/common/elab_export.h"
#include "../../../../elab/common/elab_log.h"

ELAB_TAG("OLED");
/* ==================== 1. OLED 私有收发封装 ==================== */

void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    HAL_StatusTypeDef ret = I2C1_Send(OLED_ADDR, buf, 2, 100);
    if (ret != HAL_OK)
    {
        elog_error("OLED_WriteCmd 0x%02X FAILED! I2C ret=%d", cmd, ret);
    }
}

void OLED_WriteData(uint8_t *data, uint16_t len)
{
    uint8_t buf[129];
    buf[0] = 0x40;
    uint16_t sent = 0;
    while (sent < len)
    {
        uint16_t chunk = (len - sent > 128) ? 128 : (len - sent);
        for (uint16_t i = 0; i < chunk; i++)
        {
            buf[i + 1] = data[sent + i];
        }
        HAL_StatusTypeDef ret = I2C1_Send(OLED_ADDR, buf, chunk + 1, 100);
        if (ret != HAL_OK)
        {
            elog_error("OLED_WriteData FAILED! I2C ret=%d", ret);
        }
        sent += chunk;
    }
}

/* ==================== 2. OLED 初始化 ==================== */

void OLED_Init(void)
{
    OLED_WriteCmd(0xAE);   /* display off */
    OLED_WriteCmd(0xD5);   /* set display clock divide ratio */
    OLED_WriteCmd(0x80);   /* osc frequency */
    OLED_WriteCmd(0xA8);   /* set multiplex ratio */
    OLED_WriteCmd(0x3F);   /* 1/64 duty */
    OLED_WriteCmd(0xD3);   /* set display offset */
    OLED_WriteCmd(0x00);   /* no offset */
    OLED_WriteCmd(0x40);   /* set start line address */
    OLED_WriteCmd(0x8D);   /* set DC-DC enable */
    OLED_WriteCmd(0x14);   /* charge pump on */
    OLED_WriteCmd(0x20);   /* set memory addressing mode */
    OLED_WriteCmd(0x10);   /* page mode */
    OLED_WriteCmd(0xA1);   /* set segment remap */
    OLED_WriteCmd(0xC8);   /* set COM output scan direction */
    OLED_WriteCmd(0xDA);   /* set COM pins hardware configuration */
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81);   /* set contrast control */
    OLED_WriteCmd(0xFF);   /* 最大亮度 */
    OLED_WriteCmd(0xD9);   /* set pre-charge period */
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB);   /* set vcomh deselect level */
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4);   /* output follows RAM content */
    OLED_WriteCmd(0xA6);   /* normal display */
    OLED_WriteCmd(0xAF);   /* display on */

    elog_info("OLED init done.");
}
INIT_EXPORT(OLED_Init, EXPORT_DEVICE);

/* ==================== 3. 绘图辅助 ==================== */

static void OLED_WritePage(uint8_t page, uint8_t *buf)
{
    OLED_WriteCmd(0xB0 | (page & 0x07));
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    OLED_WriteData(buf, OLED_WIDTH);
}

void OLED_Clear(void)
{
    uint8_t buf[OLED_WIDTH] = {0};
    for (int p = 0; p < OLED_PAGES; p++)
    {
        OLED_WritePage(p, buf);
    }
}

void OLED_FillAll(uint8_t value)
{
    uint8_t buf[OLED_WIDTH];
    for (int i = 0; i < OLED_WIDTH; i++) buf[i] = value;
    for (int p = 0; p < OLED_PAGES; p++)
    {
        OLED_WritePage(p, buf);
    }
}

/* ==================== 4. POLL_EXPORT 测试 ==================== */

/*
 * 4 个阶段循环，每秒切换一次：
 *   stage 0 → 全屏白亮
 *   stage 1 → 全屏熄灭
 *   stage 2 → 黑白横条纹
 *   stage 3 → 渐变图案
 */
static uint8_t test_stage = 0;

void OLED_TestPoll(void)
{
    test_stage = (test_stage + 1) % 4;
    // elog_info("OLED test stage %u", test_stage);

    uint8_t buf[OLED_WIDTH];

    switch (test_stage)
    {
        case 0:
            OLED_FillAll(0xFF);
            break;

        case 1:
            OLED_Clear();
            break;

        case 2:
            for (int p = 0; p < OLED_PAGES; p++)
            {
                uint8_t pattern = (p % 2 == 0) ? 0xFF : 0x00;
                for (int i = 0; i < OLED_WIDTH; i++) buf[i] = pattern;
                OLED_WritePage(p, buf);
            }
            break;

        case 3:
            for (int p = 0; p < OLED_PAGES; p++)
            {
                for (int i = 0; i < OLED_WIDTH; i++)
                {
                    buf[i] = (uint8_t)(i + p * 16);
                }
                OLED_WritePage(p, buf);
            }
            break;
    }
}
POLL_EXPORT(OLED_TestPoll, 1000);