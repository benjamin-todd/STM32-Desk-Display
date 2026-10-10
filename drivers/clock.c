#include "stm32f4xx.h"
#include "clock.h"

void clock_init(void) {
    RCC->CR |= RCC_CR_HSEON; // turn on 25 MHz crystal
    while (!(RCC->CR & RCC_CR_HSERDY)); // wait until its stable

    // flash: 3 wait states, plus prefetch and caches to win back speed
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN
               | FLASH_ACR_ICEN | FLASH_ACR_DCEN;

    // APB1 bus = 100 / 2 = 50 MHz (clear the old setting, then set /2)
    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    // PLL: 25 MHz / M(25) x N(200) / P(2) = 100 MHz
    // P=2 is written as 0. Q isn't used but must be 2-15, so 4.
    RCC->PLLCFGR = (25 << RCC_PLLCFGR_PLLM_Pos)
                 | (200 << RCC_PLLCFGR_PLLN_Pos)
                 | (0 << RCC_PLLCFGR_PLLP_Pos)
                 | (4 << RCC_PLLCFGR_PLLQ_Pos)
                 | RCC_PLLCFGR_PLLSRC_HSE;

    // start the PLL, wait until it locks on
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // switch the CPU to the PLL, wait until the switch is confirmed
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
