/* set chip configuration here for the config tool */
#ifndef CONFIG_H
#define CONFIG_H

// SPI settings
mcp2210_spi_settings_t config_spi_settings =
{
	.bitrate               = 1464, /* this is the minimum bitrate */
	.idle_cs               = 0x1ff,
	.active_cs             = 0,
	.cs_to_data_delay      = 100,
	.data_to_cs_delay      = 100,
	.data_delay            = 100,
	.bytes_per_transaction = 64,
	.spi_mode              = 0
};

// Chip settings
mcp2210_chip_settings_t config_chip_settings =
{
	.pins = { MCP2210_PIN_CS,
			MCP2210_PIN_GPIO,
			MCP2210_PIN_GPIO,
			MCP2210_PIN_GPIO,
			MCP2210_PIN_GPIO,
			MCP2210_PIN_GPIO,
			MCP2210_PIN_GPIO,
			MCP2210_PIN_GPIO,
			MCP2210_PIN_GPIO },
	.gpio_default   = 0,
	.gpio_direction = 0,
	.other_settings = 0,
	.nvram_lock     = 0,
	.new_password   = { 0, 0, 0, 0, 0, 0, 0, 0 }
};

// Key parameters
mcp2210_key_parameters_t config_key_parameters =
{
	.vid            = MCP2210_VID,
	.pid            = MCP2210_PID,
	//.power_options = SELF_POWERED,
	.power_options  = HOST_POWERED,
	// Required current in 2 mA
	.current_amount = 250 // default is 100 mA
};

// Product name
//#define CONFIG_PRODUCT_NAME "MCP2210 USB to SPI Master"
#define CONFIG_PRODUCT_NAME "USB SPI Flasher"
// Manufacturer name
//#define CONFIG_MANUFACTURER_NAME "Microchip Technology Inc."
#define CONFIG_MANUFACTURER_NAME "Mate Kukri"

#endif
