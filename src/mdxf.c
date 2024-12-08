#include "aciodrv/device.h"
#include "aciodrv/mdxf.h"

bool aciodrv_mdxf_start_auto_get(
	struct aciodrv_device_ctx *device, uint8_t node_id, uint8_t node_count
) {
	struct ac_io_message msg;
	char *dataptr;

	/* PANB states only the first node receives this command.
	 * My observation on serial dump matches that so...
	 * I'll assume that's the case here as well?
	 */
	if (node_id != 0) {
		return true;
	}
    
	msg.addr = node_id + 1;
	msg.cmd.code = ac_io_u16(AC_IO_CMD_MDXF_AUTO_GET_START);
	msg.cmd.nbytes = 3;
	/* Whatever libacio.dll sent with it */
	msg.cmd.raw[0] = 0x80;
	msg.cmd.raw[1] = 0x02;
	msg.cmd.raw[2] = 0;

	return aciodrv_send(device, &msg)
}


bool aciodrv_mdxf_recv_poll(
	struct aciodrv_device_ctx *device, struct ac_io_mdxf_poll_in *poll_in
) {
	struct ac_io_message msg;
	
	msg.cmd.code = ac_io_u16(AC_IO_CMD_MDXF_POLL);
	msg.cmd.nbytes = 3;

	if (!aciodrv_recv(
		device,
		&msg,
		9
	)) {
		return false;
	}

	memcpy(poll_in, &msg.cmd.raw, sizeof(struct ac_io_mdxf_poll_in));
}

