#ifndef UTILS_WLF_PARSER_H
#define UTILS_WLF_PARSER_H

#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * @brief Structure representing a configuration entry.
 */
struct wlf_config_entry {
	struct wlf_linked_list link; /**< Link to the next entry in the list */
	char *key; /**< Key of the configuration entry */
	char *value; /**< Value of the configuration entry */
};

/**
 * @brief Structure representing a section of configuration entries.
 */
struct wlf_config_section {
	struct wlf_linked_list entry_list; /**< List of configuration entries in this section */
	struct wlf_linked_list link; /**< Link to the next section in the list */
	char *name; /**< Name of the configuration section */
};

/**
 * @brief Enumeration of option types for configuration.
 */
enum wlf_option_type {
	WLF_OPTION_INTEGER, /**< Integer option type */
	WLF_OPTION_UNSIGNED_INTEGER, /**< Unsigned integer option type */
	WLF_OPTION_STRING, /**< String option type */
	WLF_OPTION_BOOLEAN /**< Boolean option type */
};

/**
 * @brief Structure representing a configuration option.
 */
struct wlf_option {
	enum wlf_option_type type; /**< Type of the option */
	const char *name; /**< Name of the option */
	char short_name; /**< Short name for the option */
	void *data; /**< Pointer to the data associated with the option */
};

/**
 * @brief Structure representing the overall configuration.
 */
struct wlf_config {
	struct wlf_linked_list section_list; /**< List of configuration sections */
	char path[PATH_MAX]; /**< Path to the configuration file */
};

/**
 * @brief 打印命令行参数帮助信息。
 * @param options 指向 wlf_option 数组。
 * @param count   选项数量。
 * @param appname 程序名称（可用于 usage 行）。
 */
void wlf_print_options_help(const struct wlf_option *options, int count, const char *appname);

/**
 * @brief Parses command line options.
 * @param options Pointer to an array of wlf_option structures.
 * @param count Number of options in the array.
 * @param argc Pointer to the argument count.
 * @param argv Pointer to the argument vector.
 * @return 0 on success, or a negative error code on failure.
 */
int parse_options(const struct wlf_option *options, int count, int *argc, char *argv[]);

/**
 * @brief Retrieves an integer value from a configuration section.
 * @param section Pointer to the wlf_config_section instance.
 * @param key Key of the configuration entry.
 * @param value Pointer to store the retrieved integer value.
 * @param default_value Default value to return if the key is not found.
 * @return 0 on success, or a negative error code on failure.
 */
int wlf_config_section_get_int(struct wlf_config_section *section, const char *key,
	int32_t *value, int32_t default_value);

/**
 * @brief Retrieves an unsigned integer value from a configuration section.
 * @param section Pointer to the wlf_config_section instance.
 * @param key Key of the configuration entry.
 * @param value Pointer to store the retrieved unsigned integer value.
 * @param default_value Default value to return if the key is not found.
 * @return 0 on success, or a negative error code on failure.
 */
int wlf_config_section_get_uint(struct wlf_config_section *section, const char *key,
	uint32_t *value, uint32_t default_value);

/**
 * @brief Retrieves a double value from a configuration section.
 * @param section Pointer to the wlf_config_section instance.
 * @param key Key of the configuration entry.
 * @param value Pointer to store the retrieved double value.
 * @param default_value Default value to return if the key is not found.
 * @return 0 on success, or a negative error code on failure.
 */
int wlf_config_section_get_double(struct wlf_config_section *section, const char *key,
	double *value, double default_value);

/**
 * @brief Retrieves a string value from a configuration section.
 * @param section Pointer to the wlf_config_section instance.
 * @param key Key of the configuration entry.
 * @param value Pointer to store the retrieved string value.
 * @param default_value Default value to return if the key is not found.
 * @return 0 on success, or a negative error code on failure.
 */
int wlf_config_section_get_string(struct wlf_config_section *section, const char *key, char **value,
	const char *default_value);

/**
 * @brief Retrieves a boolean value from a configuration section.
 * @param section Pointer to the wlf_config_section instance.
 * @param key Key of the configuration entry.
 * @param value Pointer to store the retrieved boolean value.
 * @param default_value Default value to return if the key is not found.
 * @return 0 on success, or a negative error code on failure.
 */
int wlf_config_section_get_bool(struct wlf_config_section *section, const char *key,
	bool *value, bool default_value);

#endif // UTILS_WLF_PARSER_H
