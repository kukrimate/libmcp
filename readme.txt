User space library/driver for the Microchip MCP2210 USB to SPI master bridge.
This repository includes both a library and a configuration tool for the MCP2210.

NOTE: the configuration tool is not meant as an end user applications, so
it does not include a user interface of any kind. The only way to change the
settings at the moment is by editing `config.h`.

NOTE2: at the moment this only works on Linux and requires libudev for searching
for the HID device, but it can be easily ported to any other platform where
GCC and GNU make is available by writing a replacement for `hid_linux.c` for
your platform conforming to the interface defined in `hid.h`.

==== COPYING ====
This project is distributed under the ISC license. Check `license.txt` for more
details.
