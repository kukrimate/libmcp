NOTE: this project is unmaintained and has many known flaws. Please look
elsewhere if you want to communicate with an MCP2210 chip. Or even better use
a different IC as the MCP2210 is in many way the worst possible choice you can
make.

# libmcp2210
User space library/driver for the Microchip MCP2210 USB to SPI master bridge.
This repository includes both a library and a configuration tool for the MCP2210.

- the configuration tool is not meant as an end user applications, so
it does not include a user interface of any kind. The only way to change the
settings at the moment is by editing `config.h`.

- the library supports two different backends for USB access:
	- libusb for cross-platform compatibility
	- direct hidraw access on Linux

## copying
This project is distributed under the ISC license. Check `license.txt` for more
details.
