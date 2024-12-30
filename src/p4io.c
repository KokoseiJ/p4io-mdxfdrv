#include "p4io.h"
#include <string.h>

//#define VID 0x0b05
//#define PID 0x19b6
#define VID 0x1CCF
#define PID 0x8010

#define P4IO_USBCONFIG_INDEX 0

#define INTERRUPT_SIZE P4IO_INT_SIZE
#define BULK_SIZE 64
#define LIGHTS_SIZE 0x10

#define P4IO_CMD_LIGHTS 0x12

#define USB_FRAME 1000
#define USB_NANOFRAME USB_FRAME / 8

/*
LIBUSB_ENDPOINT_TRANSFER_TYPE_CONTROL
Control endpoint.

LIBUSB_ENDPOINT_TRANSFER_TYPE_ISOCHRONOUS
Isochronous endpoint.

LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK
Bulk endpoint.

LIBUSB_ENDPOINT_TRANSFER_TYPE_INTERRUPT
Interrupt endpoint.
*/


libusb_device_handle *p4io_open(libusb_context *ctx) {
	return libusb_open_device_with_vid_pid(ctx, VID, PID);
}


int p4io_get_endpoints(libusb_device_handle *dev_handle,
			struct libusb_endpoint_descriptor **bulk_out,
			struct libusb_endpoint_descriptor **bulk_in,
			struct libusb_endpoint_descriptor **intr_in) {
	int error;
	struct libusb_device *dev;
	struct libusb_config_descriptor *config;
	struct libusb_endpoint_descriptor *endpoint_array;

	dev = libusb_get_device(dev_handle);

	error = libusb_get_config_descriptor(
		dev, 
		P4IO_USBCONFIG_INDEX,
		&config
	);

	if (error != 0) {
		return error;
	}

	if (
		config->bNumInterfaces < 1 ||
		config->interface->num_altsetting < 1 ||
		config->interface->altsetting->bNumEndpoints != 3
	) {
		return 1;
	}

	endpoint_array = config->interface->altsetting->endpoint;

	*bulk_out = &endpoint_array[0];
	*bulk_in = &endpoint_array[1];
	*intr_in = &endpoint_array[2];

	return 0;
}


uint8_t p4io_logb(uint8_t x) {
	uint8_t i = 0;

	while (x >>= 1) i++;
	return i;
}


int usb_get_interval_usec(libusb_device_handle *dev_handle, uint8_t bInterval) {
	int usec;
	switch (libusb_get_device_speed(libusb_get_device(dev_handle))) {
	case LIBUSB_SPEED_SUPER_PLUS:
	case LIBUSB_SPEED_SUPER:
	case LIBUSB_SPEED_HIGH:
		usec = (1 << (bInterval - 1)) * USB_NANOFRAME;
		break;
	case LIBUSB_SPEED_FULL:
		usec = 1 << p4io_logb(bInterval);
		if (usec > 32) usec = 32;
		usec *= USB_FRAME;
		break;
	case LIBUSB_SPEED_LOW:
		if (bInterval < 16) usec = 8;
		else if (bInterval < 36) usec = 16;
		else usec = 32;
		usec *= USB_FRAME;
		break;
	default:
		return -1;
	}

	return usec;
}


int p4io_poll(libusb_device_handle *dev_handle, uint8_t endpoint, uint8_t *buffer) {
	int transferred;

	memset(buffer, 0, INTERRUPT_SIZE);
	return libusb_interrupt_transfer(dev_handle, endpoint, buffer, INTERRUPT_SIZE, &transferred, 0);
}

