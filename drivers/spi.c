#include "stm32f4xx.h"
#include "spi.h"

// SPI clock = 100 MHz / prescaler. BR bits: 0 = /2, 1 = /4, 2 = /8, 3 = /16 ...
// 0 (/2 = 50 MHz).
#define SPI_BR 0

void spi_init(void) {
    // clocks for GPIOA (pins) and SPI1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // PA5 (SCK) and PA7 (MOSI): alternate function AF5, fast output speed
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
    // turn it on
    SPI1->CR1 |= SPI_CR1_SPE;

    // clock for DMA2 (used by spi_write_dma_start)
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
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

// send the same 16-bit value many times (high byte first). Used to fill areas with one color.
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

// start sending len bytes (max 65535) and return immediately. Don't touch the buffer until spi_dma_wait().
void spi_write_dma_start(const uint8_t *data, uint16_t len) {
    DMA2_Stream3->CR &= ~DMA_SxCR_EN;             // make sure the stream is off
    while (DMA2_Stream3->CR & DMA_SxCR_EN);

    DMA2->LIFCR = DMA_LIFCR_CTCIF3 | DMA_LIFCR_CHTIF3 | DMA_LIFCR_CTEIF3
                | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CFEIF3;   // clear old status flags

    DMA2_Stream3->PAR  = (uint32_t)&SPI1->DR;     // destination: the SPI data register
    DMA2_Stream3->M0AR = (uint32_t)data;          // source: our buffer
    DMA2_Stream3->NDTR = len;                     // how many bytes

    DMA2_Stream3->CR = (3U << DMA_SxCR_CHSEL_Pos) // channel 3 = SPI1_TX
                     | DMA_SxCR_MINC              // step through the buffer (but not the SPI address)
                     | DMA_SxCR_DIR_0             // direction: memory -> peripheral
                     | DMA_SxCR_PL_1;             // high priority
                                                  // (byte-sized transfers are the default)
    DMA2_Stream3->CR |= DMA_SxCR_EN;              // go
    SPI1->CR2 |= SPI_CR2_TXDMAEN;                 // let SPI ask the DMA for the next byte
}

// wait until the DMA transfer, and the last byte on the wire, are finished
void spi_dma_wait(void) {
    if (!(SPI1->CR2 & SPI_CR2_TXDMAEN)) return;   // nothing in flight
    while (!(DMA2->LISR & DMA_LISR_TCIF3));       // DMA has handed over every byte
    spi_wait_done();                              // ...and SPI has shifted the last one out
    SPI1->CR2 &= ~SPI_CR2_TXDMAEN;
}
