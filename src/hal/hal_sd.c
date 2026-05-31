#include "hal_sd.h"

#include <assert.h>
#include <stdlib.h>

#include "hw_config.h"
#include "middleware/sys_fault.h"

void hal_sd_card_init(void) { ABORT_IF(!sd_init_driver()); }