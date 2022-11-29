#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "parser.h"

#define MAX_CHAR_PER_LINE 256

// TODO: convert inline func
#define CLEAR_DL() memset(dl, 0, sizeof(ducky_line_t))
#define CPY_COMMAND(c) snprintf(dl->command, sizeof(dl->command), "%s", c)
#define CPY_ARGS(a) snprintf(dl->args, sizeof(dl->args), "%s", a)

static ducky_line_t *dl = &(ducky_line_t) {.command = "", .args= ""};

ducky_line_t *dp_parse_line(uint8_t *l)
{
  // Terminate line before new line(\r)
  char *r = strstr(l, "\r");
  if (r) *r = '\0';

  // Check line len
  if (strlen(l) > MAX_CHAR_PER_LINE) return NULL;

  // Clear old ducky line
  CLEAR_DL();
  
  char *pTemp = NULL;

  // Split command from line.
  pTemp = strtok(l, " ");
  if (pTemp) CPY_COMMAND(pTemp); else return NULL;

  // Split args from line.
  pTemp = strtok(NULL, "");
  if (pTemp) CPY_ARGS(pTemp);

  return dl;
}