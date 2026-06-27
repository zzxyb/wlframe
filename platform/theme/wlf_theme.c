#include "wlf/theme/wlf_theme.h"
#include "wlf/config.h"

#if WLF_HAS_MACOS_PLATFORM
#include "wlf/theme/macos/theme.h"
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
}

struct wlf_theme *wlf_theme_autocreate(void) {
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

	if (theme->impl && theme->impl->destroy) {
		theme->impl->destroy(theme);
	} else {
		free(theme);
	}
}

struct wlf_color wlf_theme_palette_color(struct wlf_theme *theme,
		enum wlf_theme_color_role role) {
	return theme->impl->theme_palette_color(theme, role);
}
