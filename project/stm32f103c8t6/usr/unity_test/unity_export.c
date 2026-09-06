/**
 * @file unity_export.c
 * @author ZC (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-10-09
 * 
 * 
 */
/* ==================== [Includes] ========================================== */

// #include "../../../../elab/common/elab_common.h"
// #include "../../../../elab/common/elab_export.h"
// #include "../../../../elab/3rd/Shell/shell.h"
// #include "../../../../elab/3rd/Unity/unity.h"
// #include "../../../../elab/3rd/Unity/unity_fixture.h"
/* ==================== [Defines] ========================================== */

/* ==================== [Macros] ============================================ */

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ========================================== */

/* ==================== [Static Variables] ========================================== */

/* ==================== [Static Functions] ================================== */
/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test fixture tear down function of device core
  */
#include "../../../../elab/common/elab_common.h"
#include "../../../../elab/common/elab_export.h"
#include "../../../../elab/3rd/Shell/shell.h"
#include "../../../../elab/3rd/Unity/unity.h"
#include "../../../../elab/3rd/Unity/unity_fixture.h"
int unity_export(int argc, char *argv[])
{
    return UnityMain(argc, (const char **)argv, elab_unit_test);
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    unity,
                    unity_export,
                    Run all unit tests);
/* ==================== [Public Functions] ================================== */