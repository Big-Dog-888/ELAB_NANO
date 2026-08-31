#ifndef ELAB_ASSERT_H
#define ELAB_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "elab_std.h"
#include "elab_log.h"
#include "stm32f1xx_hal.h"
void elab_assert_func(void);

/* 所有断言宏统一传入 TAG */
#define elab_assert(test_)                  do { \
    if (!(test_)) \
        _assert(#test_, 0, TAG, __LINE__); \
} while (0)

#define assert(test_)                       elab_assert(test_)

#define assert_id(test_, id_)               do { \
    if (!(test_)) \
        _assert(#test_, id_, TAG, __LINE__); \
} while (0)

#define assert_name(test_, name_)           do { \
    if (!(test_)) \
        _assert(name_, 0, TAG, __LINE__); \
} while (0)

// #define assert_not_null(ptr_)               assert_name((ptr_ != NULL), #ptr_)
#define assert_not_null(ptr_)  assert((ptr_) != NULL)
void _assert(const char *str_, uint32_t id_, const char *tag, uint32_t location);

#ifdef __cplusplus
}
#endif

#endif