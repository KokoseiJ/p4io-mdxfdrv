#include "thread.h"

void p4io_thread_create(p4io_thread_t *thread, void *func, void *param) {
	*thread = CreateThread(
		NULL,
		0,
		func,
		param,
		0
	);

	return;
}

