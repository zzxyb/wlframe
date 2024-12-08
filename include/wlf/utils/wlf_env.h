#ifndef WLF_ENV_H
#define WLF_ENV_H

#include <stdbool.h>
#include <unistd.h>

/**
 * @brief Gets the value of an environment variable.
 * @param name The name of the environment variable.
 * @return The value of the environment variable, or NULL if not found.
 */
const char* wlf_get_env(const char *name);

/**
 * @brief Sets the value of an environment variable.
 * @param name The name of the environment variable.
 * @param value The value to set for the environment variable.
 * @return true if the operation was successful, false otherwise.
 */
bool wlf_set_env(const char *name, const char *value);

/**
 * @brief Unsets (removes) an environment variable.
 * @param name The name of the environment variable to remove.
 * @return true if the operation was successful, false otherwise.
 */
bool wlf_unset_env(const char *name);

/**
 * @brief Parses a boolean value from an environment variable option.
 * @param option The name of the environment variable to parse.
 * @return true if the option is set to a truthy value, false otherwise.
 */
bool wlf_env_parse_bool(const char *option);

/**
 * @brief Parses a switch value from an environment variable option.
 * @param option The name of the environment variable to parse.
 * @param switches An array of valid switch strings.
 * @return The index of the matched switch in the array, or SIZE_MAX if no match is found.
 */
size_t wlf_env_parse_switch(const char *option, const char **switches);

#endif
