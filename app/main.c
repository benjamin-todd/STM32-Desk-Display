#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"

int main(void) {
    clock_init();     // 100 MHz
    systick_init();   // 1 ms tick
    uart_init();      // uart test
    
    

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER |= (1 << (13 * 2));

    uart_puts("hello\n");
    uint32_t count = 0;

    while (1) {
        GPIOC->ODR ^= (1 << 13);
        uart_puts("count: ");
        uart_put_uint(count++);
        uart_puts("\n");
        delay_ms(500);

    }
}
