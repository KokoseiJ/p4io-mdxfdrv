#define LOG_MODULE "aciodrv-port-linux"

#include "port_linux.h"
#include "util/log.h"
#include <errno.h>
#include <string.h>

#define log_errno(x) log_warning("%s() failed: %d(%s)", x, errno, strerror(errno))

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

	log_info("Opening ACIO on %s at %d baud", port_path, baud);

	/* Convert int to speed_t value for termios */
	bauddata = baud_to_speed_t(baud);

	/* WINE graciously rounds this to known value, unfortunately I don't care */
	if (bauddata == B0) {
		log_warning("Invalid baudrate %d", baud);
		return 0;
	}

	/* CreateFile equivalent.
	 * OPEN_EXISTING flag isn't feasible on Linux nor needed
	 * FILE_FLAG_WRITE_THROUGH equivalent is set with termios
	 */
	handle = open(port_path, O_RDWR, O_NOCTTY);
	if (handle < 0) {
		log_errno("open");
		return 0;
	}

	log_info("fd: %d", handle);

	/* SetCommMask and SetupComm are WinAPI specific */

	/* PurgeComm equivalent */
	if (tcflush(handle, TCIOFLUSH) < 0) {
		log_errno("tcflush");
		goto cleanup;
	}

	/* SetCommTimeouts is used to make read non-blocking,
	 * the mechanism is implemented with select() call instead
	 */

	if (tcgetattr(handle, &termios_p) < 0) {
		log_errno("tcgetattr");
		goto cleanup;
	}
	if (ioctl(handle, TIOCMGET, &tioctl) < 0) {
		log_errno("ioctl TIOCMGET");
		goto cleanup;
	}

	/* DCB part equivalent.
	 * cfmakeraw is used to ensure no extra processing is added,
	 * which seems to be the case for WinAPI port implementation.
	 */
	cfmakeraw(&termios_p);
	
	/* dcb.BaudRate */
	cfsetspeed(&termios_p, bauddata);

	/* dcb.fParity = FALSE */
	termios_p.c_iflag |= IGNPAR;
	termios_p.c_iflag &= ~INPCK;
	/* dcb.fDtrControl = DTR_CONTROL_ENABLE
	 * Pull DTR line up instantly, no handshake */
	tioctl |= TIOCM_DTR;
	/* dcb.fOutX = FALSE; dcb.fInX = FALSE */
	termios_p.c_iflag &= ~(IXON | IXOFF);
	/* dcb.fErrorChar = FALSE */
	termios_p.c_iflag &= ~PARMRK;
	/* dcb.fRtsControl = RTS_CONTROL_ENABLE; */
	tioctl |= TIOCM_RTS;
	termios_p.c_cflag &= ~CRTSCTS;
	/* dcb.ByteSize = 8 */
	termios_p.c_iflag &= ~ISTRIP;
	termios_p.c_cflag &= ~CSIZE;
	termios_p.c_cflag |= CS8;
	/* dcb.Parity = NOPARITY */
	termios_p.c_cflag &= ~PARENB;
	/* dcb.StopBits = ONESTOPBIT */
	termios_p.c_cflag &= ~CSTOPB;
	/* Xon/Xoff stuff are skipped since Xon/Xoff is disabled */

	if (tcsetattr(handle, TCSANOW, (const struct termios *) &termios_p) < 0) {
		log_errno("tcsetattr");
		goto cleanup;
	}
	if (ioctl(handle, TIOCMSET, &tioctl) < 0) {
		log_errno("ioctl TIOCMSET");
		goto cleanup;
	}

	log_info("Opened ACIO device %s at %d", port_path, handle);

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
		log_warning("bad port/buf");
		return -1;
	}

	read_bytes = read(port_fd, bytes, nbytes);
	if (read_bytes < 0) {
		log_errno("read");
		return -1;
	}

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
		log_warning("bad port/buf");
		return -1;
	}

	written_bytes = write(port_fd, bytes, nbytes);
	if (written_bytes < 0) {
		log_errno("write");
	}

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

