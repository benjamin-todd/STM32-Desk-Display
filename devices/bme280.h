#ifndef BME280_H
#define BME280_H

#include <stdint.h>

int bme280_init(uint8_t addr);
int bme280_read(int32_t *temp_c100, uint32_t *hum_pct100);

#endif
