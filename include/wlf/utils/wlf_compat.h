#ifndef UTILS_WLF_COMPAT_H
#define UTILS_WLF_COMPAT_H

#include "wlf/config.h"

#if WLF_HAS_WINDOWS_PLATFORM
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <BaseTsd.h>
#include <io.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef SSIZE_T ssize_t;

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

#ifndef strdup
#define strdup _strdup
#endif

static inline char *wlf_strndup(const char *src, size_t len) {
	char *dst = malloc(len + 1);
	if (dst == NULL) {
		return NULL;
	}

	memcpy(dst, src, len);
	dst[len] = '\0';
	return dst;
}

#ifndef strndup
#define strndup wlf_strndup
#endif

#ifndef isatty
#define isatty _isatty
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#include <stdbool.h>

static inline bool wlf_mul_overflow(size_t a, size_t b, size_t *result) {
	if (b != 0 && a > SIZE_MAX / b) {
		return true;
	}
	*result = a * b;
	return false;
}
#else
#include <stdbool.h>
#include <strings.h>
#include <unistd.h>

static inline bool wlf_mul_overflow(size_t a, size_t b, size_t *result) {
	return __builtin_mul_overflow(a, b, result);
}
#endif

#endif // UTILS_WLF_COMPAT_H
