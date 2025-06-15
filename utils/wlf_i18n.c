#include "wlf/utils/wlf_i18n.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>

// Global i18n system instance
static struct wlf_i18n_system *g_i18n_system = NULL;

// Default plural rule for English (and similar languages)
static int default_plural_rule(int n) {
	return (n != 1) ? 1 : 0;
}

// Chinese plural rule (no plural forms)
static int chinese_plural_rule(int n) {
	return 0;
}

// French plural rule
static int french_plural_rule(int n) {
	return (n > 1) ? 1 : 0;
}

// Russian plural rule (simplified)
static int russian_plural_rule(int n) {
	int mod10 = n % 10;
	int mod100 = n % 100;

	if (mod10 == 1 && mod100 != 11) {
		return 0; // one
	} else if (mod10 >= 2 && mod10 <= 4 && (mod100 < 10 || mod100 >= 20)) {
		return 1; // few
	} else {
		return 2; // many
	}
}

// Built-in locale definitions
static struct wlf_i18n_locale builtin_locales[] = {
	{
		.code = "en_US",
		.name = "English (United States)",
		.native_name = "English (United States)",
		.direction = WLF_I18N_DIR_LTR,
		.plural_rule = default_plural_rule,
		.plural_forms = 2,
		.decimal_separator = ".",
		.thousands_separator = ",",
		.date_format = "%m/%d/%Y",
		.time_format = "%I:%M:%S %p"
	},
	{
		.code = "zh_CN",
		.name = "Chinese (Simplified)",
		.native_name = "中文（简体）",
		.direction = WLF_I18N_DIR_LTR,
		.plural_rule = chinese_plural_rule,
		.plural_forms = 1,
		.decimal_separator = ".",
		.thousands_separator = ",",
		.date_format = "%Y-%m-%d",
		.time_format = "%H:%M:%S"
	},
	{
		.code = "fr_FR",
		.name = "French (France)",
		.native_name = "Français (France)",
		.direction = WLF_I18N_DIR_LTR,
		.plural_rule = french_plural_rule,
		.plural_forms = 2,
		.decimal_separator = ",",
		.thousands_separator = " ",
		.date_format = "%d/%m/%Y",
		.time_format = "%H:%M:%S"
	},
	{
		.code = "ru_RU",
		.name = "Russian (Russia)",
		.native_name = "Русский (Россия)",
		.direction = WLF_I18N_DIR_LTR,
		.plural_rule = russian_plural_rule,
		.plural_forms = 3,
		.decimal_separator = ",",
		.thousands_separator = " ",
		.date_format = "%d.%m.%Y",
		.time_format = "%H:%M:%S"
	}
};

static struct wlf_i18n_entry *wlf_i18n_entry_create(const char *key, const char *context, const char *translation) {
	struct wlf_i18n_entry *entry = calloc(1, sizeof(struct wlf_i18n_entry));
	if (!entry) {
		return NULL;
	}

	entry->key = strdup(key);
	if (context) {
		entry->context = strdup(context);
	}
	entry->translation = strdup(translation);

	if (!entry->key || !entry->translation || (context && !entry->context)) {
		free(entry->key);
		free(entry->context);
		free(entry->translation);
		free(entry);
		return NULL;
	}

	wlf_linked_list_init(&entry->link);
	return entry;
}

static void wlf_i18n_entry_destroy(struct wlf_i18n_entry *entry) {
	if (!entry) {
		return;
	}

	free(entry->key);
	free(entry->context);
	free(entry->translation);

	if (entry->plurals) {
		for (int i = 0; i < entry->plural_count; i++) {
			free(entry->plurals[i]);
		}
		free(entry->plurals);
	}

	wlf_linked_list_remove(&entry->link);
	free(entry);
}

static struct wlf_i18n_catalog *wlf_i18n_catalog_create(const char *locale, const char *domain) {
	struct wlf_i18n_catalog *catalog = calloc(1, sizeof(struct wlf_i18n_catalog));
	if (!catalog) {
		return NULL;
	}

	strncpy(catalog->locale, locale, sizeof(catalog->locale) - 1);
	if (domain) {
		catalog->domain = strdup(domain);
		if (!catalog->domain) {
			free(catalog);
			return NULL;
		}
	}

	wlf_linked_list_init(&catalog->link);
	wlf_linked_list_init(&catalog->entries);
	return catalog;
}

static void wlf_i18n_catalog_destroy(struct wlf_i18n_catalog *catalog) {
	if (!catalog) {
		return;
	}

	// Destroy all entries
	struct wlf_i18n_entry *entry, *tmp;
	wlf_linked_list_for_each_safe(entry, tmp, &catalog->entries, link) {
		wlf_i18n_entry_destroy(entry);
	}

	free(catalog->domain);
	wlf_linked_list_remove(&catalog->link);
	free(catalog);
}

static struct wlf_i18n_catalog *wlf_i18n_find_catalog(const char *locale, const char *domain) {
	if (!g_i18n_system) {
		return NULL;
	}

	struct wlf_i18n_catalog *catalog;
	wlf_linked_list_for_each(catalog, &g_i18n_system->catalogs, link) {
		if (strcmp(catalog->locale, locale) == 0) {
			if (!domain && !catalog->domain) {
				return catalog;
			}
			if (domain && catalog->domain && strcmp(catalog->domain, domain) == 0) {
				return catalog;
			}
		}
	}

	return NULL;
}

static struct wlf_i18n_entry *wlf_i18n_find_entry(struct wlf_i18n_catalog *catalog,
                                                   const char *key, const char *context) {
	if (!catalog) {
		return NULL;
	}

	struct wlf_i18n_entry *entry;
	wlf_linked_list_for_each(entry, &catalog->entries, link) {
		if (strcmp(entry->key, key) == 0) {
			if (!context && !entry->context) {
				return entry;
			}
			if (context && entry->context && strcmp(entry->context, context) == 0) {
				return entry;
			}
		}
	}

	return NULL;
}

bool wlf_i18n_init(const char *default_locale) {
	if (g_i18n_system) {
		wlf_log(WLF_INFO, "I18n system already initialized");
		return true;
	}

	g_i18n_system = calloc(1, sizeof(struct wlf_i18n_system));
	if (!g_i18n_system) {
		wlf_log(WLF_ERROR, "Failed to allocate i18n system");
		return false;
	}

	// Set default locale
	if (default_locale) {
		strncpy(g_i18n_system->current_locale, default_locale,
		        sizeof(g_i18n_system->current_locale) - 1);
	} else {
		strcpy(g_i18n_system->current_locale, "en_US");
	}

	// Initialize lists
	wlf_linked_list_init(&g_i18n_system->catalogs);
	wlf_linked_list_init(&g_i18n_system->locales);

	// Initialize signals
	wlf_signal_init(&g_i18n_system->events.locale_changed);

	// Register built-in locales
	for (size_t i = 0; i < sizeof(builtin_locales) / sizeof(builtin_locales[0]); i++) {
		wlf_i18n_register_locale(&builtin_locales[i]);
	}

	wlf_log(WLF_INFO, "I18n system initialized with locale: %s", g_i18n_system->current_locale);
	return true;
}

void wlf_i18n_cleanup(void) {
	if (!g_i18n_system) {
		return;
	}

	// Destroy all catalogs
	struct wlf_i18n_catalog *catalog, *tmp_catalog;
	wlf_linked_list_for_each_safe(catalog, tmp_catalog, &g_i18n_system->catalogs, link) {
		wlf_i18n_catalog_destroy(catalog);
	}

	free(g_i18n_system->default_domain);
	free(g_i18n_system);
	g_i18n_system = NULL;

	wlf_log(WLF_DEBUG, "I18n system cleaned up");
}

struct wlf_i18n_system *wlf_i18n_get_system(void) {
	return g_i18n_system;
}

bool wlf_i18n_set_locale(const char *locale) {
	if (!g_i18n_system || !locale) {
		return false;
	}

	if (!wlf_i18n_is_locale_supported(locale)) {
		wlf_log(WLF_INFO, "Locale not supported: %s", locale);
		return false;
	}

	char old_locale[WLF_I18N_LOCALE_MAX_LEN];
	strcpy(old_locale, g_i18n_system->current_locale);

	strncpy(g_i18n_system->current_locale, locale, sizeof(g_i18n_system->current_locale) - 1);
	g_i18n_system->current_locale[sizeof(g_i18n_system->current_locale) - 1] = '\0';

	// Emit locale changed signal
	wlf_signal_emit(&g_i18n_system->events.locale_changed, (void*)locale);

	wlf_log(WLF_INFO, "Locale changed from %s to %s", old_locale, locale);
	return true;
}

const char *wlf_i18n_get_locale(void) {
	return g_i18n_system ? g_i18n_system->current_locale : "en_US";
}

bool wlf_i18n_load_translation(const char *filename, const char *locale, const char *domain) {
	if (!g_i18n_system || !filename || !locale) {
		return false;
	}

	FILE *file = fopen(filename, "r");
	if (!file) {
		wlf_log(WLF_ERROR, "Failed to open translation file: %s", filename);
		return false;
	}

	struct wlf_i18n_catalog *catalog = wlf_i18n_find_catalog(locale, domain);
	if (!catalog) {
		catalog = wlf_i18n_catalog_create(locale, domain);
		if (!catalog) {
			fclose(file);
			return false;
		}
		wlf_linked_list_insert(&g_i18n_system->catalogs, &catalog->link);
	}

	char line[WLF_I18N_MESSAGE_MAX_LEN];
	char key[WLF_I18N_KEY_MAX_LEN];
	char translation[WLF_I18N_MESSAGE_MAX_LEN];
	int line_num = 0;

	while (fgets(line, sizeof(line), file)) {
		line_num++;

		// Skip empty lines and comments
		char *trimmed = line;
		while (isspace(*trimmed)) trimmed++;
		if (*trimmed == '\0' || *trimmed == '#') {
			continue;
		}

		// Parse key=value format
		char *equals = strchr(trimmed, '=');
		if (!equals) {
			wlf_log(WLF_INFO, "Invalid format at line %d in %s", line_num, filename);
			continue;
		}

		*equals = '\0';
		char *value = equals + 1;

		// Trim key
		char *key_end = equals - 1;
		while (key_end > trimmed && isspace(*key_end)) {
			*key_end = '\0';
			key_end--;
		}

		// Trim value
		while (isspace(*value)) value++;
		char *value_end = value + strlen(value) - 1;
		while (value_end > value && (*value_end == '\n' || isspace(*value_end))) {
			*value_end = '\0';
			value_end--;
		}

		// Remove quotes if present
		if (*value == '"' && value_end > value && *value_end == '"') {
			value++;
			*value_end = '\0';
		}

		strncpy(key, trimmed, sizeof(key) - 1);
		key[sizeof(key) - 1] = '\0';
		strncpy(translation, value, sizeof(translation) - 1);
		translation[sizeof(translation) - 1] = '\0';

		// Create entry
		struct wlf_i18n_entry *entry = wlf_i18n_entry_create(key, NULL, translation);
		if (entry) {
			wlf_linked_list_insert(&catalog->entries, &entry->link);
		}
	}

	fclose(file);
	wlf_log(WLF_INFO, "Loaded translation file: %s for locale: %s", filename, locale);
	return true;
}

bool wlf_i18n_load_json(const char *json_content, const char *locale, const char *domain) {
	if (!g_i18n_system || !json_content || !locale) {
		return false;
	}

	// Simple JSON parser for key-value pairs
	// This is a basic implementation - for production, consider using a proper JSON library

	struct wlf_i18n_catalog *catalog = wlf_i18n_find_catalog(locale, domain);
	if (!catalog) {
		catalog = wlf_i18n_catalog_create(locale, domain);
		if (!catalog) {
			return false;
		}
		wlf_linked_list_insert(&g_i18n_system->catalogs, &catalog->link);
	}

	// TODO: Implement proper JSON parsing
	// For now, this is a placeholder
	wlf_log(WLF_INFO, "JSON translation loading not yet implemented");
	return false;
}

const char *wlf_i18n_translate(const char *key, const char *domain) {
	return wlf_i18n_translate_context(NULL, key, domain);
}

const char *wlf_i18n_translate_context(const char *context, const char *key, const char *domain) {
	if (!g_i18n_system || !key) {
		return key;
	}

	struct wlf_i18n_catalog *catalog = wlf_i18n_find_catalog(g_i18n_system->current_locale, domain);
	if (!catalog) {
		// Try fallback to base language (e.g., "en" from "en_US")
		char base_locale[8];
		const char *underscore = strchr(g_i18n_system->current_locale, '_');
		if (underscore) {
			size_t len = underscore - g_i18n_system->current_locale;
			if (len < sizeof(base_locale)) {
				strncpy(base_locale, g_i18n_system->current_locale, len);
				base_locale[len] = '\0';
				catalog = wlf_i18n_find_catalog(base_locale, domain);
			}
		}
	}

	if (!catalog) {
		return key; // Return original key if no translation found
	}

	struct wlf_i18n_entry *entry = wlf_i18n_find_entry(catalog, key, context);
	return entry ? entry->translation : key;
}

const char *wlf_i18n_translate_plural(const char *key, const char *plural_key, int n, const char *domain) {
	if (!g_i18n_system || !key) {
		return key;
	}

	const struct wlf_i18n_locale *locale_info = wlf_i18n_get_locale_info(g_i18n_system->current_locale);
	if (!locale_info || !locale_info->plural_rule) {
		// No plural rule, use simple logic
		return (n == 1) ? wlf_i18n_translate(key, domain) :
		                  wlf_i18n_translate(plural_key ? plural_key : key, domain);
	}

	int plural_form = locale_info->plural_rule(n);

	struct wlf_i18n_catalog *catalog = wlf_i18n_find_catalog(g_i18n_system->current_locale, domain);
	if (!catalog) {
		return (plural_form == 0) ? key : (plural_key ? plural_key : key);
	}

	struct wlf_i18n_entry *entry = wlf_i18n_find_entry(catalog, key, NULL);
	if (!entry) {
		return (plural_form == 0) ? key : (plural_key ? plural_key : key);
	}

	if (entry->plurals && plural_form < entry->plural_count) {
		return entry->plurals[plural_form];
	}

	return entry->translation;
}

char *wlf_i18n_translate_format(const char *key, const char *domain, ...) {
	if (!key) {
		return NULL;
	}

	const char *template = wlf_i18n_translate(key, domain);

	va_list args;
	va_start(args, domain);

	// Calculate required buffer size
	va_list args_copy;
	va_copy(args_copy, args);
	int size = vsnprintf(NULL, 0, template, args_copy);
	va_end(args_copy);

	if (size < 0) {
		va_end(args);
		return NULL;
	}

	char *result = malloc(size + 1);
	if (!result) {
		va_end(args);
		return NULL;
	}

	vsnprintf(result, size + 1, template, args);
	va_end(args);

	return result;
}

bool wlf_i18n_register_locale(struct wlf_i18n_locale *locale) {
	if (!g_i18n_system || !locale) {
		return false;
	}

	// Check if locale already exists
	struct wlf_i18n_locale *existing;
	wlf_linked_list_for_each(existing, &g_i18n_system->locales, link) {
		if (strcmp(existing->code, locale->code) == 0) {
			wlf_log(WLF_INFO, "Locale already registered: %s", locale->code);
			return false;
		}
	}

	// For built-in locales, we don't need to allocate
	bool is_builtin = false;
	for (size_t i = 0; i < sizeof(builtin_locales) / sizeof(builtin_locales[0]); i++) {
		if (locale == &builtin_locales[i]) {
			is_builtin = true;
			break;
		}
	}

	if (is_builtin) {
		// Built-in locale, just add to list without copying
		wlf_linked_list_insert(&g_i18n_system->locales, (struct wlf_linked_list*)locale);
	} else {
		// Copy the locale data
		struct wlf_i18n_locale *copy = malloc(sizeof(struct wlf_i18n_locale));
		if (!copy) {
			return false;
		}
		*copy = *locale;

		// Copy string fields
		if (locale->name) copy->name = strdup(locale->name);
		if (locale->native_name) copy->native_name = strdup(locale->native_name);
		if (locale->decimal_separator) copy->decimal_separator = strdup(locale->decimal_separator);
		if (locale->thousands_separator) copy->thousands_separator = strdup(locale->thousands_separator);
		if (locale->date_format) copy->date_format = strdup(locale->date_format);
		if (locale->time_format) copy->time_format = strdup(locale->time_format);

		wlf_linked_list_insert(&g_i18n_system->locales, (struct wlf_linked_list*)copy);
	}

	wlf_log(WLF_DEBUG, "Registered locale: %s", locale->code);
	return true;
}

const char **wlf_i18n_get_available_locales(int *count) {
	if (!g_i18n_system || !count) {
		if (count) *count = 0;
		return NULL;
	}

	// Count locales
	int locale_count = 0;
	struct wlf_i18n_locale *locale;
	wlf_linked_list_for_each(locale, &g_i18n_system->locales, link) {
		locale_count++;
	}

	if (locale_count == 0) {
		*count = 0;
		return NULL;
	}

	// Allocate array
	static const char **locale_array = NULL;
	static int last_count = 0;

	if (locale_count != last_count) {
		free(locale_array);
		locale_array = malloc(sizeof(char*) * locale_count);
		last_count = locale_count;
	}

	if (!locale_array) {
		*count = 0;
		return NULL;
	}

	// Fill array
	int i = 0;
	wlf_linked_list_for_each(locale, &g_i18n_system->locales, link) {
		locale_array[i++] = locale->code;
	}

	*count = locale_count;
	return locale_array;
}

const struct wlf_i18n_locale *wlf_i18n_get_locale_info(const char *locale) {
	if (!g_i18n_system || !locale) {
		return NULL;
	}

	struct wlf_i18n_locale *info;
	wlf_linked_list_for_each(info, &g_i18n_system->locales, link) {
		if (strcmp(info->code, locale) == 0) {
			return info;
		}
	}

	return NULL;
}

bool wlf_i18n_is_locale_supported(const char *locale) {
	return wlf_i18n_get_locale_info(locale) != NULL;
}

enum wlf_i18n_direction wlf_i18n_get_text_direction(void) {
	const struct wlf_i18n_locale *locale_info = wlf_i18n_get_locale_info(wlf_i18n_get_locale());
	return locale_info ? locale_info->direction : WLF_I18N_DIR_LTR;
}

bool wlf_i18n_format_number(double number, char *buffer, size_t buffer_size) {
	if (!buffer || buffer_size == 0) {
		return false;
	}

	const struct wlf_i18n_locale *locale_info = wlf_i18n_get_locale_info(wlf_i18n_get_locale());
	if (!locale_info) {
		return snprintf(buffer, buffer_size, "%.2f", number) < (int)buffer_size;
	}

	// Simple number formatting - for production, consider using more sophisticated formatting
	return snprintf(buffer, buffer_size, "%.2f", number) < (int)buffer_size;
}

bool wlf_i18n_format_date(time_t timestamp, char *buffer, size_t buffer_size) {
	if (!buffer || buffer_size == 0) {
		return false;
	}

	const struct wlf_i18n_locale *locale_info = wlf_i18n_get_locale_info(wlf_i18n_get_locale());
	const char *format = locale_info && locale_info->date_format ? locale_info->date_format : "%Y-%m-%d";

	struct tm *tm_info = localtime(&timestamp);
	if (!tm_info) {
		return false;
	}

	return strftime(buffer, buffer_size, format, tm_info) > 0;
}

bool wlf_i18n_make_locale(const char *language, const char *country,
                          char *buffer, size_t buffer_size) {
	if (!language || !buffer || buffer_size == 0) {
		return false;
	}

	if (country) {
		return snprintf(buffer, buffer_size, "%s_%s", language, country) < (int)buffer_size;
	} else {
		return snprintf(buffer, buffer_size, "%s", language) < (int)buffer_size;
	}
}

bool wlf_i18n_parse_locale(const char *locale, char *language, size_t language_size,
                           char *country, size_t country_size) {
	if (!locale || !language || language_size == 0) {
		return false;
	}

	const char *underscore = strchr(locale, '_');
	if (!underscore) {
		// No country code
		strncpy(language, locale, language_size - 1);
		language[language_size - 1] = '\0';
		if (country && country_size > 0) {
			country[0] = '\0';
		}
		return true;
	}

	size_t lang_len = underscore - locale;
	if (lang_len >= language_size) {
		return false;
	}

	strncpy(language, locale, lang_len);
	language[lang_len] = '\0';

	if (country && country_size > 0) {
		strncpy(country, underscore + 1, country_size - 1);
		country[country_size - 1] = '\0';
	}

	return true;
}
