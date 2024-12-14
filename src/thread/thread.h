#ifndef P4IO_THREAD_H
#define P4IO_THREAD_H

#ifdef _WIN32

#include <processthreadsapi.h>
typedef HANDLE p4io_thread_t;

#else

#include <pthread.h>
typedef pthread_t p4io_thread_t;

#endif


void p4io_thread_create(p4io_thread_t *thread, void *func, void *param);

#endif

