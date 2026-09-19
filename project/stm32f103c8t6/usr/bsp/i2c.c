#include "i2c.h"                              /* 包含本模块的头文件，获取 I2C 接口声明 */
#include "../../../../elab/common/elab_export.h" /* 引入 ELAB 框架的导出宏（INIT_EXPORT、POLL_EXPORT 等） */
#include "../../../../elab/common/elab_log.h"     /* 引入 ELAB 日志模块，提供 elog_info / elog_error */

ELAB_TAG("I2C");                              /* 给当前编译单元打标签 "I2C"，用于日志过滤 */

I2C_HandleTypeDef hi2c1;                     /* 定义全局 I2C1 句柄，供 HAL 回调和其他模块使用 */

static void I2C1_MspInit(void)               /* I2C1 的 MspInit（MCU Support Package Init），负责底层 GPIO/时钟初始化 */
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};   /* 定义 GPIO 初始化结构体并清零，避免残留值干扰 */

    __HAL_RCC_I2C1_CLK_ENABLE();              /* 使能 I2C1 外设时钟，否则 I2C 无法工作 */
    __HAL_RCC_GPIOB_CLK_ENABLE();             /* 使能 GPIOB 时钟，I2C1 的 SCL/SDA 分别在 PB6/PB7 */

    GPIO_InitStruct.Pin   = GPIO_PIN_6 | GPIO_PIN_7;   /* 选择 PB6 (I2C1_SCL) 和 PB7 (I2C1_SDA) 两个引脚 */
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;           /* 复用开漏输出模式：I2C 总线必须用开漏，支持线与 */
    GPIO_InitStruct.Pull  = GPIO_PULLUP;               /* 内部上拉：I2C 总线空闲时应为高电平（需外部 4.7k 上拉配合） */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 引脚速度设为高速，适应 I2C 100kHz 或 400kHz 时序 */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);            /* 将上述配置写入 GPIOB 端口的寄存器 */
}

void I2C1_Init(void)                         /* I2C1 高层初始化函数，配置 I2C 参数并调用 MspInit */
{
    hi2c1.Instance              = I2C1;       /* 指定句柄对应的外设实例为 I2C1（STM32F1 的 I2C1 寄存器基地址） */
    hi2c1.Init.ClockSpeed       = 100000;    /* I2C 时钟频率设为 100kHz（标准模式），最高可设 400kHz（快速模式） */
    hi2c1.Init.DutyCycle        = I2C_DUTYCYCLE_2;         /* 占空比：Tlow/Thigh = 1:1，标准模式固定用此值 */
    hi2c1.Init.OwnAddress1      = 0;         /* 本机 7 位/10 位地址，此处只做主机所以设为 0 */
    hi2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT; /* 7 位地址模式，大多数 OLED 模块使用 7 位地址 */
    hi2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE; /* 禁用双地址模式，只使用 OwnAddress1 */
    hi2c1.Init.OwnAddress2      = 0;         /* 第二地址，双地址禁用时无意义 */
    hi2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE; /* 禁用广播呼叫模式，主机不响应广播地址 0x00 */
    hi2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;   /* 允许从机拉伸时钟（从机忙时拉低 SCL 等待），兼容性更好 */

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)       /* 调用 HAL 初始化函数，HAL_I2C_Init 内部会先写 MCR1/MCR2/CCR 等寄存器，再调用 HAL_I2C_MspInit */
    {
        elog_error("I2C1 init failed!");      /* 初始化失败时打印错误日志 */
        return;                               /* 直接返回，不再继续执行 */
    }

    elog_info("I2C1 init done. (PB6=SCL, PB7=SDA, 100kHz)"); /* 初始化成功，打印引脚分配和频率信息 */
}
INIT_EXPORT(I2C1_Init, EXPORT_DRVIVER);       /* 将 I2C1_Init 注册到 ELAB 驱动层初始化表，系统启动时自动调用 */

HAL_StatusTypeDef I2C1_Send(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) /* I2C1 主机发送封装 */
{
    return HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(DevAddress << 1), pData, Size, Timeout); /* 7位地址左移1位后变成8位地址格式(左移腾出R/W位)，调用HAL阻塞发送 */
}

HAL_StatusTypeDef I2C1_Recv(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout) /* I2C1 主机接收封装 */
{
    return HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(DevAddress << 1), pData, Size, Timeout);   /* 7位地址左移1位后调用HAL阻塞接收 */
}

static void I2C1_MspDeInit(void)              /* I2C1 的反初始化，复位 GPIO 和关闭时钟 */
{
    __HAL_RCC_I2C1_CLK_DISABLE();             /* 关闭 I2C1 时钟，降低功耗 */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7); /* 将 PB6/PB7 恢复为默认状态（输入模式） */
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) /* HAL 弱函数重写：HAL_I2C_Init 内部会回调此函数 */
{
    if (hi2c->Instance == I2C1)               /* 判断当前初始化的是否是 I2C1 */
    {
        I2C1_MspInit();                       /* 是 I2C1 则调用我们自己的 MspInit 完成 GPIO/时钟配置 */
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c) /* HAL 弱函数重写：HAL_I2C_DeInit 内部会回调此函数 */
{
    if (hi2c->Instance == I2C1)               /* 判断当前反初始化的是否是 I2C1 */
    {
        I2C1_MspDeInit();                     /* 是 I2C1 则调用我们自己的 MspDeInit */
    }
}

static uint8_t scan_done = 0;                 /* 扫描完成标志，static 限制作用域为本文件，0=未扫描 1=已扫描 */

void I2C1_ScanPoll(void)                      /* I2C 总线扫描函数，作为 poll 任务周期执行一次后即停止 */
{
    if (scan_done) return;                    /* 如果已经扫过，直接返回，避免重复扫描 */
    scan_done = 1;                            /* 标记扫描已完成 */

    elog_info("=== I2C1 Scan Start ===");     /* 打印扫描开始标志 */
    int found = 0;                            /* 记录找到的设备数量 */
    for (uint8_t addr = 1; addr < 127; addr++) /* 遍历所有 7 位地址（1~126），跳过 0（广播）和 127（保留） */
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 3, 10) == HAL_OK) /* 发送该地址，重试3次，每次超时10ms，检测从机是否应答 */
        {
            elog_info("  FOUND device at 0x%02X (7bit)", addr); /* 找到设备，打印其 7 位十六进制地址 */
            found++;                          /* 设备计数加 1 */
        }
    }
    elog_info("=== Scan Done: %d device(s) found ===", found); /* 扫描结束，打印总共找到多少个设备 */

    if (found == 0)                           /* 如果一个设备都没找到，很可能是接线问题 */
    {
        elog_error("NO DEVICES FOUND! Check:"); /* 打印排查提示 */
        elog_error("  1. OLED is I2C version (4 pins)");   /* 检查 OLED 是否是 I2C 版（4针），而非 SPI 版（7针） */
        elog_error("  2. SCL->PB6, SDA->PB7");             /* 检查接线：SCL 接 PB6，SDA 接 PB7 */
        elog_error("  3. 4.7k pull-up on SCL/SDA");       /* 检查 SCL/SDA 是否有外部 4.7kΩ 上拉电阻 */
        elog_error("  4. OLED VCC=3.3V");                  /* 检查 OLED 供电是否为 3.3V（不要接 5V 直接烧模块） */
    }
}
POLL_EXPORT(I2C1_ScanPoll, 500);              /* 将 I2C1_ScanPoll 注册为 poll 任务，每 500ms 执行一次（实际只执行一轮就停） */