#pragma once

// X-macro helpers: given a LIST(macro) that expands macro(name, str, ...) for
// each entry, DECLARE_ENUM/ENUM_TO_STR_SWITCH generate the enum and its
// name-lookup switch from the single list, keeping both in sync.
#define ENUM_GEN_DEFINE(name, ...) name,

#define ENUM_GEN_CASE(name, str, ...) \
    case name: return str;

#define DECLARE_ENUM(TypeName, COUNT_NAME, LIST) typedef enum { LIST(ENUM_GEN_DEFINE) COUNT_NAME } TypeName;

#define ENUM_TO_STR_SWITCH(v, LIST) \
    switch (v) {                    \
        LIST(ENUM_GEN_CASE)         \
        default: return "Unknown";  \
    }
