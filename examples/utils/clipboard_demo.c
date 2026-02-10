/**
 * @file        clipboard_demo.c
 * @brief       Demonstration of wlframe clipboard usage.
 * @details     This example shows how to create and use the Wayland clipboard
 *              for setting and getting text data.
 * @author      YaoBing Xiao
 * @date        2026-02-10
 */

#include "wlf/clipboard/wlf_wl_clipboard.h"
#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

// Global seat pointer (would normally be obtained from registry)
static struct wl_seat *global_seat = NULL;

// Registry listener to find seat
static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, "wl_seat") == 0 && global_seat == NULL) {
		global_seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wlf_log(WLF_INFO, "Found wl_seat");
	}
}

static void registry_handle_global_remove(void *data,
		struct wl_registry *registry, uint32_t name) {
	// Not used in this example
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

// Clipboard change notification callback
static void on_clipboard_changed(struct wlf_listener *listener, void *data) {
	wlf_log(WLF_INFO, "Clipboard content changed!");
}

int main(int argc, char *argv[]) {
	// Initialize logging
	wlf_log_init(WLF_DEBUG, NULL);

	// Create Wayland display
	wlf_log(WLF_INFO, "Connecting to Wayland display...");
	struct wlf_wl_display *display = wlf_wl_display_create();
	if (!display) {
		wlf_log(WLF_ERROR, "Failed to create Wayland display");
		return 1;
	}

	// Initialize registry
	if (!wlf_wl_display_init_registry(display)) {
		wlf_log(WLF_ERROR, "Failed to initialize registry");
		wlf_wl_display_destroy(display);
		return 1;
	}

	// Find seat
	wl_registry_add_listener(display->registry, &registry_listener, NULL);
	wl_display_roundtrip(display->base);

	if (!global_seat) {
		wlf_log(WLF_ERROR, "No wl_seat found");
		wlf_wl_display_destroy(display);
		return 1;
	}

	// Create clipboard
	wlf_log(WLF_INFO, "Creating clipboard...");
	struct wlf_wl_clipboard *wl_clipboard = wlf_wl_clipboard_create(display, global_seat);
	if (!wl_clipboard) {
		wlf_log(WLF_ERROR, "Failed to create clipboard");
		wlf_wl_display_destroy(display);
		return 1;
	}

	struct wlf_clipboard *clipboard = wlf_wl_clipboard_get_base(wl_clipboard);

	// Add listener for clipboard changes
	struct wlf_listener change_listener = {
		.notify = on_clipboard_changed,
	};
	wlf_signal_add(&clipboard->events.changed, &change_listener);

	// Example 1: Set text to clipboard
	wlf_log(WLF_INFO, "Setting text to clipboard...");
	const char *text_to_copy = "Hello from wlframe clipboard!";
	if (wlf_clipboard_set_text(clipboard, WLF_CLIPBOARD_MODE_CLIPBOARD, text_to_copy)) {
		wlf_log(WLF_INFO, "Successfully set text: %s", text_to_copy);
	} else {
		wlf_log(WLF_ERROR, "Failed to set clipboard text");
	}

	// Process events
	wl_display_roundtrip(display->base);

	// Example 2: Get text from clipboard
	wlf_log(WLF_INFO, "Getting text from clipboard...");
	char *retrieved_text = wlf_clipboard_text(clipboard, WLF_CLIPBOARD_MODE_CLIPBOARD);
	if (retrieved_text) {
		wlf_log(WLF_INFO, "Retrieved text: %s", retrieved_text);
		free(retrieved_text);
	} else {
		wlf_log(WLF_INFO, "No text in clipboard");
	}

	// Example 3: Check available MIME types
	wlf_log(WLF_INFO, "Checking available MIME types...");
	size_t count;
	char **mime_types = wlf_clipboard_get_mime_types(clipboard,
		WLF_CLIPBOARD_MODE_CLIPBOARD, &count);
	if (mime_types) {
		wlf_log(WLF_INFO, "Found %zu MIME types:", count);
		for (size_t i = 0; i < count; i++) {
			wlf_log(WLF_INFO, "  - %s", mime_types[i]);
			free(mime_types[i]);
		}
		free(mime_types);
	} else {
		wlf_log(WLF_INFO, "No MIME types available");
	}

	// Example 4: Set custom MIME type data
	wlf_log(WLF_INFO, "Setting custom MIME type data...");
	const char *custom_data = "Custom data with special MIME type";
	if (wlf_clipboard_set_data(clipboard, WLF_CLIPBOARD_MODE_CLIPBOARD,
			"application/x-wlframe-demo", custom_data, strlen(custom_data))) {
		wlf_log(WLF_INFO, "Successfully set custom data");
	} else {
		wlf_log(WLF_ERROR, "Failed to set custom data");
	}

	// Process events
	wl_display_roundtrip(display->base);

	// Example 5: Check if specific MIME type exists
	if (wlf_clipboard_has_mime_type(clipboard, WLF_CLIPBOARD_MODE_CLIPBOARD,
			"application/x-wlframe-demo")) {
		wlf_log(WLF_INFO, "Custom MIME type is available");
	}

	// Example 6: Clear clipboard
	wlf_log(WLF_INFO, "Clearing clipboard...");
	wlf_clipboard_clear(clipboard, WLF_CLIPBOARD_MODE_CLIPBOARD);

	// Process events
	wl_display_roundtrip(display->base);

	// Cleanup
	wlf_log(WLF_INFO, "Cleaning up...");
	wlf_wl_clipboard_destroy(wl_clipboard);
	wlf_wl_display_destroy(display);

	wlf_log(WLF_INFO, "Demo completed successfully");
	return 0;
}
