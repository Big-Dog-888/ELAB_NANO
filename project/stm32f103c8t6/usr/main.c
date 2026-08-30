/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "../../../elab/common/elab_log.h"
#include "../../../elab/3rd/xfusion/xf_utils.h"
#include "stdio.h"
#include "../../../elab/common/elab_assert.h"


uint8_t data[]={0xaa,0xbc,0xff,0x12,0xef};
uint8_t string[]="hello world";
uint8_t string2[]="hello world\r\n";

ELAB_TAG("main");

typedef struct
{
  int id;
  char* name;
  int value;
}example_obj_t;

void use_obj(example_obj_t* obj)
{
  assert_not_null(obj);
  // assert_name(obj != NULL, "obj is NULL");
  (void)obj;
}

void example_obj(uint8_t data)
{
  elab_assert(data >= 0 && data <= 80);
  // assert_name(data != NULL, "data is NULL");
  // (void)data;
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  BSP_Init();
  example_obj_t* obj = NULL;
  uint8_t data = 1000;
  // static char buff[1024] = "hello world\r\n";
  while (1)
  {
    HAL_Delay(3000);
    example_obj(data);
    // use_obj(obj);



    // SEGGER_RTT_printf(0, "%s", buff);    // 发送RTT
    // elog_debug("Hello World!");
    // elog_info("Hello World!");
    // elog_warn("Hello World!");
    // elog_error("Hello World!");

    // XF_LOG_BUFFER_HEX(data,sizeof(data));
    // XF_LOG_BUFFER_HEXDUMP(string,sizeof(string));
    // XF_LOG_BUFFER_HEXDUMP_ESCAPE(string2,sizeof(string2));
  }
}


