#include "stm32f4xx.h"
#include "button.h"
#include "systick.h"

#define DEBOUNCE_MS 200

static volatile int      pressed = 0;
static volatile uint32_t last_press_ms = 0;

void button_init(void) {
    // clocks for GPIOA and SYSCFG (SYSCFG decides which port drives each interrupt line)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // PA0 = input with a pull-up: reads 1 normally, 0 while the button is held down
    GPIOA->MODER &= ~(3U << (0 * 2));
    GPIOA->PUPDR &= ~(3U << (0 * 2));
    GPIOA->PUPDR |=  (1U << (0 * 2));

    // interrupt line 0 comes from port A (PA0, not PB0 or PC0)
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;

    // fire on the falling edge (1 -> 0, the moment it's pressed)
    EXTI->IMR  |= EXTI_IMR_MR0;               // don't mask (ignore) line 0
    EXTI->FTSR |= EXTI_FTSR_TR0;              // falling edge trigger

    // let the CPU take the interrupt
    NVIC_EnableIRQ(EXTI0_IRQn);
}

// the CPU jumps here automatically when PA0 goes low
void EXTI0_IRQHandler(void) {
    EXTI->PR = EXTI_PR_PR0;                   // clear the flag (write 1), or this runs forever

    // a button "bounces": one press makes several quick on/off blips, ignore within 200ms
    uint32_t now = millis();
    if (now - last_press_ms > DEBOUNCE_MS) {
        pressed = 1;
        last_press_ms = now;
    }
}

int button_was_pressed(void) {
    if (pressed) {
        pressed = 0;
        return 1;
    }
    return 0;
}
