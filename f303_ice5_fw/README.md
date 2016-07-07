# f303_ice5_fw

This is MCU firmware source for the f303_ice5 board.

Building
--------

* Ensure you've got the Gnu ARM Embedded toolchain installed

* Just run
	make
	
Flashing
--------

There are several flashing options in the Makefile

* gdb_flash - flash using gdb and Black Magic Probe (BMP)
* openocd_flash - flash using OpenOCD and ST-Link V2
* openocd_v2.1_flash - flash using OpenOCD and ST-Link V2.1
* openocd_jlink_flash - flash via OpenOCD and Segger J-Link

Hook up your download pod and run

make [flash option from above list]

* dfu - flash using the STM32F303 USB DFU bootloader

Hold down the BOOT button on the f303_ice5 while inserting the USB connector.
The board should be recognized by Linux as a DFU device. Then run

make dfu

Running
-------

Plug into USB cable. This basic firmware doesn't provide USB functionality but
it does provide a serial 115k command processor on pins 5/6 of the expansion
connector. Type 'help' to get the latest list of supported commands. At this
point these commands are provided:

* help - this message
* spi_read (addr) - FPGA SPI read reg
* spi_write (addr) (data) - FPGA SPI write reg, data
* dir - directory of SD card root
* config_file (file) - Configure FPGA from file

The included bitmap.bin FPGA bitstream contains just two
registers accessible via the SPI bus thru the commands "spi_read" and
"spi_write" - a device ID (0XDEADBEEF) at register 0 and a timer divider
at register 1 which controls the speed of the 3 LED blinker.

Debugging
---------

1) in a separate window start up openocd as a GDB server
	openocd -f openocd.cfg

2) run gdb with your favorite UI
	ddd --debugger arm-none-eabi-gdb main.elf

3) connect to the server within gdb
	target extended-remote localhost:3333

4) Start debugging!
