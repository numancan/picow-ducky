#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

#ifndef LOG_LEVEL
    #if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        #define LOG_LEVEL LOG_LEVEL_DEBUG
    #else
        #define LOG_LEVEL LOG_LEVEL_ERROR
    #endif
#endif

#define _LOG(level, tag, format, ...) printf("[" level "] (%s) " format "\n", tag, ##__VA_ARGS__)

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(tag, format, ...) _LOG("E", tag, format, ##__VA_ARGS__)
#else
#define LOG_ERROR(tag, format, ...) \
    do {                            \
    } while (0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN(tag, format, ...) _LOG("W", tag, format, ##__VA_ARGS__)
#else
#define LOG_WARN(tag, format, ...) \
    do {                           \
    } while (0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(tag, format, ...) _LOG("I", tag, format, ##__VA_ARGS__)
#else
#define LOG_INFO(tag, format, ...) \
    do {                           \
    } while (0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(tag, format, ...) _LOG("D", tag, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(tag, format, ...) \
    do {                            \
    } while (0)
#endif

#ifdef __cplusplus
}
#endif