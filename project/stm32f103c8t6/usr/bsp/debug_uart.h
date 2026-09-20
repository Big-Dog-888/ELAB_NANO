#ifndef DEBUG_UART_H
#define DEBUG_UART_H    

#include "stm32f1xx_hal.h"


void elab_debug_uart_init(uint32_t baudrate);
int16_t elab_debug_uart_send(void *buffer, uint16_t size);
int16_t elab_debug_uart_receive(void *buffer, uint16_t size);
void elab_debug_uart_buffer_clear(void);


extern UART_HandleTypeDef huart3;


void uart_init(uint32_t baudrate);
void uart_buffer_clear(void);
int16_t uart_receive(void *buffer, uint16_t size);
int16_t uart_receive_timeout(void *buffer, uint16_t size,
                             uint32_t timeout_ms, uint32_t silence_ms);
int16_t uart_send(void *buffer, uint16_t size);

#endif