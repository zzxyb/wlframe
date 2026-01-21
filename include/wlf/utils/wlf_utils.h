/**
 * @file        wlf_utils.h
 * @brief       Utility functions for wlframe.
 * @details     This file provides common utility functions and macros, including UTF-8 validation,
 *              random token generation, and simple set operations for integer arrays.
 *              It also includes a macro to suppress unused parameter warnings.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef UTILS_WLF_UTILS_H
#define UTILS_WLF_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#define TOKEN_SIZE 33

/**
 * @brief Check if a given string is valid UTF-8.
 *
 * @param string The string to check.
 * @return true if the string is valid UTF-8, false otherwise.
 */
bool is_utf8(const char *string);

/**
 * @brief Generate a random token and store it in the provided output buffer.
 *
 * @param[out] out The output buffer to store the generated token. Must be at least TOKEN_SIZE bytes long.
 * @return true on success, false on failure.
 */
bool generate_token(char out[static TOKEN_SIZE]);

/**
 * @brief Add a target value to the set.
 *
 * The target is added to the end of the set if not already present and if there is capacity.
 *
 * @param[in,out] values The integer set array.
 * @param[in,out] len Pointer to the current length of the set.
 * @param[in] cap The maximum capacity of the set.
 * @param[in] target The value to add.
 * @return The index of the target, or -1 if the set is full or the target already exists.
 */
ssize_t set_add(uint32_t values[], size_t *len, size_t cap, uint32_t target);

/**
 * @brief Remove a target value from the set.
 *
 * When the target is removed, the last element of the set is moved to the removed position.
 *
 * @param[in,out] values The integer set array.
 * @param[in,out] len Pointer to the current length of the set.
 * @param[in] cap The maximum capacity of the set.
 * @param[in] target The value to remove.
 * @return The previous index of the target, or -1 if the target was not found.
 */
ssize_t set_remove(uint32_t values[], size_t *len, size_t cap, uint32_t target);

/**
 * @brief Avoid "unused parameter" warnings.
 *
 * @param x The parameter to be ignored.
 */
#define WLF_UNUSED(x) (void)x;

/**
 * @brief Allocates a POSIX shared memory file descriptor.
 *
 * Creates an anonymous shared memory file of the specified size using
 * memfd_create or shm_open as a fallback.
 *
 * @param size Size of the shared memory region in bytes.
 * @return File descriptor on success, -1 on failure.
 */
int wlf_allocate_shm_file(size_t size);

/**
 * @brief Converts a string to an integer safely.
 *
 * Parses a base-10 number from the given string. Checks that the
 * string is not blank, contains only numerical characters, and is
 * within the range of INT32_MIN to INT32_MAX. If the validation is
 * successful, the result is stored in *value; otherwise *value is
 * unchanged and errno is set appropriately.
 *
 * @param str Input string to parse.
 * @param value Pointer to store the converted integer.
 * @return true if the number was parsed successfully, false on error.
 */
static inline bool safe_strtoint(const char *str, int32_t *value) {
	long ret;
	char *end;

	assert(str != NULL);

	errno = 0;
	ret = strtol(str, &end, 10);
	if (errno != 0) {
		return false;
	} else if (end == str || *end != '\0') {
		errno = EINVAL;
		return false;
	}

	if ((long)((int32_t)ret) != ret) {
		errno = ERANGE;
		return false;
	}
	*value = (int32_t)ret;

	return true;
}

#endif // UTILS_WLF_UTILS_H
