/* MCP2210 configuration tool */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <mcp2210.h>
#include "config.h"

static ssize_t get_device_list(hid_handle_t ***devices)
{
	ssize_t device_count;
	for (size_t i = 10;; i *= 2) {
		*devices = malloc(i);
		if (!*devices) abort();
		device_count = hid_find_devices(MCP2210_VID, MCP2210_PID, *devices, 10);
		if (-1 == device_count) {
			if (errno == ENOMEM) {
				free(*devices);
				continue;
			}
			return -1;
		}
		break;
	}
	return device_count;
}

static int command_list()
{
	hid_handle_t **devices;
	ssize_t device_count = get_device_list(&devices);
	if (-1 == device_count) {
		perror("Failed to search for HID devices");
		return 1;
	}

	if (!device_count) {
		printf("Found no devices\n");
		goto done;
	}

	printf("Found %ld devices\n", device_count);
	for (size_t i = 0; i < (size_t) device_count; ++i) {
		printf("[%ld] %s\n", i + 1, hid_device_desc(devices[i]));
		hid_cleanup_device(devices[i]);
	}

done:
	free(devices);
	return 0;
}

static void print_spi_settings(mcp2210_spi_settings_t s)
{
	printf("{\n\t.bitrate = %d\n\t.idle_cs = 0x%04x\n"
		"\t.active_cs = 0x%04x\n\t.cs_to_data_delay = 0x%04x\n"
		"\t.data_to_cs_delay = 0x%04x\n\t.data_delay = 0x%04x\n"
		"\t.bytes_per_transaction = 0x%04x\n"
		"\t.spi_mode = %d\n}\n",
		 b32(s.bitrate), b16(s.idle_cs), b16(s.active_cs), b16(s.cs_to_data_delay),
		 b16(s.data_to_cs_delay), b16(s.data_delay), b16(s.bytes_per_transaction),
		 s.spi_mode);
}

static void print_chip_settings(mcp2210_chip_settings_t s)
{
	printf("{\n\t.pins = \n\t{\n");
	for (size_t i = 0; i < 9; ++i) {
		uint8_t pin = s.pins[i];
		switch (pin) {
		case MCP2210_PIN_GPIO:
			printf("\t\tMCP2210_PIN_GPIO,\n");
			break;
		case MCP2210_PIN_CS:
			printf("\t\tMCP2210_PIN_CS,\n");
			break;
		case MCP2210_PIN_DEDICATED:
			printf("\t\tMCP2210_PIN_DEDICATED,\n");
			break;
		}
	}
	printf("\t},\n\t.gpio_default = 0x%04x,\n\t.gpio_direction = 0x%04x,\n"
		"\t.other_settings = 0x%02x,\n\t.nvram_lock = 0x%02x\n}\n",
		b16(s.gpio_default), b16(s.gpio_direction), s.other_settings,
		s.nvram_lock);
}

static void print_key_parameters(mcp2210_key_parameters_t s)
{
	printf("{\n\t.vid = 0x%04x\n\t.pid = 0x%04x\n"
		"\t.power_options = 0x%02x\n\t.current_amount = %d mA\n}\n",
		 b16(s.vid), b16(s.pid), s.power_options, s.current_amount * 2);
}

static int command_get_set(int argc, char **argv)
{
	bool get;
	if (!strcmp(argv[0], "get")) {
		get = true;
	} else if (!strcmp(argv[0], "set")) {
		get = false;
	} else {
		fprintf(stderr, "Internal error, "
			"command function called with incorrect command %s\n", argv[0]);
		return 1;
	}

	int opt;
	bool is_nvram = false;
	while ((opt = getopt(argc, argv, "n")) != -1) {
		switch ((char) opt) {
		case 'n':
			is_nvram = true;
			break;
		default:
			return 1;
		}
	}

	if (optind >= argc - 1) {
		fprintf(stderr, "Usage: %s <chip_number>"
			" <product_name|manufacturer_name|spi_settings|"
			"chip_settings|key_parameters>\n",
			argv[0]);
		return 1;
	}

	hid_handle_t **devices;
	ssize_t device_count = get_device_list(&devices);
	if (-1 == device_count) {
		perror("Failed to search for HID devices");
		return 1;
	}

	int status = 0;

	size_t index = strtol(argv[optind], NULL, 10);
	if (index > (size_t) device_count || !index) {
		fprintf(stderr, "Invalid device number %ld\n", index);
		status = 1;
		goto done;
	}

	if (!strcmp(argv[optind + 1], "spi_settings")) {
		mcp2210_spi_settings_t spi_settings;

		if (!get) {
			printf("Writing SPI settings...\n");
			if (-1 == write_spi_settings(devices[index - 1], &config_spi_settings, is_nvram)) {
				perror("Failed to write SPI settings");
				status = 1;
				goto done;
			}
			printf("Done\n");
		}

		printf("Reading SPI settings...\n");
		if (-1 == read_spi_settings(devices[index - 1], &spi_settings, is_nvram)) {
			perror("Failed to read SPI settings");
			status = 1;
			goto done;
		}
		print_spi_settings(spi_settings);
	} else if (!strcmp(argv[optind + 1], "chip_settings")) {
		mcp2210_chip_settings_t chip_settings;

		if (!get) {
			printf("Writing chip settings...\n");
			if (-1 == write_chip_settings(devices[index - 1], &config_chip_settings, is_nvram)) {
				perror("Failed to write chip settings");
				status = 1;
				goto done;
			}
			printf("Done\n");
		}

		printf("Reading chip settings...\n");
		if (-1 == read_chip_settings(devices[index - 1], &chip_settings, is_nvram)) {
			perror("Failed to read chip settings");
			status = 1;
			goto done;
		}
		print_chip_settings(chip_settings);
	} else if (!strcmp(argv[optind + 1], "product_name")) {
		if (!is_nvram && !get)
			printf("NOTE: The product name is always set in NVRAM.\n");

		if (!get) {
			printf("Writing product name...\n");
			if (-1 == write_product_name(devices[index - 1], CONFIG_PRODUCT_NAME)) {
				perror("Failed to write product name");
				status = 1;
				goto done;
			}
		}

		printf("Reading product name...\n");
		char *buffer = malloc(100);
		if (-1 == read_product_name(devices[index - 1], buffer, 100)) {
			free(buffer);
			perror("Failed to read product name");
			status = 1;
			goto done;
		}
		printf("Product name: %s\n", buffer);
		free(buffer);
	} else if (!strcmp(argv[optind + 1], "manufacturer_name")) {
		if (!is_nvram && !get)
			printf("NOTE: The manufacturer name is always set in NVRAM.\n");

		if (!get) {
			printf("Writing manufacturer name...\n");
			if (-1 == write_manufacturer_name(devices[index - 1], CONFIG_MANUFACTURER_NAME)) {
				perror("Failed to write manufacturer name");
				status = 1;
				goto done;
			}
		}

		printf("Reading manufacturer name...\n");
		char *buffer = malloc(100);
		if (-1 == read_manufacturer_name(devices[index - 1], buffer, 100)) {
			free(buffer);
			perror("Failed to read manufacturer name");
			status = 1;
			goto done;
		}
		printf("Manufacturer name: %s\n", buffer);
		free(buffer);
	} else if (!strcmp(argv[optind + 1], "key_parameters")) {
		if (!is_nvram && !get)
			printf("NOTE: The key parameters are always set in NVRAM.\n");

		if (!get) {
			printf("Writing key parameters...\n");
			if (-1 == write_key_parameters(devices[index - 1], &config_key_parameters)) {
				perror("Failed to write key parameters");
				status = 1;
				goto done;
			}
		}

		printf("Reading key parameters...\n");
		mcp2210_key_parameters_t key_parameters;
		if (-1 == read_key_parameters(devices[index - 1], &key_parameters)) {
			perror("Failed to read key parameters");
			status = 1;
			goto done;
		}
		print_key_parameters(key_parameters);
	} else {
		fprintf(stderr, "Unknown setting type %s\n", argv[optind + 1]);
		status = 1;
		goto done;
	}

done:
	for (size_t i = 0; i < (size_t) device_count; ++i) {
		hid_cleanup_device(devices[i]);
	}
	free(devices);
	return status;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s command\n", argv[0]);
		return 1;
	}

	// Initialize HID library
	if (-1 == hid_init()) {
		perror("Failed to initialize HID module");
		return 1;
	}

	// Commands
	if (!strcmp(argv[1], "list")) {
		return command_list();
	} else if (!strcmp(argv[1], "get") || !strcmp(argv[1], "set")) {
		return command_get_set(argc - 1, argv + 1);
	} else {
		fprintf(stderr, "Unknown command %s\n", argv[1]);
		return 1;
	}

	hid_fini();
	return 0;
}
