#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init(void);
int  i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t value);
int  i2c_read_regs(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t n);

#endif
