#include "wlf/platform/wlf_theme.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/types/wlf_color.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

struct wlf_backend *active_backend;
struct wlf_listener theme_destroy_listener;
struct wlf_listener theme_changed_listener;
struct wlf_listener highlight_changed_listener;

static const char *appearance_name(enum wlf_theme_appearance appearance) {
	switch (appearance) {
	case WLF_THEME_APPEARANCE_DARK:
		return "dark";
	case WLF_THEME_APPEARANCE_LIGHT:
	default:
		return "light";
	}
}

static const char *role_name(enum wlf_theme_color_role role);

static void print_theme_summary(struct wlf_theme *theme) {
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
	size_t i;

	wlf_log(WLF_INFO, "theme implementation: %s", theme->impl->name);
	wlf_log(WLF_INFO, "appearance: %s", appearance_name(theme->appearance));

	for (i = 0; i < sizeof(roles) / sizeof(roles[0]); ++i) {
		struct wlf_color color = wlf_theme_palette_color(theme, roles[i]);

		wlf_log(WLF_INFO, "%-12s #%06X",
			role_name(roles[i]),
			wlf_color_to_hex_rgb(&color));
	}
}

static void theme_destroy_notify(struct wlf_listener *listener, void *data) {
	wlf_linked_list_remove(&theme_destroy_listener.link);
	wlf_linked_list_remove(&theme_changed_listener.link);
	wlf_linked_list_remove(&highlight_changed_listener.link);
}

static void theme_changed_notify(struct wlf_listener *listener, void *data) {
	struct wlf_theme *theme = data;
	struct wlf_color highlight =
		wlf_theme_palette_color(theme, WLF_THEME_COLOR_HIGHLIGHT);
	(void)listener;

	wlf_log(WLF_INFO, "theme_changed: appearance=%s highlight=#%06X",
		appearance_name(theme->appearance),
		wlf_color_to_hex_rgb(&highlight));
}

static void highlight_changed_notify(struct wlf_listener *listener, void *data) {
	struct wlf_theme *theme = data;
	struct wlf_color highlight =
		wlf_theme_palette_color(theme, WLF_THEME_COLOR_HIGHLIGHT);
	(void)listener;

	wlf_log(WLF_INFO, "highlight_changed: highlight=#%06X",
		wlf_color_to_hex_rgb(&highlight));
}

static void handle_sigint(int signo) {
	(void)signo;

	if (active_backend != NULL) {
		wlf_backend_quit(active_backend);
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
	wlf_log_init(WLF_DEBUG, NULL);
	active_backend = wlf_backend_autocreate();
	if (active_backend == NULL) {
		wlf_log(WLF_ERROR, "Failed to auto-create backend");
		return EXIT_FAILURE;
	}

	struct wlf_theme *theme = wlf_theme_autocreate();
	if (theme == NULL) {
		wlf_log(WLF_ERROR, "Failed to auto-create theme");
		wlf_backend_destroy(active_backend);
		return EXIT_FAILURE;
	}

	theme_destroy_listener.notify = theme_destroy_notify;
	wlf_signal_add(&theme->events.destroy, &theme_destroy_listener);

	theme_changed_listener.notify = theme_changed_notify;
	wlf_signal_add(&theme->events.theme_changed, &theme_changed_listener);

	highlight_changed_listener.notify = highlight_changed_notify;
	wlf_signal_add(&theme->events.highlight_changed, &highlight_changed_listener);

	print_theme_summary(theme);

	signal(SIGINT, handle_sigint);
	wlf_log(WLF_INFO, "backend implementation: %s", active_backend->impl->name);
	wlf_log(WLF_INFO, "listening for theme changes, press Ctrl+C to quit");

	wlf_backend_exe(active_backend);
	active_backend = NULL;
	wlf_theme_destroy(theme);
	wlf_backend_destroy(active_backend);

	return EXIT_SUCCESS;
}
