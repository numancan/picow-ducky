#pragma once

#include <stdint.h>

#include "lib/u8g2/csrc/u8g2.h"

void hal_display_init(u8g2_t* u8g2);
void hal_display_on(u8g2_t* u8g2);
void hal_display_off(u8g2_t* u8g2);