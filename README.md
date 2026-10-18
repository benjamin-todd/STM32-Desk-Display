# STM32-Desk-Display

<img width="4032" height="2268" alt="1F16F738-BD96-45B5-969F-A3AB489B0183_1_201_a" src="https://github.com/user-attachments/assets/412b68c1-4fb3-49ac-a8ae-9477dfc9ce5e" />

A desk clock which also shows temperature and humidity, built on a STM32F411 Black Pill and written bare-metal in C.

Time is displayed from the chip's real-time clock, temperature and humidity are read from a BME280 sensor, and information is displayed on a 240x240 ST7789 color screen.

Some highlights:
- Full-screen refill went from 147 ms -> 19 ms by raising the SPI clock from 6.25 MHz to 50 MHz, then added DMA.
- Full UI redraw went from 48 ms -> 19 ms using DMA with double buffering.
- Per-second clock update went from 19 ms -> 3 ms using partial redraw.
- Lowered the frame memory from 115 KB -> 19 KB using strip buffers instead of a full framebuffer.

Hardware I used:
- STM32F411 Black Pill
- ST7789 240×240 IPS screen (SPI)
- BME280 temperature / humidity sensor (I2C)
- CP2102 USB-serial adapter
- ST-Link V2

Build and flash:
Needs arm-none-eabhi-gcc, make, and openocd.
```
make            # build build/firmware.elf
make flash      # flash with ST-Link
screen /dev/tty.usbserial-0001 115200   # serial output (macOS)
```
Debugging: run openocd -f interface/stlink.cfg -f target/stm32f4x.cfg, then arm-none-eabi-gdb build/firmware.elf and target extended-remote :3333.

Short Video Demo:
<img width="1080" height="1920" alt="5600F48A-8000-48F0-A07A-FDCCE55D3712_1_102_o" src="https://github.com/user-attachments/assets/4bf2d1f0-3d2e-4793-a65a-d27b9b8e2039" />
