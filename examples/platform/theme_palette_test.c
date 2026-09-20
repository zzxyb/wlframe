#include "wlf/platform/wlf_theme.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

struct wlf_backend *active_backend;

struct theme_state {
	struct wlf_theme *theme;
	struct wlf_listener destroy;
	struct wlf_listener changed;
	struct wlf_listener highlight_changed;
};

static const char *role_name(enum wlf_theme_color_role role);

static void print_theme_summary(struct wlf_theme *theme) {
	static const enum wlf_theme_color_role roles[] = {
		WLF_THEME_COLOR_HIGHLIGHT,
	};
	size_t i;

	wlf_log(WLF_INFO, "theme implementation: %s", theme->impl->name);
	wlf_log(WLF_INFO, "appearance: %s",
		wlf_theme_appearance_name(theme->appearance));

	for (i = 0; i < sizeof(roles) / sizeof(roles[0]); ++i) {
		struct wlf_color color = theme->palette[roles[i]];

		wlf_log(WLF_INFO, "%-12s #%06X",
			role_name(roles[i]),
			wlf_color_to_hex_rgb(&color));
	}
}

static void theme_destroy_notify(struct wlf_listener *listener, void *data) {
	WLF_UNUSED(data);
	struct theme_state *state =
		wlf_container_of(listener, state, destroy);
	wlf_linked_list_remove(&state->destroy.link);
	wlf_linked_list_remove(&state->changed.link);
	wlf_linked_list_remove(&state->highlight_changed.link);
}

static void theme_changed_notify(struct wlf_listener *listener, void *data) {
	WLF_UNUSED(data);
	struct theme_state *state =
		wlf_container_of(listener, state, changed);

	wlf_log(WLF_INFO, "theme_changed: appearance=%s",
		wlf_theme_appearance_name(state->theme->appearance));
}

static void highlight_changed_notify(struct wlf_listener *listener, void *data) {
	WLF_UNUSED(data);
	struct theme_state *state =
		wlf_container_of(listener, state, highlight_changed);
	struct wlf_color highlight =
		state->theme->palette[WLF_THEME_COLOR_HIGHLIGHT];

	wlf_log(WLF_INFO, "highlight_changed: highlight=#%06X",
		wlf_color_to_hex_rgb(&highlight));
}

static void handle_sigint(int signo) {
	WLF_UNUSED(signo);

	if (active_backend != NULL) {
		wlf_backend_quit(active_backend);
	}
}

static const char *role_name(enum wlf_theme_color_role role) {
	switch (role) {
	case WLF_THEME_COLOR_HIGHLIGHT:
		return "highlight";
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

	struct theme_state state = {
		.theme = theme,
	};
	state.destroy.notify = theme_destroy_notify;
	wlf_signal_add(&theme->events.destroy, &state.destroy);

	state.changed.notify = theme_changed_notify;
	wlf_signal_add(&theme->events.theme_changed, &state.changed);

	state.highlight_changed.notify = highlight_changed_notify;
	wlf_signal_add(&theme->events.highlight_changed,
		&state.highlight_changed);

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
