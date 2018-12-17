#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include "hid.h"
#include "mcp2210.h"

// NVRAM
#define MCP2210_CMD_SET_NVRAM      0x60
#define MCP2210_CMD_GET_NVRAM      0x61
#define MCP2210_SUB_SPI_SETTINGS   0x10
#define MCP2210_SUB_CHIP_SETTINGS  0x20
#define MCP2210_SUB_KEY_PARAMETERS 0x30

// Volatile RAM
#define MCP2210_CMD_GET_SPI_SETTINGS 0x41
#define MCP2210_CMD_SET_SPI_SETTINGS 0x40

#define MCP2210_CMD_GET_CHIP_SETTINGS 0x20
#define MCP2210_CMD_SET_CHIP_SETTINGS 0x21

// Transfer SPI data
#define MCP2210_CMD_TRANSFER_SPI_DATA 0x42

typedef union mcp2210_data {
	mcp2210_spi_settings_t spi_settings;
	mcp2210_chip_settings_t chip_settings;
	mcp2210_key_parameters_t key_parameters;
	uint8_t raw[60];
} __attribute__((packed)) mcp2210_data_t;

typedef struct mcp2210_cmd {
	uint8_t hdr[4];
	mcp2210_data_t data;
} __attribute__((packed)) mcp2210_cmd_t;

typedef struct mcp2210_resp {
	uint8_t hdr[4];
	mcp2210_data_t data;
} __attribute__((packed)) mcp2210_resp_t;

static inline int do_usb_cmd(hid_handle_t *handle, mcp2210_cmd_t *cmd, mcp2210_resp_t *resp)
{
	if (-1 == hid_write(handle, cmd, sizeof(*cmd)))
		return -1;
	if (-1 == hid_read(handle, resp, sizeof(*resp)))
		return -1;
	return 0;
}

static inline void prepare_cmd(mcp2210_cmd_t *cmd, uint8_t hdr_0, uint8_t hdr_1)
{
	bzero(cmd, sizeof(*cmd));
	cmd->hdr[0] = hdr_0;
	cmd->hdr[1] = hdr_1;
}

// Read SPI settings
int read_spi_settings(hid_handle_t *handle, mcp2210_spi_settings_t *spi_settings, bool nv)
{
	mcp2210_cmd_t cmd;
	if (nv)
		prepare_cmd(&cmd, MCP2210_CMD_GET_NVRAM, MCP2210_SUB_SPI_SETTINGS);
	else
		prepare_cmd(&cmd, MCP2210_CMD_GET_SPI_SETTINGS, 0);

	mcp2210_resp_t resp;
	if (-1 == do_usb_cmd(handle, &cmd, &resp))
		return -1;

	if (resp.hdr[0] != cmd.hdr[0] || resp.hdr[1] != 0 ||
            (nv && resp.hdr[2] != MCP2210_SUB_SPI_SETTINGS)) {
		errno = EACCES;
		return -1;
	}

	*spi_settings = resp.data.spi_settings;
	return 0;
}
// Write NVRAM settings
int write_spi_settings(hid_handle_t *handle, mcp2210_spi_settings_t *spi_settings, bool nv)
{
	mcp2210_cmd_t cmd;
	if (nv)
		prepare_cmd(&cmd, MCP2210_CMD_SET_NVRAM, MCP2210_SUB_SPI_SETTINGS);
	else
		prepare_cmd(&cmd, MCP2210_CMD_SET_SPI_SETTINGS, 0);
	cmd.data.spi_settings = *spi_settings;
	
	mcp2210_resp_t resp;
	if (-1 == do_usb_cmd(handle, &cmd, &resp))
		return -1;

	if (resp.hdr[0] != cmd.hdr[0] || resp.hdr[1] != 0 || 
			nv && resp.hdr[2] != MCP2210_SUB_SPI_SETTINGS) {
		errno = EACCES;
		return -1;
	}

	return 0;
}
// Read chip settings
int read_chip_settings(hid_handle_t *handle, mcp2210_chip_settings_t *chip_settings, bool nv)
{
	mcp2210_cmd_t cmd;
	if (nv)
		prepare_cmd(&cmd, MCP2210_CMD_GET_NVRAM, MCP2210_SUB_CHIP_SETTINGS);
	else
		prepare_cmd(&cmd, MCP2210_CMD_GET_CHIP_SETTINGS, 0);

	mcp2210_resp_t resp;
	if (-1 == do_usb_cmd(handle, &cmd, &resp))
		return -1;

	if (resp.hdr[0] != cmd.hdr[0] || resp.hdr[1] != 0 || 
			nv && resp.hdr[2] != MCP2210_SUB_CHIP_SETTINGS) {
		errno = EACCES;
		return -1;
	}

	*chip_settings = resp.data.chip_settings;
	return 0;
}
// Write chip settings
int write_chip_settings(hid_handle_t *handle, mcp2210_chip_settings_t *chip_settings, bool nv)
{
	mcp2210_cmd_t cmd;
	if (nv)
		prepare_cmd(&cmd, MCP2210_CMD_SET_NVRAM, MCP2210_SUB_CHIP_SETTINGS);
	else
		prepare_cmd(&cmd, MCP2210_CMD_SET_CHIP_SETTINGS, 0);
	cmd.data.chip_settings = *chip_settings;

	mcp2210_resp_t resp;
	if (-1 == do_usb_cmd(handle, &cmd, &resp))
		return -1;

	if (resp.hdr[0] != cmd.hdr[0] || resp.hdr[1] != 0 || 
			nv && resp.hdr[2] != MCP2210_SUB_CHIP_SETTINGS) {
		errno = EACCES;
		return -1;
	}

	return 0;
}

// Write data to SPI
static int mcp2210_spi_write(hid_handle_t *handle, 
	void *data,
	size_t data_len,
	mcp2210_resp_t *resp)
{
	assert(data_len <= 60);

	mcp2210_cmd_t cmd;
    prepare_cmd(&cmd, MCP2210_CMD_TRANSFER_SPI_DATA, data_len);
	if (data_len)
		memcpy(cmd.data.raw, data, data_len);
	
	if (-1 == do_usb_cmd(handle, &cmd, resp))
		return -1;

	printf("%01x:%01x:%01x:%01x\n", 
		resp->hdr[0], resp->hdr[1], resp->hdr[2], resp->hdr[3]);

	if (resp->hdr[0] != cmd.hdr[0] || resp->hdr[1] != 0) {
		errno = EBUSY;
		return -1;
	}

	return 0;
}

int mcp2210_spi_transfer(hid_handle_t *handle, 
	void *ibuf, size_t ilen, 
	void *obuf, size_t *olen)
{
	mcp2210_resp_t resp;

	assert(ilen <= 60);

	if (-1 == mcp2210_spi_write(handle, ibuf, ilen, &resp))
		return -1;

	if (resp.hdr[3] != 0x20) {
		// this must be the case after the first write
		errno = EINVAL;
		return -1;
	}

	size_t clen = 0;
	for (;;) {
		if (clen > *olen) {
			errno = ENOMEM;
			return -1;
		}

		if (-1 == mcp2210_spi_write(handle, NULL, 0, &resp))
			return -1;

		if (resp.hdr[3] == 0x20) {
			errno = EINVAL;
			return -1;
		}

		clen += resp.hdr[2];
		memcpy(obuf + clen, resp.data.raw, resp.hdr[2]);

		switch (resp.hdr[3]) {
		case 0x10:
			goto done;
		case 0x30:
			continue;
		}
	}

done:
	*olen = clen;
	return 0;
}
