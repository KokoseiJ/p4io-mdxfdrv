#include "mdxf.h"

bool aciodrv_mdxf_start_auto_get(
	struct aciodrv_device_ctx *device, uint8_t node_id
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
	msg.cmd.nbytes = 2;
	/* Whatever libacio.dll sent with it */
	msg.cmd.raw[0] = 0x80;
	msg.cmd.raw[1] = 0x02;

	return aciodrv_send(device, &msg);
}


bool aciodrv_mdxf_recv_poll(
	struct aciodrv_device_ctx *device, uint8_t node_id, struct ac_io_mdxf_poll_in *poll_in
) {
	struct ac_io_message msg;
	
	msg.addr = node_id + 1;
	msg.cmd.code = ac_io_u16(AC_IO_CMD_MDXF_POLL);
	msg.cmd.nbytes = 3;

#ifdef MDXF_AUTOGET
	if (!aciodrv_recv(
#else
	if (!aciodrv_send_and_recv(
#endif
		device,
		&msg,
		9
	)) {
		return false;
	}

	memcpy(poll_in, &msg.cmd.raw, sizeof(struct ac_io_mdxf_poll_in));
	return true;
}

