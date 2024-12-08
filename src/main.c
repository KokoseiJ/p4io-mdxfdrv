#include <stdio.h>
#include "aciodrv/device.h"
#include "acio/mdxf.h"
#include "mdxf.h"

int main() {
	struct aciodrv_device_ctx *device;
	uint8_t i, j, nodes;
	char k;

	printf("Hello, World!\n");
	printf("Opening connection on COM2 @ 115200\n");

	device = aciodrv_device_open_path("COM2", 115200);
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

	/* lord forgive me for i have sinned */
	uint8_t poll_in[3];

	printf("And let er rip!\n\n");

	while (1) {
		for (i=0; i<2; i++) {
			if(!aciodrv_mdxf_recv_poll(device, i, &poll_in)) {
				printf("uh oh lmao\n");
				return -1;
			}
			for (j=0; j<2; j++) {
				for (k=7; k>=0; k--) {
					printf("%c", ( poll_in[j] >> k ) & 1 ? '1' : '0');
				}
				printf(" ");
			}
			printf("|");
		}
		/* faster than carriage return loam */
		printf("\n");
	}

	return 0;
}

