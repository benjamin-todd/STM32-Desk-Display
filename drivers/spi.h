#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void spi_init(void);
void spi_write(const uint8_t *data, uint32_t len);
void spi_write_repeat16(uint16_t value, uint32_t count);

#endif
