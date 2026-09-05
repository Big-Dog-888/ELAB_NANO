#include "stm32f1xx_hal.h"
#include "debug_uart.h"
#include "../../../../elab/common/elab_export.h"
#include "../../../../elab/common/elab_log.h"
#include "../../../../elab/3rd/Shell/shell.h"
#include <stdio.h>
// #include "SEGGER_RTT.h"

ELAB_TAG("BSP");

#define SHELL_POLL_PERIOD_MS                (10)
#define SHELL_BUFFER_SIZE                   (512)

static Shell shell_uart;
static char shell_uart_buffer[SHELL_BUFFER_SIZE];

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

uint32_t elab_time(void)
{
    return HAL_GetTick();
}

uint32_t elab_time_ms(void) 
{ 
  return HAL_GetTick(); 
}



 void BSP_Init(void)
 {
  HAL_Init();
  // SEGGER_RTT_Init();
  
  SystemClock_Config();
  elab_debug_uart_init(115200);
  printf("BSP_init.\r\n");
  (void)TAG;  // ★ 加这行，告诉编译器 TAG 是故意不用的
  // static unsigned char upBuffer[1024];
  // static unsigned char downBuffer[1024];
  // SEGGER_RTT_ConfigUpBuffer(0, "up", upBuffer, sizeof(upBuffer), SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  // SEGGER_RTT_ConfigDownBuffer(0, "down", downBuffer, sizeof(downBuffer), SEGGER_RTT_MODE_NO_BLOCK_SKIP);
 }
INIT_EXPORT(BSP_Init, EXPORT_LEVEL_BSP);

void Shell_Init(void)
{
    shell_uart.read=(int16_t (*)(char *, uint16_t))elab_debug_uart_receive;
    shell_uart.write = (int16_t (*)(char *, uint16_t))elab_debug_uart_send;
    shellInit(&shell_uart, shell_uart_buffer, SHELL_BUFFER_SIZE);
}
INIT_EXPORT(Shell_Init, EXPORT_USER);

static void shell_poll(void)
{
    char byte;
    while (shell_uart.read && shell_uart.read(&byte, 1) == 1)
    {
        shellHandler(&shell_uart, byte);
    }
} 
POLL_EXPORT(shell_poll, SHELL_POLL_PERIOD_MS);