#ifndef RTC_H
#define RTC_H

#include <stdint.h>

// start the real-time clock on the 32.768 kHz crystal.
// returns 0 if it was already running (time kept), 1 if it just started (time needs setting),
// -1 if the crystal never started.
int rtc_init(void);

void rtc_set_time(uint8_t h, uint8_t m, uint8_t s);
void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s);

#endif
