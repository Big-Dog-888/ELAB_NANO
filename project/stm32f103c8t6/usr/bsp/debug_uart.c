#include "debug_uart.h"
#include "stdint.h"
#include "stdio.h"
#include "../../../elab/elib/elib_queue.h"
#include "../../../elab/common/elab_export.h"
#include "../../../elab/common/elab_log.h"

ELAB_TAG("UART");

#define USARTx                           USART1
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()
#define USARTx_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_PIN                    GPIO_PIN_9
#define USARTx_RX_PIN                    GPIO_PIN_10
#define USARTx_PORT                      GPIOA
#define USARTx_IRQn                      USART1_IRQn

#define ELAB_DEBUG_UART_BUFFER_TX               (1024)
#define ELAB_DEBUG_UART_BUFFER_RX               (16)

#define UART_BUFFER_TX               (1024)
#define UART_BUFFER_RX               (16)

#define UART_DEFAULT_BAUDRATE   115200       /* 默认波特率：集中一处，改这里就行 */

UART_HandleTypeDef huart1;
static elib_queue_t queue_rx;
static uint8_t buffer_rx[ELAB_DEBUG_UART_BUFFER_RX];
static elib_queue_t queue_tx;
static uint8_t buffer_tx[ELAB_DEBUG_UART_BUFFER_TX];
static uint8_t byte_recv;


UART_HandleTypeDef huart3;
static elib_queue_t uart3_queue_rx;
static uint8_t uart3_buffer_rx[UART_BUFFER_RX];
static elib_queue_t uart3_queue_tx;
static uint8_t uart3_buffer_tx[UART_BUFFER_TX];
static uint8_t uart3_byte_recv;

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(huart->Instance == USARTx)
    {

    /* USART1 clock enable */
    USARTx_CLK_ENABLE();

    USARTx_GPIO_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = USARTx_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = USARTx_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USARTx_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USARTx_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USARTx_IRQn);
    }

    if(huart->Instance == USART3)
    {

        /* USART3 clock enable */
        USART3_CLK_ENABLE();
        
        USART3_GPIO_CLK_ENABLE();
        /**USART3 GPIO Configuration
        PB10     ------> USART3_TX
        PB11     ------> USART3_RX
        */
        GPIO_InitStruct.Pin = USART3_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = USART3_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(USART3_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
    }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */

/* private functions -------------------------------------------------------- */
/**
  * @brief  The weak UART tx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    uint8_t byte = 0;
    uint8_t uart3_byte = 0;
    if (UartHandle->Instance == USARTx)
    {
        elib_queue_pop(&queue_tx, 1);
        if (elib_queue_pull(&queue_tx, &byte, 1))
        {
            HAL_UART_Transmit_IT(&huart1, &byte, 1);
        }
    }
    
    if (UartHandle->Instance == USART3)
    {
        elib_queue_pop(&uart3_queue_tx, 1);
        if (elib_queue_pull(&uart3_queue_tx, &uart3_byte, 1))
        {
            HAL_UART_Transmit_IT(&huart3, &uart3_byte, 1);
        }
    }

}

/**
  * @brief  The weak UART rx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if (UartHandle->Instance == USARTx)
    {
        HAL_UART_Receive_IT(&huart1, &byte_recv, 1);
        elib_queue_push(&queue_rx, &byte_recv, 1);
    }
    if (UartHandle->Instance == USART3)
    {
        HAL_UART_Receive_IT(&huart3, &uart3_byte_recv, 1);
        elib_queue_push(&uart3_queue_rx, &uart3_byte_recv, 1);
    }
}

/**
  * @brief This function handles USART3 and USART4 interrupts.
  */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}


/* USART1 init function */

void elab_debug_uart_init(uint32_t baudrate)
{

  huart1.Instance = USARTx;
  huart1.Init.BaudRate = baudrate;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
  HAL_UART_Receive_IT(&huart1, &byte_recv, 1);
  elib_queue_init(&queue_rx, buffer_rx, ELAB_DEBUG_UART_BUFFER_RX);
  elib_queue_init(&queue_tx, buffer_tx, ELAB_DEBUG_UART_BUFFER_TX);
}

/**
  * @brief  Send data to the debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_send(void *buffer, uint16_t size)
{
    int16_t ret = 0;
    uint8_t byte = 0;

    HAL_NVIC_DisableIRQ(USARTx_IRQn);
    if (elib_queue_is_empty(&queue_tx))
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
        if (elib_queue_pull(&queue_tx, &byte, 1) == 1)
        {
            HAL_UART_Transmit_IT(&huart1, &byte, 1);
        }
    }
    else
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
    }
    HAL_NVIC_EnableIRQ(USARTx_IRQn);

    return ret;
}

/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_receive(void *buffer, uint16_t size)
{
    int16_t ret = 0;

    HAL_NVIC_DisableIRQ(USARTx_IRQn);
    ret = elib_queue_pull_pop(&queue_rx, buffer, size);
    HAL_NVIC_EnableIRQ(USARTx_IRQn);

    return ret;
}

/**
  * @brief  Clear buffer of the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void elab_debug_uart_buffer_clear(void)
{
    HAL_NVIC_DisableIRQ(USARTx_IRQn);

    elib_queue_clear(&queue_rx);
    elib_queue_clear(&queue_tx);

    HAL_NVIC_EnableIRQ(USARTx_IRQn);
}















void uart_init(uint32_t baudrate)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = baudrate;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);
    HAL_UART_Receive_IT(&huart3, &byte_recv, 1);

    elib_queue_init(&uart3_queue_rx, uart3_buffer_rx, UART_BUFFER_RX);
    elib_queue_init(&uart3_queue_tx, uart3_buffer_tx, UART_BUFFER_TX);
}


/**
  * @brief  Send data to the debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t uart_send(void *buffer, uint16_t size)
{
    int16_t ret = 0;
    uint8_t byte = 0;

    HAL_NVIC_DisableIRQ(USART3_IRQn);   
    if (elib_queue_is_empty(&uart3_queue_tx))
    {
        ret = elib_queue_push(&uart3_queue_tx, buffer, size);
        if (elib_queue_pull(&uart3_queue_tx, &byte, 1) == 1)
        {
            HAL_UART_Transmit_IT(&huart3, &byte, 1);
        }
    }
    else
    {
        ret = elib_queue_push(&uart3_queue_tx, buffer, size);
    }
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    return ret;
}

/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t uart_receive(void *buffer, uint16_t size)
{
    int16_t ret = 0;

    HAL_NVIC_DisableIRQ(USART3_IRQn);
    ret = elib_queue_pull_pop(&uart3_queue_rx, buffer, size);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    return ret;
}

/**
  * @brief  Clear buffer of the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void uart_buffer_clear(void)
{
    HAL_NVIC_DisableIRQ(USART3_IRQn);

    elib_queue_clear(&uart3_queue_rx);
    elib_queue_clear(&uart3_queue_tx);

    HAL_NVIC_EnableIRQ(USART3_IRQn);
}



void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}


static void uart_auto_init(void)             /* ELAB 自动初始化包装函数（无参，供 INIT_EXPORT 注册） */
{
    uart_init(UART_DEFAULT_BAUDRATE);        /* 内部调带参的真正初始化函数 */
    uart_buffer_clear();
    elog_info("UART %u auto init done.", UART_DEFAULT_BAUDRATE);
}
INIT_EXPORT(uart_auto_init, EXPORT_DEVICE);  /* 注册到设备层初始化，系统启动自动执行 */


void uart_test(void)
{
    uart_buffer_clear();
    uart_send("Hello, World!\n", 14);
    elog_info("UART test done.");

}
POLL_EXPORT(uart_test, 1000);             /* 每 1000ms 刷新一次，数字计数器会快速递增便于观察 */


#ifdef __ARMCC_VERSION
    void _sys_exit(int x)
    {
        (void)x;
        while (1);
    }

    int _ttywrch(int ch)
    {
        uint8_t c = (uint8_t)ch;
        elab_debug_uart_send(&c, 1);
        return ch;
    }

    int fputc(int ch, FILE *f)
    {
        (void)f;
        uint8_t c = (uint8_t)ch;
        elab_debug_uart_send(&c, 1);
        return ch;
    }

    int fgetc(FILE *f)
    {
        (void)f;
        uint8_t c = 0;
        elab_debug_uart_receive(&c, 1);
        return c;
    }
#elif defined(__GNUC__) // GCC Compiler
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
  PUTCHAR_PROTOTYPE
  {
      elab_debug_uart_send(&ch, 1);
      return ch;
  }

#endif
