#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "handler.h"
#include "keyboard.h"

static ducky_line_t dh_last_line;
static uint16_t dh_repeat_count = 0;

static dh_status_t dh_status = DH_STATUS_READY;

static inline void dh_set_status(dh_status_t s) { dh_status = s; }

dh_status_t dh_is_ready() { return dh_status == DH_STATUS_READY; }

int64_t delay_callback(alarm_id_t id, void *user_data) {
  dh_set_status(DH_STATUS_READY);
  return 0;
}

void dh_handle_dline(ducky_line_t *dl)
{
	
  if (strcmp(dl->command, "STRING") == 0) {

			kb_send_string(dl->args);
			dh_set_status(DH_STATUS_BUSY);
	}
  else if (strcmp(dl->command, "DELAY") == 0) {

    uint32_t delay_ms = strtol(dl->args, (char **)NULL, 10);

    add_alarm_in_ms(delay_ms, delay_callback, NULL, false);
    dh_set_status(DH_STATUS_DELAYED);
  }
  else if (strcmp(dl->command, "REPEAT") == 0) {

    dh_repeat_count = strtol(dl->args, (char **)NULL, 10);

    dh_handle_dline(&dh_last_line);
    dh_repeat_count--;
    return;
  }
  else {

    uint8_t keycode[6] = { kb_str_to_keycode(dl->command) };
    
    if (dl->args) {

      keycode[1] = kb_str_to_keycode(dl->args);

      if (!keycode[1]) {
        for (size_t i = 0; i < strlen(dl->args); i++)
        {
          keycode[i + 1] = kb_ascii_to_keycode(dl->args[i]);
        }
      }
    }
    
    kb_send_keycodes(keycode);
    dh_set_status(DH_STATUS_BUSY);
    // printf("%s COMMAND NOT FOUND \n", dl->command);
  }

  // TODO: while repeating, same line overwrite  
  memcpy(&dh_last_line, dl, sizeof(ducky_line_t));
}

void kb_sent_complete_cb()
{
  if (dh_repeat_count > 0) {
    dh_handle_dline(&dh_last_line);
    dh_repeat_count--;
    return;
  }

	dh_set_status(DH_STATUS_READY);
}