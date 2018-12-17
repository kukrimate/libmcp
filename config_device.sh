#!/bin/sh
# Shell script to easily configure a new device
./mcp2210_conf set 1 -n spi_settings
./mcp2210_conf set 1 -n chip_settings
./mcp2210_conf set 1 spi_settings
./mcp2210_conf set 1 chip_settings
./mcp2210_conf set 1 -n key_parameters
./mcp2210_conf set 1 -n product_name
./mcp2210_conf set 1 -n manufacturer_name
