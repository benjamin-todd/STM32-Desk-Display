#include "stm32f4xx.h"
#include "rtc.h"
#include "systick.h"

// the RTC stores each number as BCD: one 4-bit digit per nibble, so 42 is stored as 0x42
static uint8_t to_bcd(uint8_t v)   { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
static uint8_t from_bcd(uint8_t b) { return (uint8_t)((b >> 4) * 10 + (b & 0x0F)); }

// two values unlock RTC register
static void rtc_unlock(void) {
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
}

static void rtc_lock(void) {
    RTC->WPR = 0xFF;
}

// the CPU reads the time through "shadow" copies that the RTC refreshes
// after reset/setting the time, wait for a fresh copy so we don't read stale values.
static void rtc_wait_sync(void) {
    rtc_unlock();
    RTC->ISR &= ~RTC_ISR_RSF;
    rtc_lock();
    while (!(RTC->ISR & RTC_ISR_RSF));
}

int rtc_init(void) {
    // unlock RTC
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;

    if (RCC->BDCR & RCC_BDCR_RTCEN) {
        rtc_wait_sync();
        return 0;
    }

    // start the crystal
    RCC->BDCR |= RCC_BDCR_LSEON;
    uint32_t start = millis();
    while (!(RCC->BDCR & RCC_BDCR_LSERDY)) {
        if (millis() - start > 3000) return -1;
    }

    // feed the RTC from the crystal (RTCSEL = 01) and turn the RTC on
    RCC->BDCR |= RCC_BDCR_RTCSEL_0;
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // the default dividers already turn 32,768 Hz into exactly 1 tick per second:
    // 32768 / (127 + 1) / (255 + 1) = 1
    rtc_set_time(0, 0, 0);
    return 1;
}

void rtc_set_time(uint8_t h, uint8_t m, uint8_t s) {
    rtc_unlock();
    RTC->ISR |= RTC_ISR_INIT;                 // pause the clock so we can change it
    while (!(RTC->ISR & RTC_ISR_INITF));      // wait until it's actually paused

    RTC->TR = ((uint32_t)to_bcd(h) << 16)     // hours in bits 16-21 (24-hour format)
            | ((uint32_t)to_bcd(m) << 8)      // minutes in bits 8-14
            |  (uint32_t)to_bcd(s);           // seconds in bits 0-6

    RTC->ISR &= ~RTC_ISR_INIT;                // let it run again
    rtc_lock();
    rtc_wait_sync();
}

void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s) {
    uint32_t tr = RTC->TR;
    (void)RTC->DR;   // reading TR freezes the date copy until DR is read, so read DR to unfreeze it

    *h = from_bcd((tr >> 16) & 0x3F);
    *m = from_bcd((tr >> 8) & 0x7F);
    *s = from_bcd(tr & 0x7F);
}
