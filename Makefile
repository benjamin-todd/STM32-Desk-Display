CC      = arm-none-eabi-gcc
CFLAGS  = -mcpu=cortex-m4 -mthumb -g -O0 -Wall -DSTM32F411xE -Ivendor/include -Idrivers
LDFLAGS = -mcpu=cortex-m4 -mthumb -T STM32F411CEUx_FLASH.ld --specs=nano.specs --specs=nosys.specs

SRCS = app/main.c vendor/src/system_stm32f4xx.c vendor/src/startup_stm32f411xe.s \
       drivers/clock.c drivers/systick.c drivers/uart.c

build/firmware.elf: $(SRCS)
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $@

.PHONY: flash
flash: build/firmware.elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/firmware.elf verify reset exit"
