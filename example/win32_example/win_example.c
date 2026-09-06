#include <stdio.h>
#include <stdlib.h>
#include "../../elab/common/elab_common.h"
#include "../../elab/common/elab_log.h"
#include "../../elab/common/elab_export.h"
#include "../../elab/common/elab_assert.h"
#include "../../elab/os/cmsis_os.h"
#include "../../elab/3rd/Shell/shell.h"
#include "elab_config.h"


ELAB_TAG("win_example");

#define SHELL_POLL_PERIOD_MS                (10)
#define SHELL_BUFFER_SIZE                   (512)

static Shell shell_uart;
static char shell_uart_buffer[SHELL_BUFFER_SIZE];



void test_export(void) {
    elog_info("This is a test export function.");
}
INIT_EXPORT(test_export,EXPORT_APP);

void test_function(int *a) {
    assert(a != NULL);
}


void shell_init(void)
{
  elab_debug_uart_init(115200);
  shell_uart.read=(int16_t (*)(char *, uint16_t))elab_debug_uart_receive;
  shell_uart.write = (int16_t (*)(char *, uint16_t))elab_debug_uart_send;

 shellInit(&shell_uart, shell_uart_buffer, SHELL_BUFFER_SIZE);

 elog_info("Shell init finish");
}
INIT_EXPORT(shell_init,EXPORT_LEVEL_BSP);


static void shell_poll(void)
{
    char byte;
    while (shell_uart.read && shell_uart.read(&byte, 1) == 1)
    {
        shellHandler(&shell_uart, byte);

    }
   
} 
POLL_EXPORT(shell_poll, SHELL_POLL_PERIOD_MS);


/* ==================== Shell Commands (synced from Keil main.c) ==================== */

int func(int argc, char *argv[])
{
    elog_info("%dparameter(s)\r\n", argc);
    for (int i = 1; i < argc; i++)
    {
        elog_info("%s\r\n", argv[i]);
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), func, func, test);


void test_cmd_shell(void)
{
    elog_info("This is a test command for shell.");
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
test_cmd_shell, test_cmd_shell, "A test command for shell");


int funcParam(int i, char ch, char *str)
{
    elog_info("input int: %d, char: %c, string: %s\r\n", i, ch, str);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), funcParam, funcParam, "A function with parameters(int, char, string)  and return");


void FloatParam(int a, float b,int c,float d)
{
    elog_info("%d, %f, %d, %f \r\n", a, b, c, d );
}
SHELL_EXPORT_CMD_AGENCY(SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
FloatParam, FloatParam, FloatParam,
p1, SHELL_PARAM_FLOAT(p2), p3, SHELL_PARAM_FLOAT(p4));


int gccFloatParam(int argc, char *argv[])
{
   (void)argc;
       for (int i = 0; i <= 4; i++) {
        printf("argv[%d]=%p, str=%s\r\n", i, argv[i], argv[i]);
    }
    float test=11.7;
    printf("test=%f\r\n", test);
    int a;
    float b;
    int c;
    float d;
    sscanf(argv[1], "%d", &a);
    sscanf(argv[2], "%f", &b);
    sscanf(argv[3], "%d", &c);
    sscanf(argv[4], "%f", &d);
    printf("%d, %f, %d, %f \r\n", a, b, c, d );
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
gccFloatParam, gccFloatParam, "A function with float parameters in gcc");


void key_hello(void)
{
    elog_info("Hello World\r\n");
}
SHELL_EXPORT_KEY(SHELL_CMD_PERMISSION(0), ESH_KEY_CTRL_PLUS_A, key_hello, key_hello);


int varInt = 0;
SHELL_EXPORT_VAR(SHELL_CMD_TYPE(SHELL_TYPE_VAR_INT)|SHELL_CMD_PERMISSION(0), varInt, &varInt, "An integer variable");

char str[]="test string val";
SHELL_EXPORT_VAR(SHELL_CMD_TYPE(SHELL_TYPE_VAR_STRING)|SHELL_CMD_PERMISSION(0), varStr, str, "A string variable");


Shell *shell;
static bool start_flag=0;
void test_poll_shell(void)
{
shell = shellGetCurrent();
start_flag=1;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_DISABLE_RETURN,
test_poll_shell, test_poll_shell, "A polling function for shell");


static void timer_poll(void)
{
  if(start_flag)
  {
    char msg[] = "Hello, this is a shell output with end line!\r\n";
    shellWriteEndLine(shell, msg, strlen(msg));
  }
} 
POLL_EXPORT(timer_poll, 300);


/* ==================== osTimer (synced from Keil main.c) ==================== */

void timer_callback(void *param)
{
  (void)param;
  elog_debug("os_timer_test");
}
static const osTimerAttr_t timer_attr_test =
{
    .name = "test_timer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
};
void os_timer_init(void)
{
osTimerId_t timer=osTimerNew(timer_callback,osTimerPeriodic,NULL,&timer_attr_test);
osTimerStart(timer,1000);
}
INIT_EXPORT(os_timer_init, EXPORT_APP);


int main() {
    elog_info("Hello eLab!");
    elog_debug("This is a debug message.");
    elog_warn("This is a warning message.");
    elog_error("This is an error message.");

    elab_run();

    return 0;
}