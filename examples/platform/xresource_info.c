#include "wlf/platform/x11/xresource.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_settings(struct wlf_xresource_manager *manager) {
	static const char *hints[] = { "hintnone", "hintslight", "hintmedium", "hintfull" };
	static const char *rgba[] = { "none", "rgb", "bgr", "vrgb", "vbgr" };
	static const char *filters[] = { "lcdnone", "lcddefault", "lcdlight", "lcdlegacy" };
	printf("Xft.dpi: %.3f\nXft.antialias: %s\nXft.hinting: %s\n"
		"Xft.hintstyle: %s\nXft.rgba: %s\nXft.lcdfilter: %s\n"
		"Xcursor.theme: %s\nXcursor.size: %d\nXcursor.theme_core: %s\n"
		"Explicit Xft fields: 0x%02x\n\n",
		manager->xft.dpi, manager->xft.antialias ? "true" : "false",
		manager->xft.hinting ? "true" : "false", hints[manager->xft.hintstyle],
		rgba[manager->xft.rgba], filters[manager->xft.lcdfilter],
		manager->xcursor.theme != NULL ? manager->xcursor.theme : "(default)",
		manager->xcursor.size, manager->xcursor.theme_core ? "true" : "false",
		manager->xft.present);
	fflush(stdout);
}

static void handle_changed(struct wlf_listener *listener, void *data) {
	print_settings(data);
}

int main(int argc, char **argv) {
	if (argc > 2 || (argc == 2 && strcmp(argv[1], "--watch") != 0)) {
		fprintf(stderr, "Usage: %s [--watch]\n", argv[0]);
		return EXIT_FAILURE;
	}
	wlf_log_init(WLF_INFO, NULL);
	xcb_connection_t *xcb = xcb_connect(NULL, NULL);
	struct wlf_xresource_manager *manager = wlf_xresource_manager_create(xcb);
	if (manager == NULL) {
		fprintf(stderr, "Failed to load X resources\n");
		xcb_disconnect(xcb);
		return EXIT_FAILURE;
	}
	print_settings(manager);
	struct wlf_listener changed = { .notify = handle_changed };
	wlf_signal_add(&manager->events.changed, &changed);
	if (argc == 2) {
		xcb_generic_event_t *event;
		while ((event = xcb_wait_for_event(xcb)) != NULL) {
			wlf_xresource_manager_handle_event(manager, event);
			free(event);
		}
	}
	wlf_linked_list_remove(&changed.link);
	wlf_xresource_manager_destroy(manager);
	int result = xcb_connection_has_error(xcb) ? EXIT_FAILURE : EXIT_SUCCESS;
	xcb_disconnect(xcb);
	return result;
}
