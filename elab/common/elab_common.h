#ifndef ELAB_COMMON_H
#define ELAB_COMMON_H

#include "elab_std.h"
#include "elab_def.h"

uint32_t elab_time_ms(void);

void *elab_malloc(uint32_t size);
void elab_free(void *memory);

#endif