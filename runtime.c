#include "runtime.h"
#include <stdint.h>
#include <stdio.h>

uint8_t get_char(void) { return (uint8_t)fgetc(stdin); }

void put_char(uint8_t v) { fputc(v, stdout); }
