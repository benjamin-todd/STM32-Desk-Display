#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"
#include "st7789.h"

int main(void) {
    clock_init();
    systick_init();
    uart_init();

    uart_puts("\nScreen test\n");
    st7789_init();

    const uint16_t colors[] = { COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_WHITE, COLOR_BLACK };

    while (1) {
        for (int i = 0; i < 5; i++) {
            uint32_t start = millis();
            st7789_fill_screen(colors[i]);
            uint32_t took = millis() - start;

            uart_puts("full-screen fill: ");
            uart_put_uint(took);
            uart_puts(" ms\n");

            delay_ms(1000);
        }
    }
}
