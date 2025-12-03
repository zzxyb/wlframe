#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

static bool handle_option(const struct wlf_cmd_option *option, char *value) {
	char* p;

	switch (option->type) {
	case WLF_OPTION_INTEGER:
		if (!safe_strtoint(value, option->data))
			return false;
		return true;
	case WLF_OPTION_UNSIGNED_INTEGER:
		errno = 0;
		* (uint32_t *) option->data = strtoul(value, &p, 10);
		if (errno != 0 || p == value || *p != '\0')
			return false;
		return true;
	case WLF_OPTION_STRING:
		* (char **) option->data = strdup(value);
		return true;
	default:
		assert(0);
		return false;
	}
}

static bool long_option(const struct wlf_cmd_option *options, int count, char *arg) {
	int k, len;

	for (k = 0; k < count; k++) {
		if (options[k].name == NULL)
			continue;

		len = strlen(options[k].name);
		if (strncmp(options[k].name, arg + 2, len) != 0)
			continue;

		if (options[k].type == WLF_OPTION_BOOLEAN) {
			if (!arg[len + 2]) {
				* (bool *) options[k].data = true;

				return true;
			}
		} else if (arg[len+2] == '=') {
			return handle_option(options + k, arg + len + 3);
		}
	}

	return false;
}

static bool long_option_with_arg(const struct wlf_cmd_option *options, int count, char *arg,
	char *param) {
	int k, len;

	for (k = 0; k < count; k++) {
		if (!options[k].name)
			continue;

		len = strlen(options[k].name);
		if (strncmp(options[k].name, arg + 2, len) != 0)
			continue;

		assert(options[k].type != WLF_OPTION_BOOLEAN);

		return handle_option(options + k, param);
	}

	return false;
}

static bool short_option(const struct wlf_cmd_option *options, int count, char *arg) {
	int k;

	if (!arg[1])
		return false;

	for (k = 0; k < count; k++) {
		if (options[k].short_name != arg[1])
			continue;

		if (options[k].type == WLF_OPTION_BOOLEAN) {
			if (!arg[2]) {
				* (bool *) options[k].data = true;

				return true;
			}
		} else if (arg[2]) {
			return handle_option(options + k, arg + 2);
		} else {
			return false;
		}
	}

	return false;
}

static bool short_option_with_arg(const struct wlf_cmd_option *options, int count, char *arg, char *param) {
	int k;

	if (!arg[1])
		return false;

	for (k = 0; k < count; k++) {
		if (options[k].short_name != arg[1])
			continue;

		if (options[k].type == WLF_OPTION_BOOLEAN)
			continue;

		return handle_option(options + k, param);
	}

	return false;
}

static struct wlf_cmd_config_entry *config_section_get_entry(struct wlf_cmd_config_section *section,
	const char *key) {
	struct wlf_cmd_config_entry *e;

	if (section == NULL)
		return NULL;
	wlf_linked_list_for_each(e, &section->entry_list, link)
		if (strcmp(e->key, key) == 0)
			return e;

	return NULL;
}

void wlf_print_options_help(const struct wlf_cmd_option *options, int count, const char *appname) {
	printf("Usage: %s [options]\n\n", appname ? appname : "program");
	printf("Options:\n");
	for (int i = 0; i < count; ++i) {
		const struct wlf_cmd_option *opt = &options[i];
		if (opt->short_name && opt->name) {
			printf("  -%c, --%s", opt->short_name, opt->name);
		} else if (opt->short_name) {
			printf("  -%c", opt->short_name);
		} else if (opt->name) {
			printf("      --%s", opt->name);
		} else {
			continue;
		}

		switch (opt->type) {
			case WLF_OPTION_INTEGER:
				printf(" <int>");
				break;
			case WLF_OPTION_UNSIGNED_INTEGER:
				printf(" <uint>");
				break;
			case WLF_OPTION_STRING:
				printf(" <string>");
				break;
			case WLF_OPTION_BOOLEAN:
				// Boolean options do not require a value
				break;
			default:
				break;
		}
		printf("\n");
	}

	printf("  -h, --help    Show this help message and exit\n");
}

int wlf_cmd_parse_options(const struct wlf_cmd_option *options, int count, int *argc, char *argv[]) {
	int i, j;

	for (i = 1, j = 1; i < *argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == '-') {
				if (long_option(options, count, argv[i]))
					continue;

				if (i + 1 < *argc &&
					long_option_with_arg(options, count,
						argv[i], argv[i+1])) {
					i++;
					continue;
				}
			} else {
				/* Short option, e.g -f or -f42 */
				if (short_option(options, count, argv[i]))
					continue;

				if (i+1 < *argc &&
					short_option_with_arg(options, count, argv[i], argv[i+1])) {
					i++;
					continue;
				}
			}
		}
		argv[j++] = argv[i];
	}
	argv[j] = NULL;
	*argc = j;

	return j;
}

int wlf_cmd_config_section_get_int(struct wlf_cmd_config_section *section, const char *key,
	int32_t *value, int32_t default_value) {
	struct wlf_cmd_config_entry *entry;

	entry = config_section_get_entry(section, key);
	if (entry == NULL) {
		*value = default_value;
		errno = ENOENT;
		return -1;
	}

	if (!safe_strtoint(entry->value, value)) {
		*value = default_value;
		return -1;
	}

	return 0;
}

int wlf_cmd_config_section_get_uint(struct wlf_cmd_config_section *section, const char *key,
	uint32_t *value, uint32_t default_value) {
	long int ret;
	struct wlf_cmd_config_entry *entry;
	char *end;

	entry = config_section_get_entry(section, key);
	if (entry == NULL) {
		*value = default_value;
		errno = ENOENT;
		return -1;
	}

	errno = 0;
	ret = strtol(entry->value, &end, 0);
	if (errno != 0 || end == entry->value || *end != '\0') {
		*value = default_value;
		errno = EINVAL;
		return -1;
	}

	if (ret < 0 || ret > INT_MAX) {
		*value = default_value;
		errno = ERANGE;
		return -1;
	}

	*value = ret;

	return 0;
}

int wlf_cmd_config_section_get_double( struct wlf_cmd_config_section *section, const char *key,
	double *value, double default_value) {
	struct wlf_cmd_config_entry *entry;
	char *end;

	entry = config_section_get_entry(section, key);
	if (entry == NULL) {
		*value = default_value;
		errno = ENOENT;
		return -1;
	}

	*value = strtod(entry->value, &end);
	if (*end != '\0') {
		*value = default_value;
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int wlf_cmd_config_section_get_string(struct wlf_cmd_config_section *section, const char *key,
	char **value, const char *default_value) {
	struct wlf_cmd_config_entry *entry;

	entry = config_section_get_entry(section, key);
	if (entry == NULL) {
		if (default_value)
			*value = strdup(default_value);
		else
			*value = NULL;
		errno = ENOENT;
		return -1;
	}

	*value = strdup(entry->value);

	return 0;
}

int wlf_cmd_config_section_get_bool(struct wlf_cmd_config_section *section,
	const char *key, bool *value, bool default_value) {
	struct wlf_cmd_config_entry *entry;

	entry = config_section_get_entry(section, key);
	if (entry == NULL) {
		*value = default_value;
		errno = ENOENT;
		return -1;
	}

	if (strcmp(entry->value, "false") == 0)
		*value = false;
	else if (strcmp(entry->value, "true") == 0)
		*value = true;
	else {
		*value = default_value;
		errno = EINVAL;
		return -1;
	}

	return 0;
}
