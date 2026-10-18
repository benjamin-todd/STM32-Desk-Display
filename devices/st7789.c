#include "stm32f4xx.h"
#include "st7789.h"
#include "spi.h"
#include "systick.h"

// control pins: CS = PA4, DC = PB0, RES = PB1
#define CS_LOW()   (GPIOA->BSRR = (1U << (4 + 16)))
#define CS_HIGH()  (GPIOA->BSRR = (1U << 4))
#define DC_CMD()   (GPIOB->BSRR = (1U << (0 + 16)))
#define DC_DATA()  (GPIOB->BSRR = (1U << 0))
#define RES_LOW()  (GPIOB->BSRR = (1U << (1 + 16)))
#define RES_HIGH() (GPIOB->BSRR = (1U << 1))

#define CMD_SWRESET 0x01
#define CMD_SLPOUT  0x11
#define CMD_NORON   0x13
#define CMD_INVON   0x21
#define CMD_DISPON  0x29
#define CMD_CASET   0x2A
#define CMD_RASET   0x2B
#define CMD_RAMWR   0x2C
#define CMD_MADCTL  0x36
#define CMD_COLMOD  0x3A

static void pins_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    GPIOA->MODER &= ~(3U << (4 * 2));          // PA4 = output
    GPIOA->MODER |=  (1U << (4 * 2));
    GPIOB->MODER &= ~((3U << (0 * 2)) | (3U << (1 * 2)));   // PB0, PB1 = outputs
    GPIOB->MODER |=  (1U << (0 * 2)) | (1U << (1 * 2));

    CS_HIGH();
    RES_HIGH();
}

static void write_cmd(uint8_t cmd) {
    DC_CMD();
    CS_LOW();
    spi_write(&cmd, 1);
    CS_HIGH();
}

static void write_data(const uint8_t *data, uint32_t len) {
    DC_DATA();
    CS_LOW();
    spi_write(data, len);
    CS_HIGH();
}

static void write_cmd_byte(uint8_t cmd, uint8_t value) {
    write_cmd(cmd);
    write_data(&value, 1);
}

void st7789_init(void) {
    pins_init();
    spi_init();

    // hardware reset: hold RES low, then let it go and give the screen time to wake
    RES_LOW();
    delay_ms(10);
    RES_HIGH();
    delay_ms(120);

    write_cmd(CMD_SWRESET);  delay_ms(150);  // software reset
    write_cmd(CMD_SLPOUT);   delay_ms(120);  // wake from sleep (datasheet: wait 120 ms)
    write_cmd_byte(CMD_COLMOD, 0x55);        // 16-bit color (RGB565)
    write_cmd_byte(CMD_MADCTL, 0x00);        // normal orientation, RGB order
    write_cmd(CMD_INVON);                    // these panels need inversion on for correct colors
    write_cmd(CMD_NORON);                    // normal display mode
    write_cmd(CMD_DISPON);   delay_ms(10);   // display on
}

// tell the screen which rectangle the next pixels go into
static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t buf[4];

    write_cmd(CMD_CASET);                    // columns x0..x1
    buf[0] = x0 >> 8; buf[1] = x0 & 0xFF; buf[2] = x1 >> 8; buf[3] = x1 & 0xFF;
    write_data(buf, 4);

    write_cmd(CMD_RASET);                    // rows y0..y1
    buf[0] = y0 >> 8; buf[1] = y0 & 0xFF; buf[2] = y1 >> 8; buf[3] = y1 & 0xFF;
    write_data(buf, 4);

    write_cmd(CMD_RAMWR);                    // pixels follow
}

void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (w == 0 || h == 0) return;
    set_window(x, y, x + w - 1, y + h - 1);
    DC_DATA();
    CS_LOW();
    spi_write_repeat16(color, (uint32_t)w * h);
    CS_HIGH();
}

void st7789_fill_screen(uint16_t color) {
    st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

// send a block of pixels that's already in memory in screen order
// high byte first, which is why gfx.c stores its colors byte-swapped
void st7789_draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *pixels) {
    set_window(x, y, x + w - 1, y + h - 1);
    DC_DATA();
    CS_LOW();
    spi_write((const uint8_t *)pixels, (uint32_t)w * h * 2);
    CS_HIGH();
}

// same as st7789_draw_pixels, but returns as soon as the DMA has started
// call st7789_dma_wait() before sending anything else or changing the buffer
void st7789_draw_pixels_dma(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *pixels) {
    set_window(x, y, x + w - 1, y + h - 1);
    DC_DATA();
    CS_LOW();
    spi_write_dma_start((const uint8_t *)pixels, (uint16_t)((uint32_t)w * h * 2));
}

void st7789_dma_wait(void) {
    spi_dma_wait();
    CS_HIGH();
}
