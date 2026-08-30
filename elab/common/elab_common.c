#include "elab_common.h"

ELAB_WEAK uint32_t elab_time_ms(void)
{
#if !defined(__linux__) //win32

    #if(ELAB_RTOS_CMSIS_OS_EN!= 0)
    //osKernelGetTickCount();
    #else
    return 0;
    #endif
#else //linux
    //return osKernelGetTickCount();
#endif
}

#if !defined(__linux__) && !defined(_WIN32)
ELAB_WEAK
#endif
void *elab_malloc(uint32_t size)
{
    return malloc(size);
}

#if !defined(__linux__) && !defined(_WIN32)
ELAB_WEAK
#endif
void elab_free(void *memory)
{
    if (memory != NULL)
    {
        free(memory);
    }
}