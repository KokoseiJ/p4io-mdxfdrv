#include "windows.h"
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <time.h>

BOOL EscapeCommFunction(HANDLE hFile, DWORD dwFunc) {
	return ioctl(hFile, dwFunc == 0 ? TIOCSBRK : TIOCCBRK) == 0;
}


void Sleep(DWORD msec) {
	struct timespec ts;

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

