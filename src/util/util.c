#define LOG_MODULE "util_linux"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "util/log.h"

void *xmalloc(size_t nbytes) {
	void *mem = calloc(nbytes, 1);

	if (mem == NULL) {
		log_fatal("xcalloc(%u) failed", (uint32_t) nbytes);

		return NULL;
	}

	return mem;
}


size_t str_vformat(char *buf, size_t nchars, const char *fmt, va_list ap)
{
    int result;

    result = vsnprintf(buf, nchars, fmt, ap);

    if (result >= (int) nchars || result < 0) {
        abort();
    }

    return (size_t) result;
}


size_t str_format(char *buf, size_t nchars, const char *fmt, ...)
{
    va_list ap;
    size_t result;

    va_start(ap, fmt);
    result = str_vformat(buf, nchars, fmt, ap);
    va_end(ap);

    return result;
}

