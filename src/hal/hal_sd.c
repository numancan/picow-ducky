#include "hal_sd.h"

#include <stdlib.h>

#include "hw_config.h"

void hal_sd_card_init(void) { sd_init_driver(); }