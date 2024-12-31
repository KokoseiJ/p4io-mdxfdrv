#include <stdio.h>
#include "aciodrv/device.h"
#include "acio/mdxf.h"
#include "mdxf.h"
#include "p4io.h"

#include <pthread.h>
#include <time.h>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

// TODO: Maybe split the device into 2 or 3 and map inputs to sensible buttons

#define DDR_1P_LEFT 		BTN_0
#define DDR_1P_DOWN 		BTN_1
#define DDR_1P_UP 		BTN_2
#define DDR_1P_RIGHT 		BTN_3

#define DDR_2P_LEFT 		BTN_4
#define DDR_2P_DOWN		BTN_5
#define DDR_2P_UP		BTN_6
#define DDR_2P_RIGHT		BTN_7

#define DDR_1P_MENU_LEFT 	BTN_8
#define DDR_1P_MENU_DOWN 	BTN_9
#define DDR_1P_MENU_UP 		BTN_TRIGGER
#define DDR_1P_MENU_RIGHT	BTN_THUMB
#define DDR_1P_MENU_OK 		BTN_THUMB2

#define DDR_2P_MENU_LEFT 	BTN_TOP
#define DDR_2P_MENU_DOWN 	BTN_TOP2
#define DDR_2P_MENU_UP 		BTN_PINKIE
#define DDR_2P_MENU_RIGHT 	BTN_BASE
#define DDR_2P_MENU_OK 		BTN_BASE2

#define DDR_OP_TEST 		BTN_BASE3
#define DDR_OP_SERVICE 		BTN_BASE4
#define DDR_OP_COIN 		BTN_BASE5

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
	
	libusb_device_handle *p4io_h = NULL;
	struct libusb_endpoint_descriptor *bulk_out, *bulk_in, *intr_in;
	struct p4io_data int_buffer;
	struct timespec t;

	uint8_t nodes;
	int i, err, interval;
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

	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_LEFT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_DOWN, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_UP, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_RIGHT, NULL);

	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_LEFT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_DOWN, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_UP, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_RIGHT, NULL);

	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_MENU_LEFT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_MENU_DOWN, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_MENU_UP, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_MENU_RIGHT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_MENU_OK, NULL);

	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_MENU_LEFT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_MENU_DOWN, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_MENU_UP, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_2P_MENU_RIGHT, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_1P_MENU_OK, NULL);

	libevdev_enable_event_code(dev, EV_KEY, DDR_OP_TEST, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_OP_SERVICE, NULL);
	libevdev_enable_event_code(dev, EV_KEY, DDR_OP_COIN, NULL);

	libevdev_enable_event_type(dev, EV_ABS);
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

	printf("Opening P4IO...\n");
	libusb_init_context(NULL, NULL, 0);
	p4io_h = p4io_open(NULL);
	if (p4io_h == NULL) {
		printf("[!] P4IO Not Found!\n");
		libusb_exit(NULL);
		return -1;
	}

	p4io_get_endpoints(p4io_h, &bulk_out, &bulk_in, &intr_in);
	interval = usb_get_interval_usec(p4io_h, intr_in->bInterval);

	printf("Input interval: %d\n", interval);
	t.tv_sec = 0;
	t.tv_nsec = interval * 1000;

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
		p4io_poll(p4io_h, intr_in->bEndpointAddress, &int_buffer);

		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_MENU_LEFT, int_buffer.p1_left);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_MENU_DOWN, int_buffer.p1_down);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_MENU_UP, int_buffer.p1_up);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_MENU_RIGHT, int_buffer.p1_right);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_MENU_OK, int_buffer.p1_ok);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_MENU_LEFT, int_buffer.p2_left);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_MENU_DOWN, int_buffer.p2_down);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_MENU_UP, int_buffer.p2_up);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_MENU_RIGHT, int_buffer.p2_right);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_MENU_OK, int_buffer.p2_ok);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_OP_TEST, int_buffer.op_test);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_OP_SERVICE, int_buffer.op_service);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_OP_COIN, int_buffer.op_coin);

		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_LEFT, pad_in.p1_left ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_DOWN, pad_in.p1_down ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_UP, pad_in.p1_up ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_1P_RIGHT, pad_in.p1_right ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_LEFT, pad_in.p2_left ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_DOWN, pad_in.p2_down ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_UP, pad_in.p2_up ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_KEY, DDR_2P_RIGHT, pad_in.p2_right ? 1 : 0);
		libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);

		nanosleep(&t, NULL);
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

