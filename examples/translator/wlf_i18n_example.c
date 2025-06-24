/**
 * @file        wlf_i18n_example.c
 * @brief       Example demonstrating the wlf_i18n internationalization system.
 * @details     This example shows how to use the wlf_i18n system for
 *              multi-language support in C applications.
 * @author      wlframe team
 * @date        2024-12-17
 * @version     v1.0
 */

#include "wlf/utils/wlf_log.h"
#include "wlf/translator/wlf_i18n.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Demonstrate singular translations
 */
static void demo_singular_translations(void)
{
    printf("\n=== Singular Translations ===\n");

    printf("Key: 'hello' -> Translation: '%s'\n", _("hello"));
    printf("Key: 'goodbye' -> Translation: '%s'\n", _("goodbye"));
    printf("Key: 'file' -> Translation: '%s'\n", _("file"));
    printf("Key: 'settings' -> Translation: '%s'\n", _("settings"));
    printf("Key: 'error' -> Translation: '%s'\n", _("error"));
    printf("Key: 'warning' -> Translation: '%s'\n", _("warning"));
    printf("Key: 'info' -> Translation: '%s'\n", _("info"));
    printf("Key: 'success' -> Translation: '%s'\n", _("success"));

    // Test non-existing key (should return the key itself)
    printf("Key: 'non_existing' -> Translation: '%s'\n", _("non_existing"));
}

/**
 * @brief Demonstrate plural translations
 */
static void demo_plural_translations(void)
{
    printf("\n=== Plural Translations ===\n");

    // Test file count with different numbers
    for(int i = 0; i <= 5; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), _p("file_count", i), i);
        printf("Files (%d): %s\n", i, buffer);
    }

    printf("\n");

    // Test item count with different numbers
    for(int i = 0; i <= 5; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), _p("item_count", i), i);
        printf("Items (%d): %s\n", i, buffer);
    }
}

/**
 * @brief Demonstrate i18n system information
 */
static void demo_system_info(void)
{
    printf("\n=== System Information ===\n");

    printf("Is initialized: %s\n", wlf_i18n_is_initialized() ? "Yes" : "No");
    printf("Current locale: %s\n", wlf_i18n_get_current_locale());
    printf("Available locales: %d\n", wlf_i18n_get_locale_count());

    // List all available locales
    printf("Available locales:\n");
    for(int i = 0; i < wlf_i18n_get_locale_count(); i++) {
        const char* locale = wlf_i18n_get_locale_by_index(i);
        printf("  [%d] %s\n", i, locale);
    }
}

/**
 * @brief Demonstrate locale switching
 */
static void demo_locale_switching(void)
{
    printf("\n=== Locale Switching Demo ===\n");

    // Test with English (default)
    printf("\n--- English (en-US) ---\n");
    wlf_i18n_set_locale("en-US");
    demo_singular_translations();
    demo_plural_translations();

    // Test with Chinese
    printf("\n--- Chinese (zh-CN) ---\n");
    if(wlf_i18n_set_locale("zh-CN") == 0) {
        demo_singular_translations();
        demo_plural_translations();
    } else {
        printf("Failed to set Chinese locale\n");
    }

    // Test with non-existing locale
    printf("\n--- Non-existing locale (should fail) ---\n");
    if(wlf_i18n_set_locale("xx-XX") != 0) {
        printf("Successfully failed to set non-existing locale 'xx-XX'\n");
        printf("Current locale remains: %s\n", wlf_i18n_get_current_locale());
    }

    // Switch back to English
    wlf_i18n_set_locale("en-US");
}

/**
 * @brief Demonstrate error handling
 */
static void demo_error_handling(void)
{
    printf("\n=== Error Handling Demo ===\n");

    // Test before initialization
    wlf_i18n_reset();
    printf("Before init - Current locale: %s\n",
           wlf_i18n_get_current_locale() ? wlf_i18n_get_current_locale() : "NULL");
    printf("Before init - Is initialized: %s\n", wlf_i18n_is_initialized() ? "Yes" : "No");
    printf("Before init - Set locale result: %d\n", wlf_i18n_set_locale("en-US"));

    // Reinitialize for rest of demo
    wlf_i18n_init_default();
}

/**
 * @brief Demonstrate practical usage scenarios
 */
static void demo_practical_usage(void)
{
    printf("\n=== Practical Usage Examples ===\n");

    // Simulate a file operation with different results
    const char* operations[] = {"copy", "move", "delete"};
    int file_counts[] = {0, 1, 3, 10};

    for(int op = 0; op < 3; op++) {
        for(int fc = 0; fc < 4; fc++) {
            char msg[512];
            if(file_counts[fc] == 0) {
                snprintf(msg, sizeof(msg), "%s operation completed. No files processed.",
                        operations[op]);
            } else {
                char file_msg[256];
                snprintf(file_msg, sizeof(file_msg), _p("file_count", file_counts[fc]), file_counts[fc]);
                snprintf(msg, sizeof(msg), "%s operation completed. Processed %s.",
                        operations[op], file_msg);
            }
            printf("Operation: %s\n", msg);
        }
        printf("\n");
    }
}

int main(void)
{
    printf("wlframe Internationalization (i18n) System Demo\n");
    printf("================================================\n");

    // Initialize the i18n system
    if(wlf_i18n_init_default() != 0) {
        printf("Failed to initialize i18n system\n");
        return 1;
    }

    // Show system information
    demo_system_info();

    // Demonstrate locale switching
    demo_locale_switching();

    // Demonstrate error handling
    demo_error_handling();

    // Demonstrate practical usage
    demo_practical_usage();

    printf("\n=== Demo Complete ===\n");
    printf("The wlf_i18n system provides:\n");
    printf("- Easy-to-use macros: _() and _p()\n");
    printf("- Automatic fallback to default locale\n");
    printf("- Support for plural forms\n");
    printf("- Runtime locale switching\n");
    printf("- Error handling and validation\n");
    printf("- Extensible language pack system\n");

    return 0;
}
