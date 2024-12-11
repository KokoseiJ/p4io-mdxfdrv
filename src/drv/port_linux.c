#include "port_linux.h"

/**
 * Open a serial port for communication with a ACIO device.
 *
 * @param port Port the device is connected to (e.g. "COM1")
 * @param baud Baud rate for communication (e.g. 57600 for ICCA)
 * @return HANDLE of the port, NULL on error
 * @note This will open and setup the com port, only.
 */
HANDLE aciodrv_port_open(const char *port_path, int baud) {
	int handle;
	speed_t bauddata;
	struct termios termios_p;
	int tioctl;

	bauddata = baud_to_speed_t(baud);

	if (bauddata == B0) {
		return 0;
	}

	handle = open(port_path, O_RDWR, O_NOCTTY);

	if (handle < 0) {
		return 0;
	}

	if (tcflush(handle, TCIOFLUSH) < 0) {
		goto cleanup;
	}

	if (tcgetattr(handle, &termios_p) < 0) {
		goto cleanup;
	}
	if (ioctl(handle, TIOCMGET, &tioctl) < 0) {
		goto cleanup;
	}

	cfmakeraw(&termios_p);
	cfsetspeed(&termios_p, bauddata);
	handle |= TIOCM_DTR | TIOCM_RTS;

	if (tcsetattr(handle, TCSANOW, (const struct termios *) &termios_p) < 0) {
		goto cleanup;
	}
	if (ioctl(handle, TIOCMSET, &tioctl) < 0) {
		goto cleanup;
	}

	return handle;

cleanup:
	close(handle);
	
	return 0;
}

/**
 * Read data from the opened com port.
 *
 * @param port_fd HANDLE of opened serial port
 * @param bytes Pointer to an allocated buffer to read the data into.
 * @param nbytes Number of bytes to read. Has to be less or equal the allocated
 *        buffer size.
 * @return Number of bytes read on success or -1 on error.
 */
int aciodrv_port_read(HANDLE port_fd, void *bytes, int nbytes) {
	int read_bytes;

	if (port_fd == 0 || bytes == NULL) {
		return -1;
	}

	read_bytes = read(port_fd, bytes, nbytes);
	return read_bytes;
}

/**
 * Write data to the opened com port.
 *
 * @param port_fd HANDLE of opened serial port
 * @param bytes Pointer to an allocated buffer with data to write.
 * @param nbytes Number of bytes to write. Has to be equal or less the size
 *        of the allocated buffer.
 * @return Number of bytes written on success or -1 on error.
 */
int aciodrv_port_write(HANDLE port_fd, const void *bytes, int nbytes) {
	int written_bytes;

	if (port_fd == 0 || bytes == NULL) {
		return -1;
	}

	written_bytes = write(port_fd, bytes, nbytes);
	return written_bytes;
}

/**
 * Close the previously opened com port.
 *
 * @param port_fd HANDLE of opened serial port
 */
void aciodrv_port_close(HANDLE port_fd) {
	if (port_fd != 0) {
		close(port_fd);
	}
}

