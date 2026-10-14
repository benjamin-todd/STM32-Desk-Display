#include "stm32f4xx.h"
#include "spi.h"

// SPI clock = 100 MHz / prescaler. BR bits: 0 = /2, 1 = /4, 2 = /8, 3 = /16 ...
// start slow (/16 = 6.25 MHz), then try 0 (/2 = 50 MHz).
#define SPI_BR 0

void spi_init(void) {
    // clocks for GPIOA (pins) and SPI1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // PA5 (SCK) and PA7 (MOSI) alternate function AF5, fast output speed
    GPIOA->MODER   &= ~((3U << (5 * 2)) | (3U << (7 * 2)));
    GPIOA->MODER   |=  (2U << (5 * 2)) | (2U << (7 * 2));
    GPIOA->OSPEEDR |=  (3U << (5 * 2)) | (3U << (7 * 2));
    GPIOA->AFR[0]  &= ~((0xFU << (5 * 4)) | (0xFU << (7 * 4)));
    GPIOA->AFR[0]  |=  (5U << (5 * 4)) | (5U << (7 * 4));

    // controller mode, speed, SPI mode 3 (clock idles high, data read on the rising edge),
    // and software chip-select (we drive CS ourselves as a normal pin)
    SPI1->CR1 = SPI_CR1_MSTR
              | (SPI_BR << SPI_CR1_BR_Pos)
              | SPI_CR1_CPOL | SPI_CR1_CPHA
              | SPI_CR1_SSM | SPI_CR1_SSI;

    // turning it on
    SPI1->CR1 |= SPI_CR1_SPE;
}

// wait until the last bit has actually left the wire
static void spi_wait_done(void) {
    while (!(SPI1->SR & SPI_SR_TXE));
    while (SPI1->SR & SPI_SR_BSY);
}

void spi_write(const uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        while (!(SPI1->SR & SPI_SR_TXE));   // wait for room
        *(volatile uint8_t *)&SPI1->DR = data[i];
    }
    spi_wait_done();
}

// send the same 16-bit value many times (high byte first). used to fill areas with one color.
void spi_write_repeat16(uint16_t value, uint32_t count) {
    uint8_t hi = value >> 8;
    uint8_t lo = value & 0xFF;
    for (uint32_t i = 0; i < count; i++) {
        while (!(SPI1->SR & SPI_SR_TXE));
        *(volatile uint8_t *)&SPI1->DR = hi;
        while (!(SPI1->SR & SPI_SR_TXE));
        *(volatile uint8_t *)&SPI1->DR = lo;
    }
    spi_wait_done();
}
