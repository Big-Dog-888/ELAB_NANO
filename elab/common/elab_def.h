#ifndef ELAB_DEF_H
#define ELAB_DEF_H


#if defined(__CC_ARM) || defined(__CLANG_ARM)
    #include "stdarg.h"
    #define ELAB_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__) 
    #include "stdarg.h"
    #define ELAB_WEAK __weak
#elif defined(__GNUC__) 
    #include "stdarg.h"
    #define ELAB_WEAK __attribute__((weak))
#else
    #error "Complier not supported!"
#endif

#endif