#include "stm32f4xx.h"
#include "uart.h"

void uart_init(void) {
    // turn on clocks for GPIOA and USART1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // PA9 and PA10 -> alternate function mode (binary 10, 2 bits per pin)
    GPIOA->MODER &= ~((3U << (9 * 2)) | (3U << (10 * 2)));
    GPIOA->MODER |=  (2U << (9 * 2)) | (2U << (10 * 2));

    // pick AF7 (= USART1) for PA9 and PA10 (4 bits per pin, pins 8-15 live in AFR[1])
    GPIOA->AFR[1] &= ~((0xFU << ((9 - 8) * 4)) | (0xFU << ((10 - 8) * 4)));
    GPIOA->AFR[1] |=  (7U << ((9 - 8) * 4)) | (7U << ((10 - 8) * 4));

    // speed: 100 MHz / 115200 baud = 868
    USART1->BRR = 868;

    // turn on the USART, its transmitter, and its receiver
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart_putc(char c) {
    while (!(USART1->SR & USART_SR_TXE));   // wait until there's room
    USART1->DR = c;                          // hand over one character
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');   // terminals need "go to start of line" too
        }
        uart_putc(*s++);
    }
}

void uart_put_uint(uint32_t n) {
    char digits[10];   // a 32-bit number has at most 10 digits
    int i = 0;

    if (n == 0) {
        uart_putc('0');
        return;
    }
    while (n > 0) {
        digits[i++] = '0' + (n % 10);   // peel off the last digit
        n /= 10;
    }
    while (i > 0) {
        uart_putc(digits[--i]);         // they came out backwards, so print in reverse
    }
}

void uart_put_int(int32_t n) {
    if (n < 0) {
        uart_putc('-');
        uart_put_uint(-(uint32_t)n);
    } else {
        uart_put_uint(n);
    }
}
