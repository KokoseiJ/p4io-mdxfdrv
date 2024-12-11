#ifndef P4IO_MDXF_H
#define P4IO_MDXF_H

#include "aciodrv/device.h"
#include "acio/mdxf.h"
#include <string.h>

bool aciodrv_mdxf_start_auto_get(
	struct aciodrv_device_ctx *device, uint8_t node_id
);

bool aciodrv_mdxf_recv_poll(
	struct aciodrv_device_ctx *device, uint8_t node_id, struct ac_io_mdxf_poll_in *poll_in
);

#endif

