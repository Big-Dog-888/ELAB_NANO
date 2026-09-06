/**
 * @file test_os_timer.c
 * @author ZC (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-09-29
 * 
 * 
 */
#if defined(__linux__) || defined(_WIN32)
 /* ==================== [Includes] ========================================== */
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <time.h>
 #include "../../../elab/os/cmsis_os.h"
 #include "../../../elab/3rd/Unity/unity.h"
 #include "../../../elab/3rd/Unity/unity_fixture.h"
 #include "../../../elab/common/elab_common.h"
 #include "../../../elab/common/elab_assert.h"
 #include "../../../elab/common/elab_log.h"
 /* ==================== [Defines] ========================================== */
 #define TAG                         "os_timer"


 /* ==================== [Macros] ============================================ */
 
 /* ==================== [Typedefs] ========================================== */
 
 /* ==================== [Static Prototypes] ========================================== */

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of ostimer.
  */
TEST_GROUP(ostimer);

/**
  * @brief  Define test fixture setup function of ostimer.
  */
TEST_SETUP(ostimer)
{


}

/**
  * @brief  Define test fixture tear down function of ostimer.
  */
TEST_TEAR_DOWN(ostimer)
{

}
    
 /* ==================== [Static Variables] ========================================== */
 
 /* ==================== [Static Functions] ================================== */

 static const osTimerAttr_t timer_attr_test =
{
    .name = "test_timer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
};
 void timer_callback(void *param)
{
  (void)param;
  elog_debug("os_timer_test");
}
 /* ==================== [Public Functions] ================================== */
TEST(ostimer, new_stop_delet_cmsis_timer)
{
    osTimerId_t timer=NULL;
    osStatus_t ret_os = osOK;
    timer = osTimerNew(timer_callback,osTimerPeriodic, NULL, &timer_attr_test);
    TEST_ASSERT_NOT_NULL(timer);
    ret_os = osTimerStart(timer, 1000);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    ret_os = osTimerStop(timer);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
    ret_os = osTimerDelete(timer);
    TEST_ASSERT_EQUAL_INT32(osOK, ret_os);
}

/**
  * @brief  Define run test cases of ostimer.
  */
TEST_GROUP_RUNNER(ostimer)
{
    RUN_TEST_CASE(ostimer, new_stop_delet_cmsis_timer);
}


#endif
 