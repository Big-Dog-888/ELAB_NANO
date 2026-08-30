#ifndef DEBUG_UART_H
#define DEBUG_UART_H    

#include "stm32f1xx_hal.h"


void elab_debug_uart_init(uint32_t baudrate);
int16_t elab_debug_uart_send(void *buffer, uint16_t size);
int16_t elab_debug_uart_receive(void *buffer, uint16_t size);
void elab_debug_uart_buffer_clear(void);

#endif