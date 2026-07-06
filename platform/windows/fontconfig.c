#include "wlf/platform/windows/fontconfig.h"

#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static void windows_fontconfig_set_family(struct wlf_fontconfig *config,
		enum wlf_font_role role, size_t index, const char *family) {
	assert(config != NULL);
	assert(role >= 0 && role < WLF_FONT_ROLE_COUNT);
	assert(index < WLF_FONTCONFIG_MAX_FALLBACKS);
	assert(family != NULL);

	strncpy(config->families[role][index], family,
		WLF_FONTCONFIG_FAMILY_NAME_MAX - 1);
	config->families[role][index][WLF_FONTCONFIG_FAMILY_NAME_MAX - 1] = '\0';

	if (index >= config->counts[role]) {
		config->counts[role] = index + 1;
	}
}

static void windows_fontconfig_set_role_families(struct wlf_fontconfig *config,
		enum wlf_font_role role, const char *family0, const char *family1,
		const char *family2, const char *family3) {
	config->counts[role] = 0;
	if (family0 != NULL) {
		windows_fontconfig_set_family(config, role,
			config->counts[role], family0);
	}
	if (family1 != NULL) {
		windows_fontconfig_set_family(config, role,
			config->counts[role], family1);
	}
	if (family2 != NULL) {
		windows_fontconfig_set_family(config, role,
			config->counts[role], family2);
	}
	if (family3 != NULL) {
		windows_fontconfig_set_family(config, role,
			config->counts[role], family3);
	}
}

static void windows_fontconfig_apply_env_override(struct wlf_fontconfig *config,
		enum wlf_font_role role, const char *env_name) {
	const char *override = wlf_get_env(env_name);
	if (override == NULL || override[0] == '\0') {
		return;
	}

	config->counts[role] = 0;
	windows_fontconfig_set_family(config, role, 0, override);
}

static bool windows_fontconfig_wide_to_utf8(const wchar_t *wide,
		char *buffer, size_t size) {
	int written;

	if (wide == NULL || buffer == NULL || size == 0 || wide[0] == L'\0') {
		return false;
	}

	written = WideCharToMultiByte(CP_UTF8, 0, wide, -1, buffer, (int)size,
		NULL, NULL);
	if (written <= 0) {
		buffer[0] = '\0';
		return false;
	}

	buffer[size - 1] = '\0';
	return true;
}

static bool windows_fontconfig_system_ui_family(char *buffer, size_t size) {
	NONCLIENTMETRICSW metrics = {0};

	metrics.cbSize = sizeof(metrics);
	if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics),
			&metrics, 0)) {
		return false;
	}

	return windows_fontconfig_wide_to_utf8(metrics.lfMessageFont.lfFaceName,
		buffer, size);
}

static void windows_fontconfig_fill_defaults(struct wlf_fontconfig *config) {
	char ui_family[WLF_FONTCONFIG_FAMILY_NAME_MAX] = "Segoe UI";
	const char *ui = ui_family;

	(void)windows_fontconfig_system_ui_family(ui_family, sizeof(ui_family));

	windows_fontconfig_set_role_families(config, WLF_FONT_ROLE_UI,
		ui, "Microsoft YaHei UI", "Segoe UI Emoji", NULL);
	windows_fontconfig_set_role_families(config, WLF_FONT_ROLE_SANS_SERIF,
		"Arial", ui, "Microsoft YaHei", NULL);
	windows_fontconfig_set_role_families(config, WLF_FONT_ROLE_SERIF,
		"Times New Roman", "SimSun", "Songti SC", NULL);
	windows_fontconfig_set_role_families(config, WLF_FONT_ROLE_MONOSPACE,
		"Consolas", "Courier New", "Microsoft YaHei Mono", NULL);
	windows_fontconfig_set_role_families(config, WLF_FONT_ROLE_ROUNDED,
		"Segoe UI", "Arial Rounded MT Bold", ui, NULL);
	windows_fontconfig_set_role_families(config, WLF_FONT_ROLE_EMOJI,
		"Segoe UI Emoji", "Segoe UI Symbol", ui, NULL);
	windows_fontconfig_set_role_families(config, WLF_FONT_ROLE_TITLE,
		"Segoe UI Variable Display", "Segoe UI", ui, NULL);

	windows_fontconfig_apply_env_override(config, WLF_FONT_ROLE_UI,
		"WLF_FONT_UI");
	windows_fontconfig_apply_env_override(config, WLF_FONT_ROLE_SANS_SERIF,
		"WLF_FONT_SANS");
	windows_fontconfig_apply_env_override(config, WLF_FONT_ROLE_SERIF,
		"WLF_FONT_SERIF");
	windows_fontconfig_apply_env_override(config, WLF_FONT_ROLE_MONOSPACE,
		"WLF_FONT_MONO");
	windows_fontconfig_apply_env_override(config, WLF_FONT_ROLE_ROUNDED,
		"WLF_FONT_ROUNDED");
	windows_fontconfig_apply_env_override(config, WLF_FONT_ROLE_EMOJI,
		"WLF_FONT_EMOJI");
	windows_fontconfig_apply_env_override(config, WLF_FONT_ROLE_TITLE,
		"WLF_FONT_TITLE");
}

static void windows_fontconfig_destroy(struct wlf_fontconfig *config) {
	free(config);
}

static const struct wlf_fontconfig_impl windows_fontconfig_impl = {
	.name = "windows",
	.destroy = windows_fontconfig_destroy,
};

struct wlf_windows_fontconfig *wlf_windows_fontconfig_create(void) {
	struct wlf_windows_fontconfig *config = calloc(1, sizeof(*config));
	if (config == NULL) {
		return NULL;
	}

	wlf_fontconfig_init(&config->base, &windows_fontconfig_impl);
	windows_fontconfig_fill_defaults(&config->base);

	return config;
}

bool wlf_fontconfig_is_windows(const struct wlf_fontconfig *config) {
	return config != NULL &&
		config->impl != NULL &&
		config->impl == &windows_fontconfig_impl;
}

struct wlf_windows_fontconfig *wlf_windows_fontconfig_from_fontconfig(
		struct wlf_fontconfig *config) {
	assert(config && config->impl == &windows_fontconfig_impl);

	struct wlf_windows_fontconfig *windows_fontconfig =
		wlf_container_of(config, windows_fontconfig, base);

	return windows_fontconfig;
}
