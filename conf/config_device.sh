#!/bin/sh
# Shell script to easily configure a new device

# Add script directory to PATH
PATH=`dirname $0 | xargs realpath`:$PATH

# Change NVRAM (power-up default) settings
mcpconf set 1 -n spi_settings
mcpconf set 1 -n chip_settings

# Change the settings in RAM too to avoid re-plugging the device
mcpconf set 1 spi_settings
mcpconf set 1 chip_settings

# Changing these is usually not required
# mcpconf set 1 -n key_parameters
# mcpconf set 1 -n product_name
# mcpconf set 1 -n manufacturer_name
