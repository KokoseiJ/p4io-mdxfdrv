#include "p4io.h"
#include <stdio.h>


int main() {
	struct p4io_data int_buffer;
	int rtnval, i, j, transferred, interval_usec;
	libusb_device_handle *dev_handle = NULL;
	struct libusb_endpoint_descriptor *bulk_out, *bulk_in, *intr_in;

	struct timespec t = {1, 0};

	libusb_init_context(NULL, NULL, 0);

	dev_handle = p4io_open(NULL); // to be freed
	if (dev_handle == NULL) {
		printf("[!] dev_handle not found.\n");
		goto CLEAN;
	}

	p4io_get_endpoints(dev_handle, &bulk_out, &bulk_in, &intr_in);
	interval_usec = usb_get_interval_usec(dev_handle, intr_in->bInterval);

	printf("[*] Starting input read\n");

	printf("(%d usec)\n", interval_usec);

	t.tv_sec = 0;
	t.tv_nsec = interval_usec * 1000;

	while (1) {
		p4io_poll(dev_handle, intr_in->bEndpointAddress, &int_buffer);

		/*
		for (i=0; i<4; i++) {
			for (j=0; j<8; j++) {
				// Printing from LSB so it's more intuitive when writing struct bitfield
				printf("%c", (int_buffer[i] >> j) & 1 ? '1' : '0');
	i		}
			printf(" | ");
		}
		*/
		printf(
			"1o %d 1u %d 1d %d 1l %d 1r %d 2o %d 2u %d 2d %d 2l %d 2r %d oc %d os %d ot %d\n",
			int_buffer.p1_ok,
			int_buffer.p1_up,
			int_buffer.p1_down,
			int_buffer.p1_left,
			int_buffer.p1_right,
			int_buffer.p2_ok,
			int_buffer.p2_up,
			int_buffer.p2_down,
			int_buffer.p2_left,
			int_buffer.p2_right,
			int_buffer.op_coin,
			int_buffer.op_service,
			int_buffer.op_test);
		nanosleep(&t, NULL);
	}

	libusb_close(dev_handle);

CLEAN:
	libusb_exit(NULL);
}

