#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/translator/wlf_i18n.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const struct wlf_i18n_lang * const * current_lang_pack = NULL;
static const struct wlf_i18n_lang * current_lang = NULL;

static inline uint32_t op_n(int32_t val) { return (uint32_t)(val < 0 ? -val : val); }
static inline uint32_t op_i(uint32_t val) { return val; }

static uint8_t en_plural_fn(int32_t num)
{
    uint32_t n = op_n(num);
    uint32_t i = op_i(n);

    if (i == 1) {
        return WLF_I18N_PLURAL_TYPE_ONE;
    }

    return WLF_I18N_PLURAL_TYPE_OTHER;
}

static uint8_t zh_plural_fn(int32_t num)
{
    WLF_UNUSED(num);
    return WLF_I18N_PLURAL_TYPE_OTHER;
}

static const char * en_us_singulars[] = {
    "Hello",                    // 0="hello"
    "Goodbye",                  // 1="goodbye"
    "File",                     // 2="file"
    "Settings",                 // 3="settings"
    "Error",                    // 4="error"
    "Warning",                  // 5="warning"
    "Information",              // 6="info"
    "Success",                  // 7="success"
    NULL                        // End marker
};

static const char * en_us_plurals_one[] = {
    "%d file",                  // 0="file_count"
    "%d item",                  // 1="item_count"
    NULL                        // End marker
};

static const char * en_us_plurals_other[] = {
    "%d files",                 // 0="file_count"
    "%d items",                 // 1="item_count"
    NULL                        // End marker
};

static const struct wlf_i18n_lang en_us_lang = {
    .locale_name = "en-US",
    .singulars = en_us_singulars,
    .plurals[WLF_I18N_PLURAL_TYPE_ONE] = en_us_plurals_one,
    .plurals[WLF_I18N_PLURAL_TYPE_OTHER] = en_us_plurals_other,
    .locale_plural_fn = en_plural_fn
};

static const char * zh_cn_singulars[] = {
    "你好",                     // 0="hello"
    "再见",                     // 1="goodbye"
    "文件",                     // 2="file"
    "设置",                     // 3="settings"
    "错误",                     // 4="error"
    "警告",                     // 5="warning"
    "信息",                     // 6="info"
    "成功",                     // 7="success"
    NULL                        // End marker
};

static const char * zh_cn_plurals_other[] = {
    "%d 个文件",                // 0="file_count"
    "%d 个项目",                // 1="item_count"
    NULL                        // End marker
};

static const struct wlf_i18n_lang zh_cn_lang = {
    .locale_name = "zh-CN",
    .singulars = zh_cn_singulars,
    .plurals[WLF_I18N_PLURAL_TYPE_OTHER] = zh_cn_plurals_other,
    .locale_plural_fn = zh_plural_fn
};

const struct wlf_i18n_lang * wlf_i18n_language_pack[] = {
    &en_us_lang,                // Default language (first in array)
    &zh_cn_lang,
    NULL                        // End marker
};

#ifndef WLF_I18N_OPTIMIZE
static const char * singular_idx[] = {
    "hello",
    "goodbye",
    "file",
    "settings",
    "error",
    "warning",
    "info",
    "success",
    NULL
};

static const char * plural_idx[] = {
    "file_count",
    "item_count",
    NULL
};
#endif

const char * wlf_i18n_get_singular_by_idx(const char *msg_id, int msg_index)
{
    if(current_lang == NULL || msg_index == WLF_I18N_ID_NOT_FOUND) return msg_id;

    const struct wlf_i18n_lang * lang = current_lang;
    const char * txt;

    // Search in current locale
    if(lang->singulars != NULL) {
        txt = lang->singulars[msg_index];
        if (txt != NULL) return txt;
    }

    // Try to fallback to default locale
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->singulars != NULL) {
        txt = lang->singulars[msg_index];
        if (txt != NULL) return txt;
    }

    return msg_id;
}

const char * wlf_i18n_get_plural_by_idx(const char * msg_id, int msg_index, int32_t num)
{
    if(current_lang == NULL || msg_index == WLF_I18N_ID_NOT_FOUND) return msg_id;

    const struct wlf_i18n_lang * lang = current_lang;
    const char * txt;
    enum wlf_i18n_plural_type ptype;

    // Search in current locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = lang->plurals[ptype][msg_index];
            if (txt != NULL) return txt;
        }
    }

    // Try to fallback to default locale
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = lang->plurals[ptype][msg_index];
            if (txt != NULL) return txt;
        }
    }

    return msg_id;
}

#ifndef WLF_I18N_OPTIMIZE

static int wlf_i18n_get_id(const char * phrase, const char * * list)
{
    if (phrase == NULL || list == NULL) return WLF_I18N_ID_NOT_FOUND;

    for(int i = 0; list[i] != NULL; i++) {
        if(strcmp(list[i], phrase) == 0) return i;
    }
    return WLF_I18N_ID_NOT_FOUND;
}

int wlf_i18n_get_singular_id(const char * phrase)
{
    return wlf_i18n_get_id(phrase, singular_idx);
}

int wlf_i18n_get_plural_id(const char * phrase)
{
    return wlf_i18n_get_id(phrase, plural_idx);
}
#endif

int wlf_i18n_init(const struct wlf_i18n_lang * const * langs)
{
    if(langs == NULL) {
        wlf_log(WLF_ERROR, "wlf_i18n_init: langs parameter is NULL");
        return -1;
    }

    if(langs[0] == NULL) {
        wlf_log(WLF_ERROR, "wlf_i18n_init: language pack is empty");
        return -1;
    }

    current_lang_pack = langs;
    current_lang = langs[0];     // Automatically select the first language as default

    wlf_log(WLF_INFO, "wlf_i18n initialized with default locale: %s", current_lang->locale_name);
    return 0;
}

int wlf_i18n_init_default(void)
{
    return wlf_i18n_init(wlf_i18n_language_pack);
}

int wlf_i18n_set_locale(const char * l_name)
{
    if(current_lang_pack == NULL) {
        wlf_log(WLF_ERROR, "wlf_i18n_set_locale: i18n system not initialized");
        return -1;
    }

    if(l_name == NULL) {
        wlf_log(WLF_ERROR, "wlf_i18n_set_locale: locale name is NULL");
        return -1;
    }

    for(int i = 0; current_lang_pack[i] != NULL; i++) {
        if(strcmp(current_lang_pack[i]->locale_name, l_name) == 0) {
            current_lang = current_lang_pack[i];
            wlf_log(WLF_INFO, "wlf_i18n locale changed to: %s", l_name);
            return 0;
        }
    }

    wlf_log(WLF_INFO, "wlf_i18n_set_locale: locale '%s' not found", l_name);
    return -1;
}

const char * wlf_i18n_get_current_locale(void)
{
    if(!current_lang) return NULL;
    return current_lang->locale_name;
}

bool wlf_i18n_is_initialized(void)
{
    return (current_lang_pack != NULL && current_lang != NULL);
}

int wlf_i18n_get_locale_count(void)
{
    if(current_lang_pack == NULL) return 0;

    int count = 0;
    while(current_lang_pack[count] != NULL) {
        count++;
    }
    return count;
}

const char * wlf_i18n_get_locale_by_index(int index)
{
    if(current_lang_pack == NULL || index < 0) return NULL;

    int count = 0;
    while(current_lang_pack[count] != NULL) {
        if(count == index) {
            return current_lang_pack[count]->locale_name;
        }
        count++;
    }
    return NULL;
}

void wlf_i18n_reset(void)
{
    current_lang_pack = NULL;
    current_lang = NULL;
    wlf_log(WLF_INFO, "wlf_i18n system reset");
}
