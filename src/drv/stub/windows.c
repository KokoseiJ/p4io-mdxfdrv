#define LOG_MODULE "windows-stub"

#include "windows.h"
#include <util/log.h>
#include <sys/ioctl.h>
#include <time.h>

BOOL EscapeCommFunction(HANDLE hFile, DWORD dwFunc) {
	log_info("dwFunc %d stub", dwFunc);
	// return ioctl(hFile, dwFunc) == 0;
	return 1;
}


void Sleep(DWORD msec) {
	struct timespec ts;

	log_info("sleep %d msec", msec);

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

