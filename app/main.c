#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"
#include "uart.h"
#include "i2c.h"
#include "bme280.h"

#define BME280_ADDR 0x76

// print a number stored as hundredths
static void print_hundredths(int32_t v) {
    if (v < 0) {
        uart_putc('-');
        v = -v;
    }
    uart_put_uint(v / 100);
    uart_putc('.');
    if (v % 100 < 10) uart_putc('0');   // 5 -> 05
    uart_put_uint(v % 100);
}

int main(void) {
    clock_init();
    systick_init();
    uart_init();
    i2c_init();

    uart_puts("\nBME280 test\n");
    if (bme280_init(BME280_ADDR)) {
        uart_puts("BME280 not found\n");
        while (1);
    }

    while (1) {
        int32_t  temp;
        uint32_t hum;
        if (bme280_read(&temp, &hum) == 0) {
            uart_puts("temp: ");
            print_hundredths(temp);
            uart_puts(" C   humidity: ");
            print_hundredths(hum);
            uart_puts(" %\n");
        } else {
            uart_puts("read failed\n");
        }
        delay_ms(1000);
    }
}
