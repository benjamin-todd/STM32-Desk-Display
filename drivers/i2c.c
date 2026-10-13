#include "stm32f4xx.h"
#include "i2c.h"

#define I2C_TIMEOUT 100000   // loop count before giving up (a few ms)

void i2c_init(void) {
    // clocks for GPIOB and I2C1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // PB6 (SCL) and PB7 (SDA) alternate function, open-drain, pull-up, AF4
    GPIOB->MODER   &= ~((3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOB->MODER   |=  (2U << (6 * 2)) | (2U << (7 * 2));
    GPIOB->OTYPER  |=  (1U << 6) | (1U << 7);
    GPIOB->PUPDR   &= ~((3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOB->PUPDR   |=  (1U << (6 * 2)) | (1U << (7 * 2));
    GPIOB->AFR[0]  &= ~((0xFU << (6 * 4)) | (0xFU << (7 * 4)));
    GPIOB->AFR[0]  |=  (4U << (6 * 4)) | (4U << (7 * 4));

    // reset the I2C block
    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;

    // timing for 100 kHz "standard mode" with a 50 MHz APB1 clock
    I2C1->CR2   = 50;    // tell it the input clock is 50 MHz
    I2C1->CCR   = 250;   // 50 MHz / (2 x 100 kHz) = 250
    I2C1->TRISE = 51;    // max rise time 1000 ns -> 50 + 1

    // turn it on
    I2C1->CR1 = I2C_CR1_PE;
}

// wait for a status flag. returns -1 if the other chip said no or nothing happened.
static int wait_flag(uint32_t flag) {
    uint32_t t = I2C_TIMEOUT;
    while (!(I2C1->SR1 & flag)) {
        if (I2C1->SR1 & I2C_SR1_AF) {          // no acknowledge from the device
            I2C1->SR1 &= ~I2C_SR1_AF;
            return -1;
        }
        if (--t == 0) {
            return -1;
        }
    }
    return 0;
}

static int fail(void) {
    I2C1->CR1 |= I2C_CR1_STOP;   // release the bus
    return -1;
}

static void clear_addr(void) {
    (void)I2C1->SR1;   // read SR1 then SR2 to clear ADDR
    (void)I2C1->SR2;
}

// send START + the 7-bit address with the read/write bit
static int start(uint8_t addr_rw) {
    I2C1->CR1 |= I2C_CR1_START;
    if (wait_flag(I2C_SR1_SB)) return -1;
    I2C1->DR = addr_rw;
    if (wait_flag(I2C_SR1_ADDR)) return -1;
    return 0;
}

int i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t value) {
    if (start(addr << 1)) return fail();       // address + write
    clear_addr();

    if (wait_flag(I2C_SR1_TXE)) return fail();
    I2C1->DR = reg;                            // which register
    if (wait_flag(I2C_SR1_TXE)) return fail();
    I2C1->DR = value;                          // what to put in it
    if (wait_flag(I2C_SR1_BTF)) return fail();

    I2C1->CR1 |= I2C_CR1_STOP;
    return 0;
}

int i2c_read_regs(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t n) {
    // tell the device which register we want to start reading from
    if (start(addr << 1)) return fail();       // address + write
    clear_addr();
    if (wait_flag(I2C_SR1_TXE)) return fail();
    I2C1->DR = reg;
    if (wait_flag(I2C_SR1_BTF)) return fail();

    // start again, this time in read mode, and collect n bytes
    if (start((addr << 1) | 1)) return fail(); // address + read

    if (n == 1) {
        I2C1->CR1 &= ~I2C_CR1_ACK;             // stop after this byte
        clear_addr();
        I2C1->CR1 |= I2C_CR1_STOP;
        if (wait_flag(I2C_SR1_RXNE)) return fail();
        buf[0] = I2C1->DR;
        return 0;
    }

    I2C1->CR1 |= I2C_CR1_ACK;                  // keep going after each byte
    clear_addr();
    for (uint16_t i = 0; i < n; i++) {
        if (i == n - 1) {
            I2C1->CR1 &= ~I2C_CR1_ACK;         // last byte
            I2C1->CR1 |= I2C_CR1_STOP;
        }
        if (wait_flag(I2C_SR1_RXNE)) return fail();
        buf[i] = I2C1->DR;
    }
    return 0;
}
