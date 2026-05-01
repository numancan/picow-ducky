#pragma once

#ifdef DEBUG
#include "stdio.h"  // ?
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

#define PANIC_IF(message, assertion) \
    do {                             \
        if (!(assertion)) {          \
            panic(message);          \
        }                            \
    } while (0)