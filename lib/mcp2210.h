/* talk to the MCP2210 chip */
#ifndef MCP2210_H
#define MCP2210_H

// The HID API is part of libmcp
#include "hid.h"

// VID and PID of the MCP2210
#define MCP2210_VID 0x04d8
#define MCP2210_PID 0x00de

////
// NOTE: every multi byte value is little endian
////

#ifdef LITTLE_ENDIAN
#define b16(x) x
#define b32(x) x
#else
#define b16(x) __builtin_bswap16(x)
#define b32(x) __builtin_bswap32(x)
#endif

// SPI settings
#define MCP2210_MAX_BITRATE 12000000
typedef struct mcp2210_spi_settings {
    uint32_t bitrate;
    uint16_t idle_cs;
    uint16_t active_cs;
    // CS assert to first data byte delay
    uint16_t cs_to_data_delay;
    // Last data byte to CS assert delay
    uint16_t data_to_cs_delay;
    // Delay between subsequent data bytes
    uint16_t data_delay;
    uint16_t bytes_per_transaction;
    uint8_t spi_mode;
} __attribute__((packed)) mcp2210_spi_settings_t;

// Chip settings
#define MCP2210_PIN_GPIO 0
#define MCP2210_PIN_CS 1
#define MCP2210_PIN_DEDICATED 2

#define MCP2210_BUS_RELEASE_DISABLE 1

typedef struct mcp2210_chip_settings {
    uint8_t  pins[9];
    uint16_t gpio_default;
    uint16_t gpio_direction;
    uint8_t  other_settings;
    uint8_t  nvram_lock;
    uint8_t  new_password[8];
} __attribute__((packed)) mcp2210_chip_settings_t;

// Key parameters
#define HOST_POWERED 0x80
#define SELF_POWERED 0x40

typedef struct mcp2210_key_parameters {
    uint16_t vid;
    uint16_t pid;
    uint8_t power_options;
    // Requested USB current in 2 mA
    uint8_t current_amount;
} __attribute__((packed)) mcp2210_key_parameters_t;

// Read SPI settings
int read_spi_settings(hid_handle_t *handle,
    mcp2210_spi_settings_t *spi_settings, bool nv);
// Write SPI settings
int write_spi_settings(hid_handle_t *handle,
    mcp2210_spi_settings_t *spi_settings, bool nv);

// Read chip settings
int read_chip_settings(hid_handle_t *handle,
    mcp2210_chip_settings_t *chip_settings, bool nv);
// Write chip settings
int write_chip_settings(hid_handle_t *handle,
    mcp2210_chip_settings_t *chip_settings, bool nv);

// Read key parameters
int read_key_parameters(hid_handle_t *handle,
    mcp2210_key_parameters_t *key_parameters);
// Write key parameters
int write_key_parameters(hid_handle_t *handle,
    mcp2210_key_parameters_t *key_parameters);

// Read product name
int read_product_name(hid_handle_t *handle,char *buffer, size_t buffer_len);
// Write product name
int write_product_name(hid_handle_t *handle, char *str);

// Read manufacturer name
int read_manufacturer_name(hid_handle_t *handle,
    char *buffer, size_t buffer_len);
// Write manufacturer name
int write_manufacturer_name(hid_handle_t *handle, char *str);

// SPI transfer
#define MCP2210_SPI_STATUS_FINISHED    0x10
#define MCP2210_SPI_STATUS_NO_DATA     0x20
#define MCP2210_SPI_STATUS_DATA_NEEDED 0x30

// Returns: one of the 3 listed status codes, -1 -> error
int mcp2210_spi_transfer(hid_handle_t *handle,
    void *send, size_t send_len,
    void *recv, size_t *recv_len);

#endif
