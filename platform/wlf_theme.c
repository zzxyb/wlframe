#include "wlf/platform/wlf_theme.h"
#include "wlf/config.h"

#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/linux/theme.h"
#elif WLF_HAS_MACOS_PLATFORM
#include "wlf/platform/macos/theme.h"
#endif

#include <assert.h>
#include <stdlib.h>

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
#endif

#if WLF_HAS_MACOS_PLATFORM
	struct wlf_macos_theme *macos_theme = wlf_macos_theme_create();
	if (macos_theme == NULL) {
		return NULL;
	}

	return &macos_theme->base;
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

struct wlf_color wlf_theme_palette_color(struct wlf_theme *theme,
		enum wlf_theme_color_role role) {
	return theme->impl->theme_palette_color(theme, role);
}
