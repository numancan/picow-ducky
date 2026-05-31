#pragma once

#include "pico/assert.h"
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Halts the execution if the condition is true (Debug mode only).
 * In Release builds, this macro is completely stripped out by the compiler,
 * causing zero runtime overhead.
 */
#define ABORT_IF(cond)           \
    do {                         \
        if (cond) assert(false); \
    } while (0)

/**
 * @brief Silently halts the execution if the condition is true (Debug and Release).
 * Does not print any messages or utilize UART/USB interfaces.
 * Ideal for fatal hardware faults where standard IO might be compromised.
 */
#define HARD_ABORT_IF(cond)                 \
    do {                                    \
        if (cond) hard_assertion_failure(); \
    } while (0)

/**
 * @brief Triggers a system panic with a message if the condition is true (Debug and Release).
 * @param msg The error message to print (must be a string literal).
 * Note: The string literal will consume flash memory space in the final binary.
 */
#define PANIC_IF(cond, msg)   \
    do {                      \
        if (cond) panic(msg); \
    } while (0)

/**
 * @brief Triggers a system panic if the condition is true.
 * In Debug builds, it prints the provided message.
 * In Release builds, it halts the system but discards the message string
 * to save flash memory space.
 * @param msg The error message (must be a string literal).
 */
#define PANIC_COMPACT_IF(cond, msg)   \
    do {                              \
        if (cond) panic_compact(msg); \
    } while (0)

#ifdef __cplusplus
}
#endif