#include "wlf/platform/wlf_theme.h"
#include "wlf/config.h"

#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/linux/theme.h"
#elif WLF_HAS_MACOS_PLATFORM
#include "wlf/platform/macos/theme.h"
#elif WLF_HAS_WINDOWS_PLATFORM
#include "wlf/platform/windows/theme.h"
#endif

#include <assert.h>
#include <stdlib.h>

void wlf_theme_fill_default_palette(
		struct wlf_color palette[WLF_THEME_COLOR_COUNT],
		enum wlf_theme_appearance appearance) {
	if (appearance == WLF_THEME_APPEARANCE_DARK) {
		palette[WLF_THEME_COLOR_HIGHLIGHT] =
			(struct wlf_color){0.35, 0.67, 1.0, 1.0};
		palette[WLF_THEME_COLOR_WINDOW_BACKGROUND] =
			(struct wlf_color){0.12, 0.12, 0.13, 1.0};
		palette[WLF_THEME_COLOR_TITLEBAR_ACTIVE] =
			(struct wlf_color){0.16, 0.16, 0.17, 1.0};
		palette[WLF_THEME_COLOR_TITLEBAR_INACTIVE] =
			(struct wlf_color){0.13, 0.13, 0.14, 1.0};
		palette[WLF_THEME_COLOR_TITLEBAR_TEXT_ACTIVE] =
			(struct wlf_color){0.95, 0.95, 0.95, 1.0};
		palette[WLF_THEME_COLOR_TITLEBAR_TEXT_INACTIVE] =
			(struct wlf_color){0.68, 0.68, 0.68, 1.0};
		palette[WLF_THEME_COLOR_TITLEBAR_SEPARATOR] =
			(struct wlf_color){1.0, 1.0, 1.0, 0.12};
		return;
	}

	palette[WLF_THEME_COLOR_HIGHLIGHT] =
		(struct wlf_color){0.00, 0.40, 0.87, 1.0};
	palette[WLF_THEME_COLOR_WINDOW_BACKGROUND] =
		(struct wlf_color){0.97, 0.97, 0.97, 1.0};
	palette[WLF_THEME_COLOR_TITLEBAR_ACTIVE] =
		(struct wlf_color){0.88, 0.88, 0.87, 1.0};
	palette[WLF_THEME_COLOR_TITLEBAR_INACTIVE] =
		(struct wlf_color){0.94, 0.94, 0.95, 1.0};
	palette[WLF_THEME_COLOR_TITLEBAR_TEXT_ACTIVE] =
		(struct wlf_color){0.08, 0.08, 0.08, 1.0};
	palette[WLF_THEME_COLOR_TITLEBAR_TEXT_INACTIVE] =
		(struct wlf_color){0.25, 0.25, 0.25, 0.72};
	palette[WLF_THEME_COLOR_TITLEBAR_SEPARATOR] =
		(struct wlf_color){0.0, 0.0, 0.0, 0.14};
}

void wlf_theme_init(struct wlf_theme *theme,
		const struct wlf_theme_impl *impl) {
	assert(impl->destroy != NULL &&
		impl->name != NULL);

	*theme = (struct wlf_theme){
		.impl = impl,
	};

	wlf_signal_init(&theme->events.destroy);
	wlf_signal_init(&theme->events.theme_changed);
	wlf_signal_init(&theme->events.highlight_changed);
}

struct wlf_theme *wlf_theme_autocreate(void) {
#if WLF_HAS_LINUX_PLATFORM
	struct wlf_linux_theme *gtk_theme = wlf_linux_theme_create();
	if (gtk_theme != NULL) {
		return &gtk_theme->base;
	}
#elif WLF_HAS_MACOS_PLATFORM
	struct wlf_macos_theme *macos_theme = wlf_macos_theme_create();
	if (macos_theme == NULL) {
		return NULL;
	}

	return &macos_theme->base;
#elif WLF_HAS_WINDOWS_PLATFORM
	struct wlf_windows_theme *windows_theme = wlf_windows_theme_create();
	if (windows_theme == NULL) {
		return NULL;
	}

	return &windows_theme->base;
#endif

	return NULL;
}

void wlf_theme_destroy(struct wlf_theme *theme) {
	if (theme == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&theme->events.destroy, theme);

	assert(wlf_linked_list_empty(&theme->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&theme->events.theme_changed.listener_list));
	assert(wlf_linked_list_empty(&theme->events.highlight_changed.listener_list));

	if (theme->impl && theme->impl->destroy) {
		theme->impl->destroy(theme);
	} else {
		free(theme);
	}
}

const char *wlf_theme_appearance_name(
		enum wlf_theme_appearance appearance) {
	switch (appearance) {
	case WLF_THEME_APPEARANCE_DARK:
		return "dark";
	case WLF_THEME_APPEARANCE_LIGHT:
	default:
		return "light";
	}
}
