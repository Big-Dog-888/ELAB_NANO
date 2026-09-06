
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "../3rd/Unity/unity_fixture.h"
#include "../3rd/Shell/shell.h"
#include "../common/elab_common.h"
#include "../common/elab_export.h"

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test fixture tear down function of device core
  */
static int unity_export(int argc, const char * argv[])
{
    return UnityMain(argc, argv, elab_unit_test);
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    unity,
                    unity_export,
                    Run all unit tests);

/* ----------------------------- end of file -------------------------------- */


