#ifndef UI_H
#define UI_H

#include <stdint.h>

void ui_show_sensor(int32_t temp_c100, uint32_t hum_pct100);

// clock screen, only redraws the parts that changed since the last call
// full = 1 forces a full redraw (use it when switching to this screen)
// returns 0 if nothing was drawn, 1 for a partial redraw, 2 for a full redraw
int ui_show_clock(uint8_t h, uint8_t m, uint8_t s, int32_t temp_c100, uint32_t hum_pct100, int full);

#endif
