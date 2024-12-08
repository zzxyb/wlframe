#ifndef WLF_COMMAND_LINE_H
#define WLF_COMMAND_LINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct wlf_command_line_option
 * @brief Represents a single command-line option.
 *
 * This structure contains the short and long names of the option, a description,
 * whether the option requires a value, the default value, the current value,
 * and a flag indicating whether the option was found during parsing.
 */
struct wlf_command_line_option {
	const char *short_option;    /**< Short option name, e.g., "-h" */
	const char *long_option;     /**< Long option name, e.g., "--help" */
	const char *description;     /**< Description of the option */
	int requires_value;          /**< Whether the option requires a value (1 = yes, 0 = no) */
	const char *default_value;   /**< Default value for the option (if any) */
	const char *value;           /**< Current value of the option (if any) */
	int found;                   /**< Flag indicating if the option was found */
};

/**
 * @struct wlf_command_line_parser
 * @brief Represents the command-line parser.
 *
 * This structure contains all options, positional arguments, and their counts.
 */
struct wlf_command_line_parser {
	struct wlf_command_line_option *options; /**< Array of command-line options */
	int option_count;                        /**< Number of options */
	const char **positional_args;            /**< Array of positional arguments */
	int positional_count;                    /**< Number of positional arguments */
};

/**
 * @brief Creates a new command-line parser.
 * @return Pointer to the newly created parser.
 */
struct wlf_command_line_parser *wlf_command_line_parser_create();

/**
 * @brief Destroys the command-line parser and frees associated resources.
 * @param parser Pointer to the parser to destroy.
 */
void wlf_command_line_parser_destroy(struct wlf_command_line_parser *parser);

/**
 * @brief Adds a command-line option to the parser.
 * @param parser Pointer to the parser.
 * @param short_option Short option name.
 * @param long_option Long option name.
 * @param description Description of the option.
 * @param requires_value Whether the option requires a value.
 * @param default_value Default value for the option.
 */
void wlf_command_line_parser_add_option(struct wlf_command_line_parser *parser,
	const char *short_option, const char *long_option, const char *description,
	int requires_value, const char *default_value);

/**
 * @brief Retrieves the structure of a specified option.
 * @param parser Pointer to the parser.
 * @param option Option name (short or long).
 * @return Pointer to the option structure, or NULL if not found.
 */
struct wlf_command_line_option *wlf_command_line_parser_get_option(
	struct wlf_command_line_parser *parser, const char *option);

/**
 * @brief Parses the command-line arguments.
 * @param parser Pointer to the parser.
 * @param argc Number of arguments.
 * @param argv Array of argument strings.
 * @return 0 on success, non-zero on failure.
 */
int wlf_command_line_parser_parse(struct wlf_command_line_parser *parser,
	int argc, char *argv[]);

/**
 * @brief Retrieves the value of a specified option.
 * @param parser Pointer to the parser.
 * @param option Option name (short or long).
 * @return Value of the option, or NULL if not set.
 */
const char *wlf_command_line_parser_get_value(
	struct wlf_command_line_parser *parser, const char *option);

/**
 * @brief Checks if a specified option is set.
 * @param parser Pointer to the parser.
 * @param option Option name (short or long).
 * @return 1 if the option is set, 0 otherwise.
 */
int wlf_command_line_parser_is_set(
	struct wlf_command_line_parser *parser, const char *option);

/**
 * @brief Prints the command-line help information.
 * @param parser Pointer to the parser.
 * @param program_name Name of the program (typically argv[0]).
 */
void wlf_command_line_parser_print_help(
	struct wlf_command_line_parser *parser, const char *program_name);

#endif