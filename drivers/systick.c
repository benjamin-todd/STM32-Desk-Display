#include "stm32f4xx.h"
#include "systick.h"

static volatile uint32_t ms_ticks = 0;

// CPU runs this automatically every 1 ms
void SysTick_Handler(void) {
    ms_ticks++;
}

void systick_init(void) {
    SysTick->LOAD = 100000 - 1;   // 100 MHz / 1000 = 100,000 ticks per ms
    SysTick->VAL  = 0;            // start counting from the top
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk   // count at the full CPU speed
                  | SysTick_CTRL_TICKINT_Msk     // interrupt when it hits 0
                  | SysTick_CTRL_ENABLE_Msk;     // start it
}

void delay_ms(uint32_t ms) {
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms);
}

uint32_t millis(void) { return ms_ticks; }
