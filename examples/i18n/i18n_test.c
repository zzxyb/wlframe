#include "wlf/utils/wlf_i18n.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Demo translation data in JSON format
static const char *en_translations =
"{\n"
"  \"hello\": \"Hello\",\n"
"  \"world\": \"World\",\n"
"  \"greeting\": \"Hello, World!\",\n"
"  \"welcome\": \"Welcome to wlframe\",\n"
"  \"file_count\": \"%d file\",\n"
"  \"file_count_plural\": \"%d files\",\n"
"  \"good_morning\": \"Good morning\",\n"
"  \"good_afternoon\": \"Good afternoon\",\n"
"  \"good_evening\": \"Good evening\",\n"
"  \"button_ok\": \"OK\",\n"
"  \"button_cancel\": \"Cancel\",\n"
"  \"menu_file\": \"File\",\n"
"  \"menu_edit\": \"Edit\",\n"
"  \"menu_help\": \"Help\"\n"
"}";

static const char *zh_cn_translations =
"{\n"
"  \"hello\": \"你好\",\n"
"  \"world\": \"世界\",\n"
"  \"greeting\": \"你好，世界！\",\n"
"  \"welcome\": \"欢迎使用 wlframe\",\n"
"  \"file_count\": \"%d 个文件\",\n"
"  \"file_count_plural\": \"%d 个文件\",\n"
"  \"good_morning\": \"早上好\",\n"
"  \"good_afternoon\": \"下午好\",\n"
"  \"good_evening\": \"晚上好\",\n"
"  \"button_ok\": \"确定\",\n"
"  \"button_cancel\": \"取消\",\n"
"  \"menu_file\": \"文件\",\n"
"  \"menu_edit\": \"编辑\",\n"
"  \"menu_help\": \"帮助\"\n"
"}";

static const char *ja_jp_translations =
"{\n"
"  \"hello\": \"こんにちは\",\n"
"  \"world\": \"世界\",\n"
"  \"greeting\": \"こんにちは、世界！\",\n"
"  \"welcome\": \"wlframeへようこそ\",\n"
"  \"file_count\": \"%d ファイル\",\n"
"  \"file_count_plural\": \"%d ファイル\",\n"
"  \"good_morning\": \"おはようございます\",\n"
"  \"good_afternoon\": \"こんにちは\",\n"
"  \"good_evening\": \"こんばんは\",\n"
"  \"button_ok\": \"OK\",\n"
"  \"button_cancel\": \"キャンセル\",\n"
"  \"menu_file\": \"ファイル\",\n"
"  \"menu_edit\": \"編集\",\n"
"  \"menu_help\": \"ヘルプ\"\n"
"}";

static const char *fr_fr_translations =
"{\n"
"  \"hello\": \"Bonjour\",\n"
"  \"world\": \"Monde\",\n"
"  \"greeting\": \"Bonjour le monde!\",\n"
"  \"welcome\": \"Bienvenue dans wlframe\",\n"
"  \"file_count\": \"%d fichier\",\n"
"  \"file_count_plural\": \"%d fichiers\",\n"
"  \"good_morning\": \"Bonjour\",\n"
"  \"good_afternoon\": \"Bon après-midi\",\n"
"  \"good_evening\": \"Bonsoir\",\n"
"  \"button_ok\": \"OK\",\n"
"  \"button_cancel\": \"Annuler\",\n"
"  \"menu_file\": \"Fichier\",\n"
"  \"menu_edit\": \"Éditer\",\n"
"  \"menu_help\": \"Aide\"\n"
"}";

static void print_separator(const char *title) {
    printf("\n================== %s ==================\n", title);
}

static void test_basic_translation(void) {
    print_separator("Basic Translation Test");

    printf("Current locale: %s\n", wlf_i18n_get_locale());
    printf("Greeting: %s\n", wlf_tr("greeting"));
    printf("Welcome: %s\n", wlf_tr("welcome"));
    printf("Hello: %s\n", wlf_tr("hello"));
    printf("World: %s\n", wlf_tr("world"));
}

static void test_ui_elements(void) {
    print_separator("UI Elements Translation");

    printf("Buttons:\n");
    printf("  [%s] [%s]\n", wlf_tr("button_ok"), wlf_tr("button_cancel"));

    printf("Menu:\n");
    printf("  %s | %s | %s\n",
           wlf_tr("menu_file"),
           wlf_tr("menu_edit"),
           wlf_tr("menu_help"));
}

static void test_time_greetings(void) {
    print_separator("Time-based Greetings");

    printf("Morning: %s\n", wlf_tr("good_morning"));
    printf("Afternoon: %s\n", wlf_tr("good_afternoon"));
    printf("Evening: %s\n", wlf_tr("good_evening"));
}

static void test_formatted_messages(void) {
    print_separator("Formatted Messages");

    for (int i = 0; i <= 3; i++) {
        char *msg = wlf_trf("file_count", i);
        if (msg) {
            printf("Count %d: %s\n", i, msg);
            free(msg);
        }
    }
}

static void test_locale_info(void) {
    print_separator("Locale Information");

    const struct wlf_i18n_locale *locale_info = wlf_i18n_get_locale_info(wlf_i18n_get_locale());
    if (locale_info) {
        printf("Locale: %s\n", locale_info->code);
        printf("Name: %s\n", locale_info->name ? locale_info->name : "N/A");
        printf("Native name: %s\n", locale_info->native_name ? locale_info->native_name : "N/A");
        printf("Text direction: %s\n",
               locale_info->direction == WLF_I18N_DIR_LTR ? "LTR" :
               locale_info->direction == WLF_I18N_DIR_RTL ? "RTL" : "AUTO");
    }
}

static void test_all_languages(void) {
    const char *locales[] = {"en_US", "zh_CN", "ja_JP", "fr_FR"};
    const char *locale_names[] = {"English (US)", "Chinese (Simplified)", "Japanese", "French"};

    for (int i = 0; i < 4; i++) {
        printf("\n" "======================================\n");
        printf("Testing locale: %s (%s)\n", locales[i], locale_names[i]);
        printf("======================================\n");

        if (wlf_i18n_set_locale(locales[i])) {
            test_basic_translation();
            test_ui_elements();
            test_time_greetings();
            test_formatted_messages();
            test_locale_info();
        } else {
            printf("Failed to set locale: %s\n", locales[i]);
        }
    }
}

static bool setup_test_data(void) {
    // Load English translations
    if (!wlf_i18n_load_json(en_translations, "en_US", "test")) {
        printf("Failed to load English translations\n");
        return false;
    }

    // Load Chinese translations
    if (!wlf_i18n_load_json(zh_cn_translations, "zh_CN", "test")) {
        printf("Failed to load Chinese translations\n");
        return false;
    }

    // Load Japanese translations
    if (!wlf_i18n_load_json(ja_jp_translations, "ja_JP", "test")) {
        printf("Failed to load Japanese translations\n");
        return false;
    }

    // Load French translations
    if (!wlf_i18n_load_json(fr_fr_translations, "fr_FR", "test")) {
        printf("Failed to load French translations\n");
        return false;
    }

    return true;
}

static void print_available_locales(void) {
    print_separator("Available Locales");

    int count;
    const char **locales = wlf_i18n_get_available_locales(&count);

    printf("Available locales (%d):\n", count);
    for (int i = 0; i < count; i++) {
        const struct wlf_i18n_locale *info = wlf_i18n_get_locale_info(locales[i]);
        printf("  %s", locales[i]);
        if (info && info->name) {
            printf(" - %s", info->name);
        }
        if (info && info->native_name) {
            printf(" (%s)", info->native_name);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    // Initialize logging
    wlf_log_init(WLF_DEBUG, NULL);

    printf("wlframe Internationalization Test\n");
    printf("==================================\n");

    // Initialize i18n system
    if (!wlf_i18n_init("en_US")) {
        printf("Failed to initialize i18n system\n");
        return 1;
    }

    printf("✓ I18n system initialized\n");

    // Setup test translation data
    if (!setup_test_data()) {
        printf("Failed to setup test data\n");
        wlf_i18n_cleanup();
        return 1;
    }

    printf("✓ Test translation data loaded\n");

    // Test specific locale if provided
    if (argc > 1) {
        const char *target_locale = argv[1];
        printf("\nTesting specific locale: %s\n", target_locale);

        if (wlf_i18n_set_locale(target_locale)) {
            test_basic_translation();
            test_ui_elements();
            test_time_greetings();
            test_formatted_messages();
            test_locale_info();
        } else {
            printf("Failed to set locale: %s\n", target_locale);
            printf("Available locales:\n");
            int count;
            const char **locales = wlf_i18n_get_available_locales(&count);
            for (int i = 0; i < count; i++) {
                printf("  %s\n", locales[i]);
            }
        }
    } else {
        // Test all available locales
        print_available_locales();
        test_all_languages();
    }

    // Test some utility functions
    print_separator("Utility Functions Test");

    char locale_buffer[32];
    if (wlf_i18n_make_locale("zh", "CN", locale_buffer, sizeof(locale_buffer))) {
        printf("Made locale from 'zh' + 'CN': %s\n", locale_buffer);
    }

    char language[8], country[8];
    if (wlf_i18n_parse_locale("fr_FR", language, sizeof(language), country, sizeof(country))) {
        printf("Parsed 'fr_FR': language='%s', country='%s'\n", language, country);
    }

    printf("\nText direction for current locale: %s\n",
           wlf_i18n_get_text_direction() == WLF_I18N_DIR_LTR ? "LTR" :
           wlf_i18n_get_text_direction() == WLF_I18N_DIR_RTL ? "RTL" : "AUTO");

    // Cleanup
    wlf_i18n_cleanup();
    printf("\n✓ I18n system cleaned up\n");
    printf("\nTest completed successfully!\n");

    return 0;
}
