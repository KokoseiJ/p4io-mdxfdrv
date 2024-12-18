#include <stdio.h>
#include "aciodrv/device.h"
#include "acio/mdxf.h"
#include "mdxf.h"

#include <pthread.h>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>


#define DDR_1P_LEFT 	BTN_0
#define DDR_1P_DOWN 	BTN_1
#define DDR_1P_UP 	BTN_2
#define DDR_1P_RIGHT 	BTN_3

#define DDR_2P_LEFT 	BTN_4
#define DDR_2P_DOWN	BTN_5
#define DDR_2P_UP	BTN_6
#define DDR_2P_RIGHT	BTN_7

struct p4io_pad {
	uint8_t p1_down : 4;
	uint8_t p1_up : 4;
	uint8_t p1_right: 4;
	uint8_t p1_left: 4;
	uint8_t p2_down: 4;
	uint8_t p2_up: 4;
	uint8_t p2_right: 4;
	uint8_t p2_left: 4;
};

struct p4io_pad pad_in;

void *run_poll(struct aciodrv_device_ctx *device);


int main(int argc, char *argv[]) {
	struct libevdev *dev;
	struct libevdev_uinput *uidev;
	struct aciodrv_device_ctx *device;
	uint8_t nodes;
	int i, err;
	pthread_t thread;
	struct input_absinfo absinfo = {0,};

	if (argc != 2) {
		printf("try again with 1 argument\n");
		return -1;
	}

	printf("Hello, World!\n");
	
	dev = libevdev_new();
	libevdev_set_name(dev, "P4IO DDR Pad");

	libevdev_enable_event_type(dev, EV_KEY);
	libevdev_enable_event_type(dev, EV_ABS);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_LEFT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_DOWN, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_UP, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_RIGHT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_LEFT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_DOWN, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_UP, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_RIGHT, NULL);
	libevdev_enable_event_code(dev, EV_ABS, ABS_X, &absinfo);

	err = libevdev_uinput_create_from_device(
		dev,
		LIBEVDEV_UINPUT_OPEN_MANAGED,
		&uidev
	);
	if (err != 0) {
		printf("Failed to create uinput device: %d\n", err);
		return -1;
	}

	printf("Opening connection on %s @ 115200\n", argv[1]);

	device = aciodrv_device_open_path(argv[1], 115200);
	if (!device) {
		printf("device is not there boowomp :sadblob:");
		return -1;
	}

	nodes = aciodrv_device_get_node_count(device);
	printf("found nodes: %hhd\n", nodes);

	for (i=0; i<nodes; i++) {
		char product[5] = {0,};
		aciodrv_device_get_node_product_ident(device, i, product);

		printf("  + %hhd: %s\n", i, product);
	}

	/* open path should've opened all nodes already */

	printf("press enter to start polling...");
	getchar();

	printf("Sending 0x0116 start polling...\n");
	if (!aciodrv_mdxf_start_auto_get(device, 0)) {
		printf("and mdxf didnt like that. boowomp");
		return -1;
	}

	err = pthread_create(&thread, NULL, (void * (*) (void *)) run_poll, device);
	if (err != 0) {
		printf("Thread creation failed: %d\n", err);
		return -1;
	}

	printf("And let er rip!\n\n");

	while (1) {
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_LEFT, pad_in.p1_left ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_DOWN, pad_in.p1_down ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_UP, pad_in.p1_up ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_RIGHT, pad_in.p1_right ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_LEFT, pad_in.p2_left ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_DOWN, pad_in.p2_down ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_UP, pad_in.p2_up ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_RIGHT, pad_in.p2_right ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
	}

	printf("they told me to stop wah >.<\n");
	return 0;
}


void *run_poll(struct aciodrv_device_ctx *device) {
	uint8_t i;
	uint8_t poll_in[3];
	uint8_t *pad_dest = (uint8_t *) &pad_in;

	while (1) {
		for (i=0; i<2; i++) {
			if (!aciodrv_mdxf_recv_poll(device, i, (struct ac_io_mdxf_poll_in *) &poll_in)) {
				printf("poll failed, stopping operation\n");
				return NULL;
			}
			memcpy(&pad_dest[2*i], poll_in, 2);
		}
	}

	return NULL;
}

