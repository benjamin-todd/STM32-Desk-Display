#include "stm32f4xx.h"

int main(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;   // turn on GPIOC clock
    GPIOC->MODER |= (1 << (13 * 2));       // PC13 as output
    while (1) {
        GPIOC->ODR ^= (1 << 13);           // toggle LED
        for (volatile int i = 0; i < 2000000; i++);  // crude delay
    }
}
