#include "wlf/utils/wlf_i18n.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Advanced translation data with context and plurals
static const char *en_advanced_translations =
"{\n"
"  \"Save|File menu\": \"Save\",\n"
"  \"Save|Dialog button\": \"Save\",\n"
"  \"Close|File menu\": \"Close\",\n"
"  \"Close|Window action\": \"Close\",\n"
"  \"Open|File menu\": \"Open\",\n"
"  \"Open|Window state\": \"Open\",\n"
"  \"file|singular\": \"file\",\n"
"  \"file|plural\": \"files\",\n"
"  \"item|singular\": \"item\",\n"
"  \"item|plural\": \"items\",\n"
"  \"message|singular\": \"message\",\n"
"  \"message|plural\": \"messages\",\n"
"  \"download_progress\": \"Downloading %s... %d%% complete\",\n"
"  \"user_info\": \"User: %s, Age: %d, Score: %.2f\",\n"
"  \"status_connected\": \"Connected to server\",\n"
"  \"status_disconnected\": \"Disconnected from server\",\n"
"  \"error_file_not_found\": \"File '%s' not found\",\n"
"  \"warning_unsaved_changes\": \"You have unsaved changes. Continue?\"\n"
"}";

static const char *zh_cn_advanced_translations =
"{\n"
"  \"Save|File menu\": \"保存\",\n"
"  \"Save|Dialog button\": \"保存\",\n"
"  \"Close|File menu\": \"关闭\",\n"
"  \"Close|Window action\": \"关闭\",\n"
"  \"Open|File menu\": \"打开\",\n"
"  \"Open|Window state\": \"已打开\",\n"
"  \"file|singular\": \"个文件\",\n"
"  \"file|plural\": \"个文件\",\n"
"  \"item|singular\": \"个项目\",\n"
"  \"item|plural\": \"个项目\",\n"
"  \"message|singular\": \"条消息\",\n"
"  \"message|plural\": \"条消息\",\n"
"  \"download_progress\": \"正在下载 %s... 完成 %d%%\",\n"
"  \"user_info\": \"用户：%s，年龄：%d，分数：%.2f\",\n"
"  \"status_connected\": \"已连接到服务器\",\n"
"  \"status_disconnected\": \"与服务器断开连接\",\n"
"  \"error_file_not_found\": \"找不到文件 '%s'\",\n"
"  \"warning_unsaved_changes\": \"您有未保存的更改。是否继续？\"\n"
"}";

static const char *ja_jp_advanced_translations =
"{\n"
"  \"Save|File menu\": \"保存\",\n"
"  \"Save|Dialog button\": \"保存\",\n"
"  \"Close|File menu\": \"閉じる\",\n"
"  \"Close|Window action\": \"閉じる\",\n"
"  \"Open|File menu\": \"開く\",\n"
"  \"Open|Window state\": \"開いている\",\n"
"  \"file|singular\": \"つのファイル\",\n"
"  \"file|plural\": \"つのファイル\",\n"
"  \"item|singular\": \"つのアイテム\",\n"
"  \"item|plural\": \"つのアイテム\",\n"
"  \"message|singular\": \"つのメッセージ\",\n"
"  \"message|plural\": \"つのメッセージ\",\n"
"  \"download_progress\": \"%s をダウンロード中... %d%% 完了\",\n"
"  \"user_info\": \"ユーザー：%s、年齢：%d、スコア：%.2f\",\n"
"  \"status_connected\": \"サーバーに接続しました\",\n"
"  \"status_disconnected\": \"サーバーから切断されました\",\n"
"  \"error_file_not_found\": \"ファイル '%s' が見つかりません\",\n"
"  \"warning_unsaved_changes\": \"保存されていない変更があります。続行しますか？\"\n"
"}";

static void test_contextual_translation(void) {
    printf("\n================== Contextual Translation Test ==================\n");

    printf("Menu contexts:\n");
    printf("  File menu Save: %s\n", wlf_trc("File menu", "Save"));
    printf("  Dialog button Save: %s\n", wlf_trc("Dialog button", "Save"));
    printf("  File menu Close: %s\n", wlf_trc("File menu", "Close"));
    printf("  Window action Close: %s\n", wlf_trc("Window action", "Close"));
    printf("  File menu Open: %s\n", wlf_trc("File menu", "Open"));
    printf("  Window state Open: %s\n", wlf_trc("Window state", "Open"));
}

static void test_plural_forms(void) {
    printf("\n================== Plural Forms Test ==================\n");

    const char *items[] = {"file", "item", "message"};
    const int counts[] = {0, 1, 2, 5, 10};

    for (int i = 0; i < 3; i++) {
        printf("\n%s plurals:\n", items[i]);
        for (int j = 0; j < 5; j++) {
            int count = counts[j];
            printf("  %d %s\n", count,
                   wlf_trp(items[i] "|singular", items[i] "|plural", count));
        }
    }
}

static void test_formatted_messages(void) {
    printf("\n================== Formatted Messages Test ==================\n");

    // Download progress test
    const char *filenames[] = {"document.pdf", "video.mp4", "archive.zip"};
    const int progress[] = {25, 67, 100};

    printf("Download progress messages:\n");
    for (int i = 0; i < 3; i++) {
        char *msg = wlf_trf("download_progress", filenames[i], progress[i]);
        if (msg) {
            printf("  %s\n", msg);
            free(msg);
        }
    }

    // User info test
    printf("\nUser information messages:\n");
    const char *users[] = {"Alice", "Bob", "Charlie"};
    const int ages[] = {25, 32, 19};
    const double scores[] = {95.5, 87.2, 92.8};

    for (int i = 0; i < 3; i++) {
        char *msg = wlf_trf("user_info", users[i], ages[i], scores[i]);
        if (msg) {
            printf("  %s\n", msg);
            free(msg);
        }
    }
}

static void test_status_messages(void) {
    printf("\n================== Status Messages Test ==================\n");

    printf("Status messages:\n");
    printf("  %s\n", wlf_tr("status_connected"));
    printf("  %s\n", wlf_tr("status_disconnected"));

    printf("\nError messages:\n");
    char *error_msg = wlf_trf("error_file_not_found", "config.txt");
    if (error_msg) {
        printf("  %s\n", error_msg);
        free(error_msg);
    }

    printf("\nWarning messages:\n");
    printf("  %s\n", wlf_tr("warning_unsaved_changes"));
}

static void test_locale_switching(void) {
    printf("\n================== Locale Switching Test ==================\n");

    const char *locales[] = {"en_US", "zh_CN", "ja_JP"};
    const char *locale_names[] = {"English", "Chinese", "Japanese"};

    printf("Testing same message in different locales:\n\n");

    for (int i = 0; i < 3; i++) {
        if (wlf_i18n_set_locale(locales[i])) {
            printf("%s (%s):\n", locale_names[i], locales[i]);
            printf("  File menu Save: %s\n", wlf_trc("File menu", "Save"));
            printf("  Status connected: %s\n", wlf_tr("status_connected"));

            char *user_msg = wlf_trf("user_info", "John", 28, 88.5);
            if (user_msg) {
                printf("  User info: %s\n", user_msg);
                free(user_msg);
            }
            printf("\n");
        }
    }
}

static void simulate_ui_app(void) {
    printf("\n================== Simulated UI Application ==================\n");

    // Simulate a file manager application
    printf("File Manager Application (Locale: %s)\n", wlf_i18n_get_locale());
    printf("------------------------------------\n");

    // Menu bar
    printf("Menu: [%s] [%s] [%s]\n",
           wlf_trc("File menu", "Open"),
           wlf_trc("File menu", "Save"),
           wlf_trc("File menu", "Close"));

    // Status bar
    printf("Status: %s\n", wlf_tr("status_connected"));

    // File listing
    printf("\nFile listing:\n");
    const int file_counts[] = {1, 5, 0};
    const char *folders[] = {"Documents", "Pictures", "Downloads"};

    for (int i = 0; i < 3; i++) {
        printf("  %s: %d %s\n", folders[i], file_counts[i],
               wlf_trp("file|singular", "file|plural", file_counts[i]));
    }

    // Dialog simulation
    printf("\nDialog: %s\n", wlf_tr("warning_unsaved_changes"));
    printf("Buttons: [%s] [%s]\n",
           wlf_trc("Dialog button", "Save"),
           wlf_tr("button_cancel"));
}

static bool setup_advanced_test_data(void) {
    // Load advanced translation data
    if (!wlf_i18n_load_json(en_advanced_translations, "en_US", "advanced")) {
        printf("Failed to load English advanced translations\n");
        return false;
    }

    if (!wlf_i18n_load_json(zh_cn_advanced_translations, "zh_CN", "advanced")) {
        printf("Failed to load Chinese advanced translations\n");
        return false;
    }

    if (!wlf_i18n_load_json(ja_jp_advanced_translations, "ja_JP", "advanced")) {
        printf("Failed to load Japanese advanced translations\n");
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    // Initialize logging
    wlf_log_init(WLF_INFO, NULL);

    printf("wlframe Advanced I18n Features Test\n");
    printf("===================================\n");

    // Initialize i18n system
    if (!wlf_i18n_init("en_US")) {
        printf("Failed to initialize i18n system\n");
        return 1;
    }

    // Setup advanced test data
    if (!setup_advanced_test_data()) {
        printf("Failed to setup advanced test data\n");
        wlf_i18n_cleanup();
        return 1;
    }

    printf("✓ Advanced translation data loaded\n");

    // Test with specific locale if provided
    if (argc > 1) {
        const char *target_locale = argv[1];
        if (wlf_i18n_set_locale(target_locale)) {
            printf("\nTesting with locale: %s\n", target_locale);
            test_contextual_translation();
            test_plural_forms();
            test_formatted_messages();
            test_status_messages();
            simulate_ui_app();
        } else {
            printf("Failed to set locale: %s\n", target_locale);
            return 1;
        }
    } else {
        // Test all features with locale switching
        test_contextual_translation();
        test_plural_forms();
        test_formatted_messages();
        test_status_messages();
        test_locale_switching();

        // Test UI simulation in different locales
        const char *locales[] = {"en_US", "zh_CN", "ja_JP"};
        for (int i = 0; i < 3; i++) {
            if (wlf_i18n_set_locale(locales[i])) {
                simulate_ui_app();
            }
        }
    }

    // Cleanup
    wlf_i18n_cleanup();
    printf("\n✓ Advanced i18n test completed successfully!\n");

    return 0;
}
