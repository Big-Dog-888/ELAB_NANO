/**
 * @file test_demo.c
 * @author ZC (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-09-29
 * 
 * 
 */
/* ==================== [Includes] ========================================== */
 #include "../../../elab/3rd/Unity/unity.h"
 #include "../../../elab/3rd/Unity/unity_fixture.h"
 #include "../../../elab/common/elab_common.h"
 #include "../../../elab/common/elab_assert.h"
 #include "../../../elab/common/elab_log.h"
/* ==================== [Defines] ========================================== */
ELAB_TAG("test_demo");                      
/* ==================== [Macros] ============================================ */

TEST_GROUP(mytest); //设置测试组名称

TEST_SETUP(mytest) //测试组的初始化函数
{
    //初始化代码
}

TEST_TEAR_DOWN(mytest) //测试组的清理函数
{
    //清理代码
}
/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ========================================== */

/* ==================== [Static Variables] ========================================== */

/* ==================== [Static Functions] ================================== */

/* ==================== [Public Functions] ================================== */
TEST(mytest, test1) //定义一个测试用例
{
    int a = 5;
    int b = 5;
    elog_debug("This is a debug message in test1.");
    TEST_ASSERT_EQUAL_INT(a, b); //断言a和b相等
}


TEST_GROUP_RUNNER(mytest) //定义测试组运行函数
{
    RUN_TEST_CASE(mytest, test1); //运行test1测试用例
}

/* ----------------------------- end of file -------------------------------- */
