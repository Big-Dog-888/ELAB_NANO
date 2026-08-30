#ifndef ELAB_LOG_H
#define ELAB_LOG_H

#include "elab_common.h"


#define ELAB_COLOR_ENABLE (1)


#define ELAB_TAG(_tag)   static const char *TAG = _tag

 enum elog_level_enum
{
    ELOG_LEVEL_ERROR = 1 ,
    ELOG_LEVEL_WARNING  = 2 ,
    ELOG_LEVEL_INFO  = 3 ,
    ELOG_LEVEL_DEBUG = 4 ,
    
    ELOG_LEVEL_MAX = 5,
};

 void _elog_printf(const char *name ,uint32_t line,
                uint8_t level, const char *fmt, ...);

/* Enable error level debug message */
#if ELOG_LEVEL_CURRENT >= ELOG_LEVEL_ERROR
#define elog_error(...) _elog_printf(TAG, __LINE__, ELOG_LEVEL_ERROR, __VA_ARGS__)
#else
#define elog_error(...)
#endif

#if ELOG_LEVEL_CURRENT >= ELOG_LEVEL_WARNING
#define elog_warn(...) _elog_printf(TAG, __LINE__, ELOG_LEVEL_WARNING, __VA_ARGS__)
#else
#define elog_warn(...)
#endif

#if ELOG_LEVEL_CURRENT >= ELOG_LEVEL_INFO
#define elog_info(...) _elog_printf(TAG, __LINE__, ELOG_LEVEL_INFO, __VA_ARGS__)
#else
#define elog_info(...)
#endif

#if ELOG_LEVEL_CURRENT >= ELOG_LEVEL_DEBUG
#define elog_debug(...) _elog_printf(TAG, __LINE__, ELOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define elog_debug(...)
#endif

#endif