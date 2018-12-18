#!/bin/sh
# Shell script to easily configure a new device

# Change NVRAM (power-up default) settings 
./mcp2210_conf set 1 -n spi_settings
./mcp2210_conf set 1 -n chip_settings

# Change the settings in RAM too to avoid re-plugging the device
./mcp2210_conf set 1 spi_settings
./mcp2210_conf set 1 chip_settings

# Changing these is usually not required
#./mcp2210_conf set 1 -n key_parameters
#./mcp2210_conf set 1 -n product_name
#./mcp2210_conf set 1 -n manufacturer_name
