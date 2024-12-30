#ifndef P4IO_P4IO_H
#define P4IO_P4IO_H

#include <libusb.h>
#include <stdint.h>

#define P4IO_INT_SIZE 0x10

libusb_device_handle *p4io_open(libusb_context *ctx);
int p4io_get_endpoints(libusb_device_handle *dev_handle,
			struct libusb_endpoint_descriptor **bulk_out,
			struct libusb_endpoint_descriptor **bulk_in,
			struct libusb_endpoint_descriptor **intr_in);
int usb_get_interval_usec(libusb_device_handle *dev_handle, uint8_t bInterval);
int p4io_poll(libusb_device_handle *dev_handle, uint8_t endpoint, uint8_t *buffer);

#endif

