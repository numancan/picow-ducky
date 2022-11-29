#ifndef _DUCKY_PARSER_H_
#define _DUCKY_PARSER_H_

typedef struct 
{
  uint8_t command[12];
  uint8_t args[250];
} ducky_line_t;

/**
 * Parse duckyscript line
 * 
 * @param l duckyscript line (Ex. "STRING HELLO WORLD\r\n\0")
 * @return Parsed ducky_line_t or NULL for error
 */
ducky_line_t *dp_parse_line(uint8_t *l);

#endif