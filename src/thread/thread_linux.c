#include "thread.h"

void p4io_thread_create(p4io_thread_t *thread, void *func, void *param) {
	pthread_create(
		thread,
		NULL,
		func,
		param
	);

	return;
}

