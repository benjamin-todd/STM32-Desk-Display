#include "bme280.h"
#include "i2c.h"
#include "systick.h"

// register addresses
#define REG_ID          0xD0
#define REG_CALIB_T     0x88   // 0x88-0x8D temperature calibration
#define REG_CALIB_H1    0xA1
#define REG_CALIB_H2    0xE1   // 0xE1-0xE7 rest of humidity calibration
#define REG_CTRL_HUM    0xF2
#define REG_CTRL_MEAS   0xF4
#define REG_DATA        0xF7   // 0xF7-0xFE raw pressure, temperature, humidity

#define BME280_CHIP_ID  0x60

static uint8_t dev_addr;

static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint8_t  dig_H1, dig_H3;
static int16_t  dig_H2, dig_H4, dig_H5;
static int8_t   dig_H6;

static int32_t t_fine;

int bme280_init(uint8_t addr) {
    uint8_t id;
    uint8_t c[7];

    dev_addr = addr;

    // tests if it is a BME280
    if (i2c_read_regs(dev_addr, REG_ID, &id, 1)) return -1;
    if (id != BME280_CHIP_ID) return -1;

    // read the calibration values (little-endian: low byte first)
    if (i2c_read_regs(dev_addr, REG_CALIB_T, c, 6)) return -1;
    dig_T1 = (uint16_t)(c[1] << 8 | c[0]);
    dig_T2 = (int16_t)(c[3] << 8 | c[2]);
    dig_T3 = (int16_t)(c[5] << 8 | c[4]);

    if (i2c_read_regs(dev_addr, REG_CALIB_H1, &dig_H1, 1)) return -1;

    if (i2c_read_regs(dev_addr, REG_CALIB_H2, c, 7)) return -1;
    dig_H2 = (int16_t)(c[1] << 8 | c[0]);
    dig_H3 = c[2];
    dig_H4 = (int16_t)(((int8_t)c[3] * 16) | (c[4] & 0x0F));   // 12 bits, split oddly
    dig_H5 = (int16_t)(((int8_t)c[5] * 16) | (c[4] >> 4));
    dig_H6 = (int8_t)c[6];

    // start measuring: humidity x1, temperature x1, pressure off, normal mode
    //    (ctrl_hum only takes effect after ctrl_meas is written, so this order matters)
    if (i2c_write_reg(dev_addr, REG_CTRL_HUM, 0x01)) return -1;
    if (i2c_write_reg(dev_addr, REG_CTRL_MEAS, (1 << 5) | (0 << 2) | 0x3)) return -1;

    delay_ms(10);   // lets the first measurement finish
    return 0;
}

static int32_t compensate_T(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12)
            * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

// returns humidity in %RH x 1024 (47445 = 46.333 %RH)
static uint32_t compensate_H(int32_t adc_H) {
    int32_t v = t_fine - ((int32_t)76800);
    v = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v))
           + ((int32_t)16384)) >> 15)
         * (((((((v * ((int32_t)dig_H6)) >> 10) * (((v * ((int32_t)dig_H3)) >> 11)
           + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v = (v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v = (v < 0 ? 0 : v);
    v = (v > 419430400 ? 419430400 : v);
    return (uint32_t)(v >> 12);
}

int bme280_read(int32_t *temp_c100, uint32_t *hum_pct100) {
    uint8_t d[8];

    // one read of all 8 data bytes so temperature and humidity come from the same measurement
    if (i2c_read_regs(dev_addr, REG_DATA, d, 8)) return -1;

    int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);   // 20 bits
    int32_t adc_H = ((int32_t)d[6] << 8) | d[7];                                   // 16 bits

    *temp_c100  = compensate_T(adc_T);                         // must run first: sets t_fine
    *hum_pct100 = (compensate_H(adc_H) * 100) >> 10;           // %RH x 1024 -> %RH x 100
    return 0;
}
