#include "wlf/platform/wlf_theme.h"
#include "wlf/types/wlf_color.h"
#include <stdio.h>

static const char *appearance_name(enum wlf_theme_appearance appearance) {
	switch (appearance) {
	case WLF_THEME_APPEARANCE_DARK:
		return "dark";
	case WLF_THEME_APPEARANCE_LIGHT:
	default:
		return "light";
	}
}

static const char *role_name(enum wlf_theme_color_role role) {
	switch (role) {
	case WLF_THEME_COLOR_WINDOW:
		return "window";
	case WLF_THEME_COLOR_WINDOW_TEXT:
		return "window_text";
	case WLF_THEME_COLOR_BASE:
		return "base";
	case WLF_THEME_COLOR_TEXT:
		return "text";
	case WLF_THEME_COLOR_BUTTON:
		return "button";
	case WLF_THEME_COLOR_ACCENT:
		return "accent";
	case WLF_THEME_COLOR_HIGHLIGHT:
		return "highlight";
	case WLF_THEME_COLOR_ERROR:
		return "error";
	default:
		return "other";
	}
}

int main(void) {
	static const enum wlf_theme_color_role roles[] = {
		WLF_THEME_COLOR_WINDOW,
		WLF_THEME_COLOR_WINDOW_TEXT,
		WLF_THEME_COLOR_BASE,
		WLF_THEME_COLOR_TEXT,
		WLF_THEME_COLOR_BUTTON,
		WLF_THEME_COLOR_ACCENT,
		WLF_THEME_COLOR_HIGHLIGHT,
		WLF_THEME_COLOR_ERROR,
	};
	struct wlf_theme *theme = wlf_theme_autocreate();
	size_t i;

	if (theme == NULL) {
		fprintf(stderr, "failed to create theme\n");
		return 1;
	}

	printf("theme implementation: %s\n", theme->impl->name);
	printf("appearance: %s\n", appearance_name(theme->appearance));

	for (i = 0; i < sizeof(roles) / sizeof(roles[0]); ++i) {
		struct wlf_color color = wlf_theme_palette_color(theme, roles[i]);

		printf("%-12s #%06X\n",
			role_name(roles[i]),
			wlf_color_to_hex_rgb(&color));
	}

	wlf_theme_destroy(theme);
	return 0;
}
