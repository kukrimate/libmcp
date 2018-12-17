/* MCP2210 configuration tool */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include "hid.h"
#include "mcp2210.h"
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

static int command_list(int argc, char **argv)
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

	printf("Found %d devices\n", device_count);
	for (size_t i = 0; i < device_count; ++i) {
		printf("[%d] %s\n", i + 1, hid_device_desc(devices[i]));
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
		 s.bitrate, s.idle_cs, s.active_cs, s.cs_to_data_delay,
		 s.data_to_cs_delay, s.data_delay, s.bytes_per_transaction,
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
		"\t.other_settings = 0x%02x,\n\t.nvram_chip_access_control = 0x%02x\n}\n",
		s.gpio_default, s.gpio_direction, s.other_settings,
		s.nvram_chip_access_control);
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
			" <spi_settings|chip_settings|key_parameters>\n",
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
	if (index > device_count || !index) {
		fprintf(stderr, "Invalid device number %d\n", index);
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
	} else {
		fprintf(stderr, "Unknown setting type %s\n", argv[optind + 1]);
		status = 1;
		goto done;
	}

done:
	for (size_t i = 0; i < device_count; ++i) {
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
		return command_list(argc - 1, argv + 1);
	} else if (!strcmp(argv[1], "get") || !strcmp(argv[1], "set")) {
		return command_get_set(argc - 1, argv + 1);
	} else {
		fprintf(stderr, "Unknown command %s\n", argv[1]);
		return 1;
	}

	hid_fini();
	return 0;
}
