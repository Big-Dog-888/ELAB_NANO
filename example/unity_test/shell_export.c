
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include "../../elab/3rd/Shell/shell.h"
#include "../../elab/common/elab_assert.h"
#include "../../elab/common/elab_export.h"
#include "../../elab/common/elab_common.h"
#include "../../elab/os/cmsis_os.h"

ELAB_TAG("ShellExport");

/* private defines ---------------------------------------------------------- */
#define ELAB_SHELL_BUFF_SIZE                    (2048)

typedef struct esh_key_info
{
    uint8_t key_id;
    uint8_t count;
    uint8_t value[5];
} esh_key_info_t;

/* private variables -------------------------------------------------------- */
static const osThreadAttr_t thread_sh_attr =
{
    .name = "shell",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

static Shell shell;
static uint8_t buff_shell[ELAB_SHELL_BUFF_SIZE];

/* private function prototype ----------------------------------------------- */
static uint8_t _func_key_parser(uint8_t *key, uint8_t count);

/* public function ---------------------------------------------------------- */
/**
  * @brief  Shell exporting fucntion
  * @param  None
  * @retval None
  */
static void shell_export(void)
{
    elab_debug_uart_init(115200);
    shell.read = (int16_t (*)(char *, uint16_t))elab_debug_uart_receive;
    shell.write = (int16_t (*)(char *, uint16_t))elab_debug_uart_send;
    shellInit(&shell, buff_shell, ELAB_SHELL_BUFF_SIZE);
    shell.backsapceMode = SHELL_BACKSPACE_MODE_BACK_SPACE_BACK;

    osThreadId_t thread = osThreadNew(shellTask, (void*)&shell, &thread_sh_attr);
    elab_assert(thread != NULL);
}

/**
  * @brief  Export pin device board level init and exit function
  */
INIT_EXPORT(shell_export, EXPORT_LEVEL_HW_INDEPNEDENT);

/* ----------------------------- end of file -------------------------------- */
