#include "oled.h"                              /* 包含本模块的头文件，获取 OLED_ADDR、OLED_WIDTH 等宏和函数声明 */
#include "i2c.h"                               /* 包含 I2C 模块头文件，使用 I2C1_Send 接口向 OLED 发数据 */
#include "../../../../elab/common/elab_export.h" /* 引入 ELAB 框架的导出宏（INIT_EXPORT、POLL_EXPORT） */
#include "../../../../elab/common/elab_log.h"     /* 引入 ELAB 日志模块，提供 elog_info / elog_error */

ELAB_TAG("OLED");                              /* 给当前编译单元打标签 "OLED"，用于日志过滤 */
/* ==================== 1. OLED 私有收发封装 ==================== */

void OLED_WriteCmd(uint8_t cmd)                /* 向 OLED 发送一条命令（SSD1306 通过 I2C 控制字节区分命令和数据） */
{
    uint8_t buf[2] = {0x00, cmd};              /* 构造发送缓冲区：[控制字节, 命令字节]，0x00 表示后续是命令 */
    HAL_StatusTypeDef ret = I2C1_Send(OLED_ADDR, buf, 2, 100); /* 通过 I2C1 发送，从机地址 OLED_ADDR(0x3C)，2字节，超时100ms */
    if (ret != HAL_OK)                         /* 如果发送返回值不是 HAL_OK，说明 I2C 通信失败 */
    {
        elog_error("OLED_WriteCmd 0x%02X FAILED! I2C ret=%d", cmd, ret); /* 打印失败的命令字节和 HAL 返回值，便于排查 */
    }
}

void OLED_WriteData(uint8_t *data, uint16_t len) /* 向 OLED 发送显示数据（显存内容），自动分片防止 I2C 缓冲区溢出 */
{
    uint8_t buf[129];                          /* 发送缓冲区：1字节控制字 + 最多128字节数据，SSD1306 一页正好128列 */
    buf[0] = 0x40;                             /* 控制字节 0x40 表示后续是显示数据（D/C 位 = 1） */
    uint16_t sent = 0;                         /* 已发送字节计数，用于循环终止 */
    while (sent < len)                         /* 循环直到所有数据都发送完毕 */
    {
        uint16_t chunk = (len - sent > 128) ? 128 : (len - sent); /* 本帧发送多少字节：剩余超过128就发128，否则发剩下的 */
        for (uint16_t i = 0; i < chunk; i++)   /* 将本帧数据拷贝到 buf[1..chunk] 位置 */
        {
            buf[i + 1] = data[sent + i];       /* buf[0] 已放控制字 0x40，数据从 buf[1] 开始填 */
        }
        HAL_StatusTypeDef ret = I2C1_Send(OLED_ADDR, buf, chunk + 1, 100); /* 发送本帧：控制字 + chunk 字节数据，共 chunk+1 字节 */
        if (ret != HAL_OK)                     /* 如果这一帧发送失败 */
        {
            elog_error("OLED_WriteData FAILED! I2C ret=%d", ret); /* 打印错误日志，注意不 return 以保证后续帧继续尝试 */
        }
        sent += chunk;                         /* 更新已发送计数，准备下一轮 */
    }
}

/* ==================== 2. OLED 初始化 ==================== */

void OLED_Init(void)                           /* SSD1306 初始化序列，按数据手册顺序发送 20 多条命令 */
{
    OLED_WriteCmd(0xAE);   /* display off：先关闭显示再配置，避免配置过程花屏 */
    OLED_WriteCmd(0xD5);   /* set display clock divide ratio：设置时钟分频命令 */
    OLED_WriteCmd(0x80);   /* osc frequency：分频系数 0x80 → 分频 = 1，振荡器频率默认 */
    OLED_WriteCmd(0xA8);   /* set multiplex ratio：设置复用率命令 */
    OLED_WriteCmd(0x3F);   /* 1/64 duty：64 行复用（128x64 屏用 64），0x3F = 63，实际是 64 行 */
    OLED_WriteCmd(0xD3);   /* set display offset：设置显示偏移命令 */
    OLED_WriteCmd(0x00);   /* no offset：显示偏移 0，第一行像素映射到 COM0 */
    OLED_WriteCmd(0x40);   /* set start line address：设置显存起始行为 0 */
    OLED_WriteCmd(0x8D);   /* set DC-DC enable：电荷泵设置命令 */
    OLED_WriteCmd(0x14);   /* charge pump on：开启内部电荷泵，否则 OLED 不够亮 */
    OLED_WriteCmd(0x20);   /* set memory addressing mode：设置显存寻址模式命令 */
    OLED_WriteCmd(0x10);   /* page mode：选择页寻址模式（水平地址模式也可以，这里用页模式） */
    OLED_WriteCmd(0xA1);   /* set segment remap：列映射，A1 = SEG0 映射到列 127（水平镜像） */
    OLED_WriteCmd(0xC8);   /* set COM output scan direction：COM 扫描方向，C8 = 从 COM63 到 COM0（垂直镜像） */
    OLED_WriteCmd(0xDA);   /* set COM pins hardware configuration：COM 引脚配置命令 */
    OLED_WriteCmd(0x12);   /* 0x12 = 禁用左右 COM 重映射，交替 COM 引脚配置（128x64 必须设为 0x12） */
    OLED_WriteCmd(0x81);   /* set contrast control：对比度设置命令 */
    OLED_WriteCmd(0xFF);   /* 最大亮度：对比度寄存器设为 255，屏幕最亮 */
    OLED_WriteCmd(0xD9);   /* set pre-charge period：预充电周期命令 */
    OLED_WriteCmd(0xF1);   /* 0xF1 = Phase1=15 DCLK, Phase2=1 DCLK，典型值 */
    OLED_WriteCmd(0xDB);   /* set vcomh deselect level：VCOMH 电压命令 */
    OLED_WriteCmd(0x40);   /* 0x40 = VCOMH = 0.77 x VCC，消除残影的典型值 */
    OLED_WriteCmd(0xA4);   /* output follows RAM content：输出遵循显存内容（不用全亮测试模式） */
    OLED_WriteCmd(0xA6);   /* normal display：正常显示模式（A7 是反色显示） */
    OLED_WriteCmd(0xAF);   /* display on：配置完毕，打开屏幕显示 */

    elog_info("OLED init done.");              /* 初始化成功打印日志 */
}
INIT_EXPORT(OLED_Init, EXPORT_DEVICE);          /* 将 OLED_Init 注册到 ELAB 设备层初始化表，系统启动时在 I2C 之后自动调用 */

/* ==================== 3. 绘图辅助 ==================== */

static void OLED_WritePage(uint8_t page, uint8_t *buf) /* 写一页（8像素高 x 128列）显存，static 私有函数 */
{
    OLED_WriteCmd(0xB0 | (page & 0x07));      /* 设置页地址：0xB0~0xB7 对应 page0~page7，与 0x07 与运算保证范围 */
    OLED_WriteCmd(0x00);                      /* 设置列地址低 4 位为 0（列从 0 开始） */
    OLED_WriteCmd(0x10);                      /* 设置列地址高 4 位为 0（0x10~0x1F 对应列高4位） */
    OLED_WriteData(buf, OLED_WIDTH);          /* 发送 128 字节显存数据到当前页 */
}

void OLED_Clear(void)                          /* 清屏：将所有页的显存写 0x00，全部熄灭 */
{
    uint8_t buf[OLED_WIDTH] = {0};             /* 构造一个 128 字节全 0 的缓冲区 */
    for (int p = 0; p < OLED_PAGES; p++)       /* 遍历所有 8 页 */
    {
        OLED_WritePage(p, buf);                /* 每页都写入全 0 缓冲区 */
    }
}

void OLED_FillAll(uint8_t value)               /* 全屏填充指定值：0xFF 全亮，0x00 全灭，或任意图案 */
{
    uint8_t buf[OLED_WIDTH];                   /* 128 字节缓冲区，放在栈上 */
    for (int i = 0; i < OLED_WIDTH; i++) buf[i] = value; /* 把缓冲区每个字节都填成用户指定的 value */
    for (int p = 0; p < OLED_PAGES; p++)       /* 遍历所有 8 页 */
    {
        OLED_WritePage(p, buf);                /* 每页都写入相同的填充值 */
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
static uint8_t test_stage = 0;                 /* 当前测试阶段，static 限制作用域为本文件，初始为 0 */

void OLED_TestPoll(void)                       /* OLED 测试轮询函数，每秒执行一次，循环 4 种显示效果 */
{
    test_stage = (test_stage + 1) % 4;         /* 阶段切换：0→1→2→3→0→...，用模 4 实现循环 */
    // elog_info("OLED test stage %u", test_stage); /* 调试日志（已注释），取消注释可观察阶段切换 */

    uint8_t buf[OLED_WIDTH];                   /* 每页的数据缓冲区 */

    switch (test_stage)                        /* 根据当前阶段执行不同的显示效果 */
    {
        case 0:                                /* 阶段 0：全屏点亮 */
            OLED_FillAll(0xFF);                /* 调 OLED_FillAll，所有显存写 0xFF → 所有像素亮 */
            break;                             /* 跳出 switch */

        case 1:                                /* 阶段 1：全屏熄灭 */
            OLED_Clear();                      /* 调 OLED_Clear，所有显存写 0x00 → 所有像素灭 */
            break;                             /* 跳出 switch */

        case 2:                                /* 阶段 2：黑白交替横条纹（每页全亮或全灭） */
            for (int p = 0; p < OLED_PAGES; p++) /* 遍历 8 个页 */
            {
                uint8_t pattern = (p % 2 == 0) ? 0xFF : 0x00; /* 偶数页全亮(0xFF)，奇数页全灭(0x00) → 黑白横条纹 */
                for (int i = 0; i < OLED_WIDTH; i++) buf[i] = pattern; /* 本页 128 列全部填同一种 pattern */
                OLED_WritePage(p, buf);        /* 把构造好的条纹数据写入第 p 页 */
            }
            break;                             /* 跳出 switch */

        case 3:                                /* 阶段 3：渐变图案（对角线渐变） */
            for (int p = 0; p < OLED_PAGES; p++) /* 遍历 8 个页 */
            {
                for (int i = 0; i < OLED_WIDTH; i++) /* 遍历本页的 128 列 */
                {
                    buf[i] = (uint8_t)(i + p * 16); /* 数值 = 列索引 + 页索引×16，形成对角线渐变效果 */
                }
                OLED_WritePage(p, buf);        /* 把渐变数据写入第 p 页 */
            }
            break;                             /* 跳出 switch */
    }
}
POLL_EXPORT(OLED_TestPoll, 1000);              /* 将 OLED_TestPoll 注册为 poll 任务，每 1000ms(1秒) 执行一次 */