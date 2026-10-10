#include "stm32f4xx.h"
#include "clock.h"
#include "systick.h"

int main(void) {
    clock_init();     // 100 MHz
    systick_init();   // 1 ms tick

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER |= (1 << (13 * 2));
    while (1) {
        GPIOC->ODR ^= (1 << 13);
        delay_ms(500);
    }
}
