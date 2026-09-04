// test_debug_uart.c
// 功能：测试 elab_debug_uart 的发送和接收 + elab_assert 断言
// 使用方式：烧录到开发板，打开串口助手（115200，8N1）


#include "../../../elab/elib/elib_queue.h"
#include "../../../elab/common/elab_export.h"
#include "../../../elab/common/elab_log.h"
#include <stdio.h>
#include "led.h"

ELAB_TAG("Main");

void test_bsp_init(void)
{

}
INIT_EXPORT(test_bsp_init, EXPORT_DRVIVER);

void test_driver_init(void)
{

}
INIT_EXPORT(test_driver_init, EXPORT_DRVIVER);

void test_app_init(void)
{

    LED_Ctl(LED_1, 0);
    LED_Ctl(LED_2, 0);
}
INIT_EXPORT(test_app_init, EXPORT_APP);


// ============================================
// 主函数
// ============================================
int main(void)
{
    elab_run();
}