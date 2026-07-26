#pragma once

#include "pico/assert.h"
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

// Programmer-error check. Debug only: stripped entirely in release builds (zero cost).
#define ABORT_IF(cond)           \
    do {                         \
        if (cond) assert(false); \
    } while (0)

// Silent halt, no IO. Debug and release. For fatal hardware faults where UART/USB may be compromised.
#define HARD_ABORT_IF(cond)                 \
    do {                                    \
        if (cond) hard_assertion_failure(); \
    } while (0)

// Unrecoverable failure. Debug and release; msg is kept in flash (must be a string literal).
#define PANIC_IF(cond, msg)   \
    do {                      \
        if (cond) panic(msg); \
    } while (0)

// Unrecoverable failure. msg is printed in debug builds only, stripped in release to save flash.
#define PANIC_COMPACT_IF(cond, msg)   \
    do {                              \
        if (cond) panic_compact(msg); \
    } while (0)

#ifdef __cplusplus
}
#endif