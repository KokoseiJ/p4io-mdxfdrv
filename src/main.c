#include <stdio.h>
#include "aciodrv/device.h"
#include "acio/mdxf.h"
#include "mdxf.h"
#include "util/log.h"
#include "thread/thread.h"

uint8_t pad_in[4];

/* I know I'm not supposed to do this...
 * but it's poc so it should be fine right? (clueless)
 */
uint8_t should_stop = 0;


void run_poll(struct aciodrv_device_ctx *device);

int main(int argc, char *argv[]) {
	struct aciodrv_device_ctx *device;
	uint8_t nodes;
	int i, j;

	log_to_writer(log_writer_stdout, NULL);

	if (argc != 2) {
		printf("try again with 1 argument\n");
		return -1;
	}

	printf("Hello, World!\n");
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

	/* yes it has no mechanism to signal other thread
	 * unfortunately I dont care LMAOOOOO
	 */
	p4io_thread_t thread;
	p4io_thread_create(&thread, run_poll, device);

	printf("And let er rip!\n\n");

	while (!should_stop) {
		for (i=0; i<4; i++) {
			for (j=7; j>=0; j--) {
				printf("%c", (pad_in[i] >> j) & 1 ? '1' : '0');
			}
			printf(" | ");
		}
		printf("\n");
	}

	printf("they told me to stop wah >.<\n");
	return 0;
}


void run_poll(struct aciodrv_device_ctx *device) {
	uint8_t i;
	/* lord forgive me for i have sinned */
	uint8_t poll_in[3];

	while (!should_stop) {
		for (i=0; i<2; i++) {
			if (!aciodrv_mdxf_recv_poll(device, i, (struct ac_io_mdxf_poll_in *) &poll_in)) {
				printf("\n\n\n===============\nFailure!\n==========\n\n\n");
				/* this is so cursed */
				should_stop = 1;
				return;
			}
			memcpy(&pad_in[2*i], poll_in, 2);
		}
	}

	return;
}

