#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_uint(uint32_t n);
void uart_put_int(int32_t n);

#endif
