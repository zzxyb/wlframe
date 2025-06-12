/**
 * @file        wlf_cmd_parser.h
 * @brief       Command line option parser and configuration utility for wlframe.
 * @details     This file provides functionality for parsing command line options and managing
 *              configuration entries. It supports integer, unsigned integer, string, and boolean
 *              option types with both short and long option formats. The parser can handle
 *              configuration files with sections and key-value pairs.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef UTILS_WLF_CMD_PARSER_H
#define UTILS_WLF_CMD_PARSER_H

#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * @brief Structure representing a configuration entry.
 *
 * Each configuration entry contains a key-value pair that can be used
 * to store configuration data within a section.
 */
struct wlf_cmd_config_entry {
	struct wlf_linked_list link; /**< Link to the next entry in the list */
	char *key;                   /**< Key of the configuration entry */
	char *value;                 /**< Value of the configuration entry */
};

/**
 * @brief Structure representing a section of configuration entries.
 *
 * Configuration sections group related entries together and can contain
 * multiple key-value pairs.
 */
struct wlf_cmd_config_section {
	struct wlf_linked_list entry_list; /**< List of configuration entries in this section */
	struct wlf_linked_list link;       /**< Link to the next section in the list */
	char *name;                         /**< Name of the configuration section */
};

/**
 * @brief Enumeration of option types for configuration.
 *
 * Defines the supported data types for command line options and configuration values.
 */
enum wlf_cmd_option_type {
	WLF_OPTION_INTEGER,          /**< Integer option type */
	WLF_OPTION_UNSIGNED_INTEGER, /**< Unsigned integer option type */
	WLF_OPTION_STRING,           /**< String option type */
	WLF_OPTION_BOOLEAN           /**< Boolean option type */
};

/**
 * @brief Structure representing a configuration option.
 *
 * Defines a command line option with its type, names, and data pointer.
 * Options can have both long names (--option) and short names (-o).
 *
 * @code
 * bool verbose = false;
 * struct wlf_cmd_option option = {
 *     .type = WLF_OPTION_BOOLEAN,
 *     .name = "verbose",
 *     .short_name = 'v',
 *     .data = &verbose
 * };
 * @endcode
 */
struct wlf_cmd_option {
	enum wlf_cmd_option_type type; /**< Type of the option */
	const char *name;              /**< Name of the option (for long format --name) */
	char short_name;               /**< Short name for the option (for short format -n) */
	void *data;                    /**< Pointer to the data associated with the option */
};

/**
 * @brief Structure representing the overall configuration.
 *
 * Contains a list of configuration sections and the path to the configuration file.
 */
struct wlf_cmd_config {
	struct wlf_linked_list section_list; /**< List of configuration sections */
	char path[PATH_MAX];                  /**< Path to the configuration file */
};

/**
 * @brief Prints command line option help information.
 *
 * This function generates and displays help text for the provided options,
 * showing both short and long option formats along with their types.
 *
 * @param options Pointer to array of wlf_cmd_option structures.
 * @param count   Number of options in the array.
 * @param appname Program name to use in the usage line.
 *
 * @code
 * struct wlf_cmd_option options[] = {
 *     {WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose},
 *     {WLF_OPTION_STRING, "file", 'f', &filename}
 * };
 * wlf_print_options_help(options, 2, "myprogram");
 * @endcode
 */
void wlf_print_options_help(const struct wlf_cmd_option *options, int count, const char *appname);

/**
 * @brief Parses command line options.
 *
 * Processes command line arguments according to the provided option specifications.
 * Supports both short format (-o) and long format (--option) arguments.
 * Non-option arguments are preserved in argv.
 *
 * @param options Pointer to an array of wlf_cmd_option structures.
 * @param count   Number of options in the array.
 * @param argc    Pointer to the argument count (will be modified).
 * @param argv    Pointer to the argument vector (will be modified).
 * @return        Number of remaining arguments, or negative error code on failure.
 *
 * @note The argc and argv parameters are modified during parsing to remove
 *       processed options, leaving only non-option arguments.
 *
 * @code
 * int argc = original_argc;
 * char **argv = original_argv;
 * int remaining = wlf_cmd_parse_options(options, option_count, &argc, argv);
 * if (remaining < 0) {
 *     // Handle parsing error
 * }
 * @endcode
 */
int wlf_cmd_parse_options(const struct wlf_cmd_option *options, int count, int *argc, char *argv[]);

/**
 * @brief Retrieves an integer value from a configuration section.
 *
 * Searches for the specified key in the configuration section and converts
 * its value to an integer. If the key is not found, the default value is used.
 *
 * @param section       Pointer to the wlf_cmd_config_section instance.
 * @param key           Key of the configuration entry to retrieve.
 * @param value         Pointer to store the retrieved integer value.
 * @param default_value Default value to return if the key is not found.
 * @return              0 on success, or a negative error code on failure.
 */
int wlf_cmd_config_section_get_int(struct wlf_cmd_config_section *section, const char *key,
	int32_t *value, int32_t default_value);

/**
 * @brief Retrieves an unsigned integer value from a configuration section.
 *
 * Searches for the specified key in the configuration section and converts
 * its value to an unsigned integer. If the key is not found, the default value is used.
 *
 * @param section       Pointer to the wlf_cmd_config_section instance.
 * @param key           Key of the configuration entry to retrieve.
 * @param value         Pointer to store the retrieved unsigned integer value.
 * @param default_value Default value to return if the key is not found.
 * @return              0 on success, or a negative error code on failure.
 */
int wlf_cmd_config_section_get_uint(struct wlf_cmd_config_section *section, const char *key,
	uint32_t *value, uint32_t default_value);

/**
 * @brief Retrieves a double value from a configuration section.
 *
 * Searches for the specified key in the configuration section and converts
 * its value to a double. If the key is not found, the default value is used.
 *
 * @param section       Pointer to the wlf_cmd_config_section instance.
 * @param key           Key of the configuration entry to retrieve.
 * @param value         Pointer to store the retrieved double value.
 * @param default_value Default value to return if the key is not found.
 * @return              0 on success, or a negative error code on failure.
 */
int wlf_cmd_config_section_get_double(struct wlf_cmd_config_section *section, const char *key,
	double *value, double default_value);

/**
 * @brief Retrieves a string value from a configuration section.
 *
 * Searches for the specified key in the configuration section and returns
 * a copy of its string value. If the key is not found, the default value is used.
 * The returned string must be freed by the caller.
 *
 * @param section       Pointer to the wlf_cmd_config_section instance.
 * @param key           Key of the configuration entry to retrieve.
 * @param value         Pointer to store the retrieved string value (must be freed).
 * @param default_value Default value to return if the key is not found.
 * @return              0 on success, or a negative error code on failure.
 *
 * @note The returned string is dynamically allocated and must be freed with free().
 */
int wlf_cmd_config_section_get_string(struct wlf_cmd_config_section *section, const char *key, char **value,
	const char *default_value);

/**
 * @brief Retrieves a boolean value from a configuration section.
 *
 * Searches for the specified key in the configuration section and converts
 * its value to a boolean. Accepts "true" and "false" as valid boolean strings.
 * If the key is not found, the default value is used.
 *
 * @param section       Pointer to the wlf_cmd_config_section instance.
 * @param key           Key of the configuration entry to retrieve.
 * @param value         Pointer to store the retrieved boolean value.
 * @param default_value Default value to return if the key is not found.
 * @return              0 on success, or a negative error code on failure.
 */
int wlf_cmd_config_section_get_bool(struct wlf_cmd_config_section *section, const char *key,
	bool *value, bool default_value);

#endif // UTILS_WLF_CMD_PARSER_H
