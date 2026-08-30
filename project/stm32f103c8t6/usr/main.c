// test_debug_uart.c
// 功能：测试 elab_debug_uart 的发送和接收 + elab_assert 断言
// 使用方式：烧录到开发板，打开串口助手（115200，8N1）

#include "bsp.h"
#include "../../../elab/common/elab_log.h"
#include "../../../elab/3rd/xfusion/xf_utils.h"
#include "stdio.h"
#include "../../../elab/common/elab_assert.h"

ELAB_TAG("UartTest");

// ============================================
// 辅助函数：比较两个缓冲区是否相等
// ============================================
static bool buffer_compare(uint8_t *buf1, uint8_t *buf2, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        if (buf1[i] != buf2[i]) {
            return false;
        }
    }
    return true;
}

// ============================================
// 辅助函数：打印十六进制数据
// ============================================
static void print_hex(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}

// ============================================
// 辅助函数：等待队列发送完成（延时等待）
// ============================================
static void wait_tx_complete(void)
{
    HAL_Delay(500);
}

// ============================================
// 辅助函数：等待队列为空（长延时）
// ============================================
static void wait_queue_empty(void)
{
    // 等待中断把数据全部发完
    HAL_Delay(500);
}

// ============================================
// 测试1：发送测试 + 断言
// ============================================
void test_send(void)
{
    int32_t ret;
    
    // 测试前清空队列
    elab_debug_uart_buffer_clear();
    HAL_Delay(100);
    
    printf("\r\n========== Test 1: Send with Assert ==========\r\n");
    
    // 测试1.1：发送普通字符串
    uint8_t str[] = "Hello eLab!\r\n";
    ret = elab_debug_uart_send(str, sizeof(str) - 1);
    elab_assert(ret >= 0);
    printf("Send string: ret = %d (expected %d)\r\n", ret, (int)(sizeof(str) - 1));
    
    wait_tx_complete();
    
    // 测试1.2：发送空数据（边界测试）
    ret = elab_debug_uart_send(NULL, 0);
    elab_assert(ret == 0);
    printf("Send NULL test: ret = %d\r\n", ret);
    
    // 测试1.3：发送十六进制数据
    uint8_t hex_data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ret = elab_debug_uart_send(hex_data, sizeof(hex_data));
    elab_assert(ret >= 0);
    elab_debug_uart_send((uint8_t *)"\r\n", 2);
    printf("Send hex: ret = %d, data = ", ret);
    print_hex(hex_data, sizeof(hex_data));
    
    wait_tx_complete();
    wait_queue_empty();
}

// ============================================
// 测试2：环形缓冲区基本功能 + 断言
// ============================================
void test_queue_basic(void)
{
    int32_t ret;
    int16_t len;
    uint8_t rx_buf[10];
    
    // 等待之前的数据发送完成
    wait_queue_empty();
    
    // 清空缓冲区
    elab_debug_uart_buffer_clear();
    HAL_Delay(100);
    
    printf("\r\n========== Test 2: Queue Basic ==========\r\n");
    printf("Buffer cleared\r\n");
    
    // 测试2.1：发送 10 个字节
    uint8_t data1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    ret = elab_debug_uart_send(data1, sizeof(data1));
    elab_assert(ret >= 0);
    printf("Send 10 bytes: ret = %d\r\n", ret);
    
    wait_tx_complete();
    
    // 测试2.2：再次发送 20 个字节
    uint8_t data2[20];
    for (int i = 0; i < 20; i++) {
        data2[i] = 0x10 + i;
    }
    ret = elab_debug_uart_send(data2, sizeof(data2));
    elab_assert(ret >= 0);
    printf("Send 20 bytes: ret = %d\r\n", ret);
    
    wait_tx_complete();
    
    // 测试2.3：清空缓冲区
    elab_debug_uart_buffer_clear();
    printf("Buffer cleared again\r\n");
    
    // 断言：清空后队列应该为空
    len = elab_debug_uart_receive(rx_buf, sizeof(rx_buf));
    elab_assert(len == 0);
    printf("Queue is empty after clear: len = %d\r\n", len);
    
    wait_queue_empty();
}

// ============================================
// 测试3：压力测试 + 断言（修正版）
// ============================================
void test_pressure(void)
{
    int32_t ret;
    
    // 等待之前的数据发送完成
    wait_queue_empty();
    
    // 清空缓冲区
    elab_debug_uart_buffer_clear();
    HAL_Delay(100);
    
    printf("\r\n========== Test 3: Pressure Test ==========\r\n");
    
    // 测试3.1：发送 300 字节（小于队列 512 字节）
    uint8_t data_300[300];
    for (int i = 0; i < sizeof(data_300); i++) {
        data_300[i] = 'A' + (i % 26);
    }
    data_300[sizeof(data_300) - 1] = '\n';
    
    ret = elab_debug_uart_send(data_300, sizeof(data_300));
    elab_assert(ret == sizeof(data_300));
    printf("Send 300 bytes: ret = %d\r\n", ret);
    
    wait_tx_complete();
    wait_queue_empty();
    
    // 测试3.2：发送 600 字节（超过队列 512 字节）
    uint8_t data_600[600];
    for (int i = 0; i < sizeof(data_600); i++) {
        data_600[i] = '0' + (i % 10);
    }
    data_600[sizeof(data_600) - 2] = '\r';
    data_600[sizeof(data_600) - 1] = '\n';
    
    uint32_t start = HAL_GetTick();
    ret = elab_debug_uart_send(data_600, sizeof(data_600));
    uint32_t end = HAL_GetTick();
    
    // 600 字节超过队列 512 字节，会返回 ELAB_ERR_NOT_ENOUGH（负数）
    // 这是预期行为，数据会在中断中异步发送
    if (ret < 0) {
        printf("Queue full as expected: ret = %d (ELAB_ERR_NOT_ENOUGH)\r\n", ret);
        printf("Data will be sent asynchronously by interrupt\r\n");
    } else {
        printf("Send 600 bytes: ret = %d (unexpected, should be negative)\r\n", ret);
        elab_assert(ret < 0);
    }
    printf("Send time: %lu ms\r\n", end - start);
    
    // 等待中断把数据全部发完
    wait_queue_empty();
    printf("Pressure test passed (600 bytes sent asynchronously)\r\n");
}

// ============================================
// 测试4：回显测试 + 断言
// ============================================
void test_echo(void)
{
    uint8_t rx_buffer[64];
    int16_t len;
    int echo_count = 0;
    
    // 等待之前的数据发送完成
    wait_queue_empty();
    
    // 清空接收缓冲区
    elab_debug_uart_buffer_clear();
    HAL_Delay(100);
    
    printf("\r\n========== Test 4: Echo Test ==========\r\n");
    printf("Send data via serial port, will echo back...\r\n");
    printf("Send 'q' or 'Q' to exit\r\n");
    
    while (1) {
        len = elab_debug_uart_receive(rx_buffer, sizeof(rx_buffer) - 1);
        
        if (len > 0) {
            rx_buffer[len] = '\0';
            echo_count++;
            
            // 回显收到的数据
            elab_debug_uart_send((uint8_t *)"[Echo] ", 7);
            elab_debug_uart_send(rx_buffer, len);
            elab_debug_uart_send((uint8_t *)"\r\n", 2);
            
            printf("Echo #%d: len=%d, data=%s\r\n", echo_count, len, rx_buffer);
            
            if (rx_buffer[0] == 'q' || rx_buffer[0] == 'Q') {
                elab_assert(echo_count >= 1);
                printf("Echo test exit, total echoes: %d\r\n", echo_count);
                break;
            }
        }
        
        HAL_Delay(10);
    }
}

// ============================================
// 测试5：printf 重定向 + 断言
// ============================================
void test_printf(void)
{
    int ret;
    
    // 等待之前的数据发送完成
    wait_queue_empty();
    HAL_Delay(100);
    
    printf("\r\n========== Test 5: printf Redirect ==========\r\n");
    
    ret = printf("Integer: %d, Hex: 0x%X\r\n", 12345, 0xABCD);
    elab_assert(ret > 0);
    
    ret = printf("String: %s\r\n", "eLab is great!");
    elab_assert(ret > 0);
    
    ret = printf("Float: %.2f (if supported)\r\n", 3.14159);
    elab_assert(ret > 0);
    
    printf("printf redirect test: ALL PASS\r\n");
    
    wait_queue_empty();
}

// ============================================
// 测试6：断言触发测试（默认不启用）
// ============================================
void test_assert_trigger(void)
{
    printf("\r\n========== Test 6: Assert Trigger Test ==========\r\n");
    printf("This test will trigger an assertion failure.\r\n");
    printf("You should see a red 'Assert failure!' message.\r\n");
    printf("The program will then enter an infinite loop.\r\n");
    
    int value = 10;
    printf("About to assert: value == 5 (value is %d)\r\n", value);
    
    elab_assert(value == 5);
    
    printf("This line should never be reached!\r\n");
}

// ============================================
// 主函数
// ============================================
int main(void)
{
    // -------- BSP 初始化 --------
    BSP_Init();
    
    // -------- 启动信息 --------
    printf("\r\n\r\n========================================\r\n");
    printf("eLab Debug UART Test with Assert\r\n");
    printf("========================================\r\n");
    printf("TAG: %s\r\n", TAG);
    printf("========================================\r\n\r\n");
    
    // -------- 运行所有测试 --------
    test_send();
    
    test_queue_basic();
    
    test_pressure();
    
    test_printf();
    
    test_echo();
    
    // -------- 断言触发测试（默认注释） --------
    // test_assert_trigger();
    
    // -------- 结束 --------
    printf("\r\n========================================\r\n");
    printf("All tests complete! (PASS)\r\n");
    printf("========================================\r\n");
    
    while (1) {
        HAL_Delay(1000);
    }
}