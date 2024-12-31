#ifndef P4IO_P4IO_H
#define P4IO_P4IO_H

#include <libusb.h>
#include <stdint.h>

#define P4IO_INT_SIZE 0x10

struct p4io_data {
	uint8_t p1_ok 		:1;
	uint8_t p1_up 		:1;
	uint8_t p1_down 	:1;
	uint8_t p1_left 	:1;
	uint8_t p1_right 	:1;
	uint8_t _pad1 		:3;
	uint8_t p2_ok 		:1;
	uint8_t p2_up 		:1;
	uint8_t p2_down 	:1;
	uint8_t p2_left 	:1;
	uint8_t p2_right 	:1;
	uint8_t _pad2;
	uint8_t op_coin 	:1;
	uint8_t op_service 	:1;
	uint8_t _pad3 		:2;
	uint8_t op_test 	:1;
};

libusb_device_handle *p4io_open(libusb_context *ctx);
int p4io_get_endpoints(libusb_device_handle *dev_handle,
			struct libusb_endpoint_descriptor **bulk_out,
			struct libusb_endpoint_descriptor **bulk_in,
			struct libusb_endpoint_descriptor **intr_in);
int usb_get_interval_usec(libusb_device_handle *dev_handle, uint8_t bInterval);
int p4io_poll(libusb_device_handle *dev_handle, uint8_t endpoint, struct p4io_data *buffer);

#endif

