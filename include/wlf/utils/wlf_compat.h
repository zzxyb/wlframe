/**
 * @file        wlf_compat.h
 * @brief       Cross-platform compatibility helpers for wlframe.
 * @details     This file provides small portability definitions and standard
 *              library compatibility helpers used by platform-specific code.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** @brief Signed size type used by POSIX-compatible interfaces on Windows. */
typedef SSIZE_T ssize_t;

#ifndef strcasecmp
/**
 * @brief Maps case-insensitive string comparison to the Windows CRT.
 */
#define strcasecmp _stricmp
#endif

#ifndef strncasecmp
/**
 * @brief Maps bounded case-insensitive comparison to the Windows CRT.
 */
#define strncasecmp _strnicmp
#endif

#ifndef strdup
/**
 * @brief Maps string duplication to the Windows CRT.
 */
#define strdup _strdup
#endif

/**
 * @brief Copies exactly @p len bytes and appends a terminating NUL byte.
 * @param src Source containing at least @p len readable bytes.
 * @param len Number of bytes to copy.
 * @return Allocated NUL-terminated copy, or NULL on allocation failure; free with free().
 */
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
/**
 * @brief Maps bounded byte duplication to the local Windows helper.
 */
#define strndup wlf_strndup
#endif

#ifndef isatty
/**
 * @brief Maps terminal descriptor detection to the Windows CRT.
 */
#define isatty _isatty
#endif

#ifndef STDERR_FILENO
/**
 * @brief File descriptor number for standard error.
 */
#define STDERR_FILENO 2
#endif
#else
#include <strings.h>
#include <unistd.h>
#endif

#endif // UTILS_WLF_COMPAT_H
