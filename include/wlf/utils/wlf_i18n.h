/**
 * @file        wlf_i18n.h
 * @brief       Internationalization and localization utilities for wlframe.
 * @details     This file provides a comprehensive i18n system inspired by gettext
 *              and Qt's internationalization framework. It supports message translation,
 *              locale management, pluralization rules, and runtime language switching.
 *
 *              Typical usage:
 *                  - Initialize i18n system with wlf_i18n_init()
 *                  - Load translation files with wlf_i18n_load_translation()
 *                  - Use wlf_tr() macro for translatable strings
 *                  - Switch languages with wlf_i18n_set_locale()
 *
 * @author      YaoBing Xiao
 * @date        2025-06-15
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-15, initial version\n
 */

#ifndef UTILS_WLF_I18N_H
#define UTILS_WLF_I18N_H

#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>

/**
 * @brief Maximum length for locale string (e.g., "en_US", "zh_CN").
 */
#define WLF_I18N_LOCALE_MAX_LEN 16

/**
 * @brief Maximum length for context string.
 */
#define WLF_I18N_CONTEXT_MAX_LEN 64

/**
 * @brief Maximum length for message key.
 */
#define WLF_I18N_KEY_MAX_LEN 256

/**
 * @brief Maximum length for translated message.
 */
#define WLF_I18N_MESSAGE_MAX_LEN 1024

/**
 * @brief Text direction for different languages.
 */
enum wlf_i18n_direction {
	WLF_I18N_DIR_LTR,  /**< Left-to-right */
	WLF_I18N_DIR_RTL,  /**< Right-to-left */
	WLF_I18N_DIR_AUTO, /**< Auto-detect from content */
};

/**
 * @brief Translation entry structure.
 */
struct wlf_i18n_entry {
	struct wlf_linked_list link; /**< Linked list node */
	char *key;                   /**< Original message key */
	char *context;               /**< Translation context (optional) */
	char *translation;           /**< Translated message */
	char **plurals;              /**< Plural forms (optional) */
	int plural_count;            /**< Number of plural forms */
};

/**
 * @brief Locale information structure.
 */
struct wlf_i18n_locale {
	char code[WLF_I18N_LOCALE_MAX_LEN];     /**< Locale code (e.g., "en_US") */
	char *name;                             /**< Human-readable name */
	char *native_name;                      /**< Native name of the language */
	enum wlf_i18n_direction direction;     /**< Text direction */

	/* Plural rules function pointer */
	int (*plural_rule)(int n);              /**< Function to determine plural form */
	int plural_forms;                       /**< Number of plural forms */

	/* Number and date formatting */
	char *decimal_separator;                /**< Decimal separator */
	char *thousands_separator;              /**< Thousands separator */
	char *date_format;                      /**< Date format string */
	char *time_format;                      /**< Time format string */
};

/**
 * @brief Translation catalog structure.
 */
struct wlf_i18n_catalog {
	struct wlf_linked_list link;     /**< Linked list node */
	char locale[WLF_I18N_LOCALE_MAX_LEN]; /**< Locale code */
	char *domain;                    /**< Translation domain */
	struct wlf_linked_list entries;  /**< List of translation entries */
	struct wlf_i18n_locale *locale_info; /**< Locale information */
};

/**
 * @brief Main i18n system structure.
 */
struct wlf_i18n_system {
	char current_locale[WLF_I18N_LOCALE_MAX_LEN]; /**< Current locale */
	char *default_domain;                          /**< Default translation domain */
	struct wlf_linked_list catalogs;               /**< List of translation catalogs */
	struct wlf_linked_list locales;                /**< List of available locales */

	struct {
		struct wlf_signal locale_changed; /**< Emitted when locale changes */
	} events;
};

/**
 * @brief Initialize the i18n system.
 * @param default_locale Default locale to use (e.g., "en_US").
 * @return true on success, false on failure.
 */
bool wlf_i18n_init(const char *default_locale);

/**
 * @brief Cleanup the i18n system.
 */
void wlf_i18n_cleanup(void);

/**
 * @brief Get the global i18n system instance.
 * @return Pointer to the i18n system.
 */
struct wlf_i18n_system *wlf_i18n_get_system(void);

/**
 * @brief Set the current locale.
 * @param locale Locale code to set.
 * @return true on success, false if locale is not available.
 */
bool wlf_i18n_set_locale(const char *locale);

/**
 * @brief Get the current locale.
 * @return Current locale code.
 */
const char *wlf_i18n_get_locale(void);

/**
 * @brief Load a translation file.
 * @param filename Path to the translation file.
 * @param locale Locale for this translation.
 * @param domain Translation domain (optional, can be NULL).
 * @return true on success, false on failure.
 */
bool wlf_i18n_load_translation(const char *filename, const char *locale, const char *domain);

/**
 * @brief Load translation from JSON format.
 * @param json_content JSON string containing translations.
 * @param locale Locale for this translation.
 * @param domain Translation domain (optional, can be NULL).
 * @return true on success, false on failure.
 */
bool wlf_i18n_load_json(const char *json_content, const char *locale, const char *domain);

/**
 * @brief Translate a message.
 * @param key Message key to translate.
 * @param domain Translation domain (optional, can be NULL).
 * @return Translated message or original key if translation not found.
 */
const char *wlf_i18n_translate(const char *key, const char *domain);

/**
 * @brief Translate a message with context.
 * @param context Translation context.
 * @param key Message key to translate.
 * @param domain Translation domain (optional, can be NULL).
 * @return Translated message or original key if translation not found.
 */
const char *wlf_i18n_translate_context(const char *context, const char *key, const char *domain);

/**
 * @brief Translate a message with plural forms.
 * @param key Singular form message key.
 * @param plural_key Plural form message key.
 * @param n Number to determine plural form.
 * @param domain Translation domain (optional, can be NULL).
 * @return Translated message in appropriate plural form.
 */
const char *wlf_i18n_translate_plural(const char *key, const char *plural_key, int n, const char *domain);

/**
 * @brief Format a translated message with arguments.
 * @param key Message key to translate.
 * @param domain Translation domain (optional, can be NULL).
 * @param ... Arguments for string formatting.
 * @return Formatted translated message (caller must free).
 */
char *wlf_i18n_translate_format(const char *key, const char *domain, ...);

/**
 * @brief Register a new locale.
 * @param locale Pointer to locale information.
 * @return true on success, false on failure.
 */
bool wlf_i18n_register_locale(struct wlf_i18n_locale *locale);

/**
 * @brief Get available locales.
 * @param count Pointer to store the number of locales.
 * @return Array of locale codes (caller must not free).
 */
const char **wlf_i18n_get_available_locales(int *count);

/**
 * @brief Get locale information.
 * @param locale Locale code.
 * @return Pointer to locale information or NULL if not found.
 */
const struct wlf_i18n_locale *wlf_i18n_get_locale_info(const char *locale);

/**
 * @brief Check if a locale is supported.
 * @param locale Locale code to check.
 * @return true if supported, false otherwise.
 */
bool wlf_i18n_is_locale_supported(const char *locale);

/**
 * @brief Get text direction for current locale.
 * @return Text direction enum.
 */
enum wlf_i18n_direction wlf_i18n_get_text_direction(void);

/**
 * @brief Format a number according to current locale.
 * @param number Number to format.
 * @param buffer Buffer to store formatted string.
 * @param buffer_size Size of the buffer.
 * @return true on success, false if buffer too small.
 */
bool wlf_i18n_format_number(double number, char *buffer, size_t buffer_size);

/**
 * @brief Format a date according to current locale.
 * @param timestamp Unix timestamp.
 * @param buffer Buffer to store formatted string.
 * @param buffer_size Size of the buffer.
 * @return true on success, false if buffer too small.
 */
bool wlf_i18n_format_date(time_t timestamp, char *buffer, size_t buffer_size);

/**
 * @brief Create a locale from language and country codes.
 * @param language Language code (e.g., "en", "zh").
 * @param country Country code (e.g., "US", "CN").
 * @param buffer Buffer to store the locale string.
 * @param buffer_size Size of the buffer.
 * @return true on success, false if buffer too small.
 */
bool wlf_i18n_make_locale(const char *language, const char *country,
                          char *buffer, size_t buffer_size);

/**
 * @brief Parse a locale string into components.
 * @param locale Locale string to parse.
 * @param language Buffer to store language code.
 * @param language_size Size of language buffer.
 * @param country Buffer to store country code.
 * @param country_size Size of country buffer.
 * @return true on success, false on parse error.
 */
bool wlf_i18n_parse_locale(const char *locale, char *language, size_t language_size,
                           char *country, size_t country_size);

/* Convenience macros */

/**
 * @brief Translate a message (shorthand macro).
 */
#define wlf_tr(key) wlf_i18n_translate((key), NULL)

/**
 * @brief Translate a message with context (shorthand macro).
 */
#define wlf_trc(context, key) wlf_i18n_translate_context((context), (key), NULL)

/**
 * @brief Translate a message with plural forms (shorthand macro).
 */
#define wlf_trp(key, plural_key, n) wlf_i18n_translate_plural((key), (plural_key), (n), NULL)

/**
 * @brief Translate and format a message (shorthand macro).
 */
#define wlf_trf(key, ...) wlf_i18n_translate_format((key), NULL, __VA_ARGS__)

/**
 * @brief Mark a string for translation (no-op, for extraction tools).
 */
#define wlf_tr_noop(key) (key)

#endif // UTILS_WLF_I18N_H
