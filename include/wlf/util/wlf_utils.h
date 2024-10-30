#ifndef WLF_UTILS_H
#define WLF_UTILS_H

#include <stdbool.h>

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

#endif
