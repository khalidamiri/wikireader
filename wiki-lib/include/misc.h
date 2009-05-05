#ifndef MISC_H
#define MISC_H

#include <stdlib.h>

// character input
int serial_input_available(void);
int serial_input_char(void);

// character output
void print_char(char c);
void print(const char *txt);

// decimal output
void print_dec32(uint32_t value);

// hexadecimal output
void hex_dump(const uint8_t *buf, uint32_t size);
void print_byte(uint8_t val);
void print_u32(uint32_t val);

// simple busy wait delay loops
void delay(uint32_t nops);  // deprecated
void delay_us(unsigned int microsec);

#endif /* MISC_H */
