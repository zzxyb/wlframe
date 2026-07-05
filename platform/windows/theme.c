#include "wlf/platform/windows/theme.h"
#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define WLF_WINDOWS_PERSONALIZE_KEY \
	L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"
#define WLF_WINDOWS_DWM_KEY \
	L"Software\\Microsoft\\Windows\\DWM"
#define WLF_WINDOWS_APPS_USE_LIGHT_THEME L"AppsUseLightTheme"
#define WLF_WINDOWS_ACCENT_COLOR L"AccentColor"

static bool windows_theme_read_dword(HKEY root, const wchar_t *subkey,
		const wchar_t *name, DWORD *value) {
	HKEY key;
	DWORD type;
	DWORD size = sizeof(*value);
	LSTATUS status;

	if (value == NULL) {
		return false;
	}

	status = RegOpenKeyExW(root, subkey, 0, KEY_QUERY_VALUE, &key);
	if (status != ERROR_SUCCESS) {
		return false;
	}

	status = RegQueryValueExW(key, name, NULL, &type, (BYTE *)value, &size);
	RegCloseKey(key);

	return status == ERROR_SUCCESS && type == REG_DWORD &&
		size == sizeof(*value);
}

static enum wlf_theme_appearance windows_theme_detect_appearance(void) {
	DWORD apps_use_light_theme;

	if (windows_theme_read_dword(HKEY_CURRENT_USER,
			WLF_WINDOWS_PERSONALIZE_KEY,
			WLF_WINDOWS_APPS_USE_LIGHT_THEME,
			&apps_use_light_theme) &&
			apps_use_light_theme == 0) {
		return WLF_THEME_APPEARANCE_DARK;
	}

	return WLF_THEME_APPEARANCE_LIGHT;
}

static struct wlf_color windows_theme_color_from_colorref(COLORREF color) {
	return (struct wlf_color){
		.r = GetRValue(color) / 255.0,
		.g = GetGValue(color) / 255.0,
		.b = GetBValue(color) / 255.0,
		.a = 1.0,
	};
}

static struct wlf_color windows_theme_color_from_abgr(DWORD color) {
	return (struct wlf_color){
		.r = (color & 0xff) / 255.0,
		.g = ((color >> 8) & 0xff) / 255.0,
		.b = ((color >> 16) & 0xff) / 255.0,
		.a = 1.0,
	};
}

static struct wlf_color windows_theme_default_accent(
		enum wlf_theme_appearance appearance) {
	if (appearance == WLF_THEME_APPEARANCE_DARK) {
		return (struct wlf_color){0.35, 0.67, 1.0, 1.0};
	}

	return (struct wlf_color){0.00, 0.47, 0.84, 1.0};
}

static void windows_theme_fill_palette(
		struct wlf_color palette[WLF_THEME_COLOR_COUNT],
		enum wlf_theme_appearance appearance) {
	DWORD accent_color;

	palette[WLF_THEME_COLOR_HIGHLIGHT] =
		windows_theme_default_accent(appearance);

	if (windows_theme_read_dword(HKEY_CURRENT_USER,
			WLF_WINDOWS_DWM_KEY,
			WLF_WINDOWS_ACCENT_COLOR,
			&accent_color)) {
		palette[WLF_THEME_COLOR_HIGHLIGHT] =
			windows_theme_color_from_abgr(accent_color);
		return;
	}

	palette[WLF_THEME_COLOR_HIGHLIGHT] =
		windows_theme_color_from_colorref(GetSysColor(COLOR_HIGHLIGHT));
}

void wlf_windows_theme_reload(struct wlf_windows_theme *theme) {
	enum wlf_theme_appearance appearance;
	struct wlf_color palette[WLF_THEME_COLOR_COUNT];
	bool appearance_changed;
	bool highlight_changed;

	if (theme == NULL) {
		return;
	}

	appearance = windows_theme_detect_appearance();
	windows_theme_fill_palette(palette, appearance);
	if (appearance == theme->base.appearance &&
			memcmp(theme->palette, palette, sizeof(palette)) == 0) {
		return;
	}

	appearance_changed = appearance != theme->base.appearance;
	highlight_changed = memcmp(
		&theme->palette[WLF_THEME_COLOR_HIGHLIGHT],
		&palette[WLF_THEME_COLOR_HIGHLIGHT],
		sizeof(struct wlf_color)) != 0;

	theme->base.appearance = appearance;
	memcpy(theme->palette, palette, sizeof(palette));

	if (appearance_changed) {
		wlf_signal_emit_mutable(&theme->base.events.theme_changed,
			&theme->base);
	}
	if (highlight_changed) {
		wlf_signal_emit_mutable(&theme->base.events.highlight_changed,
			&theme->base);
	}
}

static bool windows_theme_open_monitor_key(const wchar_t *subkey, HKEY *key) {
	return RegOpenKeyExW(HKEY_CURRENT_USER, subkey, 0, KEY_NOTIFY, key) ==
		ERROR_SUCCESS;
}

static bool windows_theme_arm_key_notification(HKEY key, HANDLE event) {
	return RegNotifyChangeKeyValue(key, FALSE, REG_NOTIFY_CHANGE_LAST_SET,
		event, TRUE) == ERROR_SUCCESS;
}

static DWORD WINAPI windows_theme_monitor_thread(void *data) {
	struct wlf_windows_theme *theme = data;
	HKEY personalize_key = NULL;
	HKEY dwm_key = NULL;
	HANDLE personalize_event = NULL;
	HANDLE dwm_event = NULL;
	HANDLE events[3];
	DWORD event_count = 1;
	bool personalize_armed = false;
	bool dwm_armed = false;

	events[0] = theme->stop_event;
	if (windows_theme_open_monitor_key(WLF_WINDOWS_PERSONALIZE_KEY,
			&personalize_key)) {
		personalize_event = CreateEventW(NULL, FALSE, FALSE, NULL);
		if (personalize_event != NULL) {
			events[event_count++] = personalize_event;
		} else {
			RegCloseKey(personalize_key);
			personalize_key = NULL;
		}
	}
	if (windows_theme_open_monitor_key(WLF_WINDOWS_DWM_KEY, &dwm_key)) {
		dwm_event = CreateEventW(NULL, FALSE, FALSE, NULL);
		if (dwm_event != NULL) {
			events[event_count++] = dwm_event;
		} else {
			RegCloseKey(dwm_key);
			dwm_key = NULL;
		}
	}

	while (WaitForSingleObject(theme->stop_event, 0) == WAIT_TIMEOUT) {
		DWORD result;

		if (personalize_key != NULL && !personalize_armed) {
			personalize_armed = windows_theme_arm_key_notification(
				personalize_key, personalize_event);
			if (!personalize_armed) {
				break;
			}
		}
		if (dwm_key != NULL && !dwm_armed) {
			dwm_armed = windows_theme_arm_key_notification(
				dwm_key, dwm_event);
			if (!dwm_armed) {
				break;
			}
		}

		result = WaitForMultipleObjects(event_count, events, FALSE, INFINITE);
		if (result == WAIT_OBJECT_0) {
			break;
		}
		if (personalize_event != NULL &&
				result == WAIT_OBJECT_0 + 1) {
			personalize_armed = false;
			wlf_windows_theme_reload(theme);
		} else if (dwm_event != NULL && result == WAIT_OBJECT_0 +
				(personalize_event != NULL ? 2 : 1)) {
			dwm_armed = false;
			wlf_windows_theme_reload(theme);
		} else {
			break;
		}
	}

	if (personalize_key != NULL) {
		RegCloseKey(personalize_key);
	}
	if (dwm_key != NULL) {
		RegCloseKey(dwm_key);
	}
	if (personalize_event != NULL) {
		CloseHandle(personalize_event);
	}
	if (dwm_event != NULL) {
		CloseHandle(dwm_event);
	}

	return 0;
}

static struct wlf_color windows_theme_palette_color(struct wlf_theme *theme,
		enum wlf_theme_color_role role) {
	struct wlf_windows_theme *windows_theme =
		wlf_windows_theme_from_theme(theme);

	if (role >= WLF_THEME_COLOR_COUNT) {
		return WLF_COLOR_TRANSPARENT;
	}

	return windows_theme->palette[role];
}

static void windows_theme_destroy(struct wlf_theme *theme) {
	struct wlf_windows_theme *windows_theme =
		wlf_windows_theme_from_theme(theme);

	if (windows_theme->stop_event != NULL) {
		SetEvent(windows_theme->stop_event);
	}
	if (windows_theme->monitor_thread != NULL) {
		WaitForSingleObject(windows_theme->monitor_thread, INFINITE);
		CloseHandle(windows_theme->monitor_thread);
		windows_theme->monitor_thread = NULL;
	}
	if (windows_theme->stop_event != NULL) {
		CloseHandle(windows_theme->stop_event);
		windows_theme->stop_event = NULL;
	}

	free(windows_theme);
}

static const struct wlf_theme_impl windows_theme_impl = {
	.name = "windows",
	.destroy = windows_theme_destroy,
	.theme_palette_color = windows_theme_palette_color,
};

struct wlf_windows_theme *wlf_windows_theme_create(void) {
	struct wlf_windows_theme *theme = calloc(1, sizeof(*theme));

	if (theme == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_windows_theme");
		return NULL;
	}

	wlf_theme_init(&theme->base, &windows_theme_impl);
	theme->base.appearance = windows_theme_detect_appearance();
	windows_theme_fill_palette(theme->palette, theme->base.appearance);

	theme->stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (theme->stop_event != NULL) {
		theme->monitor_thread = CreateThread(NULL, 0,
			windows_theme_monitor_thread, theme, 0, NULL);
	}

	return theme;
}

bool wlf_theme_is_windows(const struct wlf_theme *theme) {
	return theme != NULL && theme->impl == &windows_theme_impl;
}

struct wlf_windows_theme *wlf_windows_theme_from_theme(
		struct wlf_theme *theme) {
	assert(theme && theme->impl == &windows_theme_impl);

	struct wlf_windows_theme *windows_theme =
		wlf_container_of(theme, windows_theme, base);

	return windows_theme;
}
