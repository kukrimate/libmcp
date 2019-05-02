/* HID support code for libusb */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libusb.h>
#include "hid.h"

// HID handle
struct hid_handle {
	// libusb handle
	libusb_device_handle *libusb_handle;

	// IN and OUT endpoints
	uint8_t endpoint_in, endpoint_out;

	// User-friendly text description
	char text[33];
};

int hid_init()
{
	if (libusb_init(NULL) < 0) {
		return -1;
	}
	return 0;
}

ssize_t hid_find_devices(uint16_t vid, uint16_t pid, hid_handle_t **dest, size_t dest_len)
{
	libusb_device **devices;
	ssize_t device_count = libusb_get_device_list(NULL, &devices);
	if (device_count < 0)
		return -1;

	size_t dev_index = 0;

	for (size_t dev_i = 0; dev_i < device_count; ++dev_i) {
		struct libusb_device_descriptor dev_desc;

		if (libusb_get_device_descriptor(devices[dev_i], &dev_desc) < 0) {
			goto err;
		}

		if (dev_desc.idVendor == vid && dev_desc.idProduct == pid) {
			// Create a handle and open the device
			hid_handle_t *handle = malloc(sizeof(hid_handle_t));
			if (libusb_open(devices[dev_i], &handle->libusb_handle) < 0) {
				free(handle);
				goto err;
			}

			// Now check if it actually has a HID interface and claim it
			struct libusb_config_descriptor *conf_desc;
			if (libusb_get_config_descriptor(devices[dev_i], 0, &conf_desc) < 0) {
				libusb_close(handle->libusb_handle);
				free(handle);
				goto err;
			}

			// FIXME: This works with the MCP2210, but someone more familiar with
			// libusb and USB terminology should clean this up
			if (conf_desc->bNumInterfaces != 1 ||
					conf_desc->interface[0].num_altsetting != 1 ||
					conf_desc->interface[0].altsetting[0].bInterfaceClass
						!= LIBUSB_CLASS_HID) {
				libusb_free_config_descriptor(conf_desc);
				libusb_close(handle->libusb_handle);
				free(handle);
				goto err;
			}

			size_t in_cnt = 0, out_cnt = 0;

			// We need an IN and OUT interrupt endpoint
			for (size_t ep_i = 0; ep_i < conf_desc->interface[0]
					.altsetting[0].bNumEndpoints; ++ep_i) {
				const struct libusb_endpoint_descriptor *endpoint =
					&conf_desc->interface[0].altsetting[0].endpoint[ep_i];

				// Looking only for interrupt transfer endpoints
				if ((endpoint->bmAttributes | LIBUSB_TRANSFER_TYPE_MASK) |
						LIBUSB_TRANSFER_TYPE_INTERRUPT) {
					if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
						handle->endpoint_in = endpoint->bEndpointAddress;
						in_cnt++;
					} else {
						handle->endpoint_out = endpoint->bEndpointAddress;
						out_cnt++;
					}
				}
			}

			// It should only have one IN and one OUT endpoint
			if (in_cnt != 1 || out_cnt != 1) {
				libusb_free_config_descriptor(conf_desc);
				libusb_close(handle->libusb_handle);
				free(handle);
				goto err;
			}

			libusb_set_auto_detach_kernel_driver(handle->libusb_handle, 1);
			if (libusb_claim_interface(handle->libusb_handle, 0) < 0) {
				libusb_free_config_descriptor(conf_desc);
				libusb_close(handle->libusb_handle);
				free(handle);
				goto err;
			}

			// Generate a user friendly description of the device
			snprintf(handle->text, 33, "MCP2210 => VID: %04x, PID: %04x\n",
				dev_desc.idVendor, dev_desc.idProduct);

			// Add it to the array
			dest[dev_index++] = handle;

			// Cleanup
			libusb_free_config_descriptor(conf_desc);
		}
	}

	libusb_free_device_list(devices, 1);
	return dev_index;
err:
	libusb_free_device_list(devices, 1);
	return -1;
}

const char *hid_device_desc(hid_handle_t *handle)
{
	return handle->text;
}

ssize_t hid_write(hid_handle_t *handle, void *data, size_t data_len)
{
	int transferred;
	if (libusb_interrupt_transfer(handle->libusb_handle, handle->endpoint_out,
			data, data_len, &transferred, 0) < 0)
		return -1;
	if (transferred != data_len) {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

ssize_t hid_read(hid_handle_t *handle, void *buffer, size_t buffer_len)
{
	int read;
	if (libusb_interrupt_transfer(handle->libusb_handle, handle->endpoint_in,
			buffer, buffer_len, &read, 0) < 0)
		return -1;
	if (read != buffer_len) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

void hid_cleanup_device(hid_handle_t *handle)
{
	libusb_release_interface(handle->libusb_handle, 0);
	libusb_close(handle->libusb_handle);
	free(handle);
}

void hid_fini()
{
	libusb_exit(NULL);
}
