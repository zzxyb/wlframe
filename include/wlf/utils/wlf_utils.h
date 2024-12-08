#ifndef WLF_UTILS_H
#define WLF_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define TOKEN_SIZE 33

/**
 * @brief Checks if a given string is valid UTF-8.
 * @param string The string to check.
 * @return true if the string is valid UTF-8, false otherwise.
 */
bool is_utf8(const char *string);

/**
 * @brief Generates a random token and stores it in the provided output buffer.
 * @param out The output buffer to store the generated token. Must be at least TOKEN_SIZE bytes long.
 */
bool generate_token(char out[static TOKEN_SIZE]);

/**
 * Add target to values.
 *
 * Target is added to the end of the set.
 *
 * Returns the index of target, or -1 if the set is full or target already
 * exists.
 */
ssize_t set_add(uint32_t values[], size_t *len, size_t cap, uint32_t target);

/**
 * Remove target from values.
 *
 * When target is removed, the last element of the set is moved to where
 * target was.
 *
 * Returns the previous index of target, or -1 if target wasn't in values.
 */
ssize_t set_remove(uint32_t values[], size_t *len, size_t cap, uint32_t target);

/**
 * @brief Avoids "unused parameter" warnings.
 * @param x The parameter to be ignored.
 */
#define WLF_UNUSED(x) (void)x;

#endif
