#include "wlf/utils/wsm_command_line.h"
#include "wlf/utils/wlf_log.h"

struct wlf_command_line_parser *wlf_command_line_parser_create() {
	struct wlf_command_line_parser *parser = malloc(sizeof(struct wlf_command_line_parser));
	if (!parser) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for command line parser");
		return NULL;
	}

	parser->options = NULL;
	parser->option_count = 0;
	parser->positional_args = NULL;
	parser->positional_count = 0;

	return parser;
}

void wlf_command_line_parser_destroy(struct wlf_command_line_parser *parser) {
	if (parser) {
		for (int i = 0; i < parser->option_count; i++) {
			free((char *)parser->options[i].short_option);
			free((char *)parser->options[i].long_option);
			free((char *)parser->options[i].description);
			free((char *)parser->options[i].default_value);
		}

		free(parser->options);
		free(parser->positional_args);
		free(parser);
	}
}

void wlf_command_line_parser_add_option(struct wlf_command_line_parser *parser,
		const char *short_option, const char *long_option, const char *description,
		int requires_value, const char *default_value) {
	parser->options = realloc(parser->options, sizeof(struct wlf_command_line_option) * (parser->option_count + 1));
	if (!parser->options) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for command line options");
		return;
	}

	struct wlf_command_line_option *option = &parser->options[parser->option_count++];
	option->short_option = strdup(short_option);
	option->long_option = strdup(long_option);
	option->description = strdup(description);
	option->requires_value = requires_value;
	option->default_value = default_value ? strdup(default_value) : NULL;
	option->value = NULL;
	option->found = 0;
}

struct wlf_command_line_option *wlf_command_line_parser_get_option(
		struct wlf_command_line_parser *parser, const char *option) {
	for (int i = 0; i < parser->option_count; ++i) {
		if ((parser->options[i].short_option && strcmp(option, parser->options[i].short_option) == 0) ||
				(parser->options[i].long_option && strcmp(option, parser->options[i].long_option) == 0)) {
			return &parser->options[i];
		}
	}

	return NULL;
}

int wlf_command_line_parser_parse(struct wlf_command_line_parser *parser,
		int argc, char *argv[]) {
	parser->positional_args = malloc(sizeof(const char *) * argc);
	parser->positional_count = 0;

	for (int i = 1; i < argc; ++i) {
		char *arg = argv[i];

		if (arg[0] == '-') {
			char *equal_sign = strchr(arg, '=');
			char temp[256] = {0};
			const char *option_part = arg;
			const char *value_part = NULL;

			if (equal_sign) {
				size_t len = equal_sign - arg;
				if (len >= sizeof(temp)) len = sizeof(temp) - 1;
					strncpy(temp, arg, len);
					temp[len] = '\0';
					option_part = temp;
					value_part = equal_sign + 1;
				}

				struct wlf_command_line_option *opt = wlf_command_line_parser_get_option(parser, option_part);
				if (!opt) {
					wlf_log(WLF_ERROR, "Unknown option: %s", option_part);
					return -1;
				}
				opt->found = 1;
				if (opt->requires_value) {
					if (value_part) {
						opt->value = value_part;
					} else {
						if (i + 1 >= argc) {
							wlf_log(WLF_ERROR, "Option %s requires a value", arg);
							return -1;
						}
						opt->value = argv[++i];
					}
				}
			} else {
				parser->positional_args[parser->positional_count++] = arg;
			}
	}

	return 0;
}

const char *wlf_command_line_parser_get_value(
		struct wlf_command_line_parser *parser, const char *option) {
	struct wlf_command_line_option *opt = wlf_command_line_parser_get_option(parser, option);
	if (opt && (opt->found || opt->default_value)) {
		return opt->value;
	}

	return NULL;
}

int wlf_command_line_parser_is_set(
		struct wlf_command_line_parser *parser, const char *option) {
	struct wlf_command_line_option *opt = wlf_command_line_parser_get_option(parser, option);

	return opt ? opt->found : 0;
}

void wlf_command_line_parser_print_help(
		struct wlf_command_line_parser *parser, const char *program_name) {
	printf("Usage: %s [options] [arguments]\n\nOptions:\n", program_name);
	for (int i = 0; i < parser->option_count; ++i) {
		struct wlf_command_line_option *opt = &parser->options[i];
		printf("  %-4s %-20s %s",
				opt->short_option ? opt->short_option : "",
				opt->long_option ? opt->long_option : "",
				opt->description ? opt->description : "");
		if (opt->requires_value) {
			if (opt->default_value) {
				printf(" (default: %s)", opt->default_value);
			}
		}
		printf("\n");
	}
}
