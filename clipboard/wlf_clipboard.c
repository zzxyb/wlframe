#include "wlf/clipboard/wlf_clipboard.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct wlf_clipboard *wlf_clipboard_create(const struct wlf_clipboard_impl *impl) {
	assert(impl);
	assert(impl->destroy);
	assert(impl->set_data);
	assert(impl->get_data);
	assert(impl->get_mime_types);
	assert(impl->clear);

	struct wlf_clipboard *clipboard = malloc(sizeof(struct wlf_clipboard));
	if (clipboard == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate wlf_clipboard");
		return NULL;
	}

	clipboard->impl = impl;
	clipboard->data = NULL;

	wlf_signal_init(&clipboard->events.destroy);
	wlf_signal_init(&clipboard->events.changed);
	wlf_signal_init(&clipboard->events.selection_changed);

	wlf_log(WLF_DEBUG, "Clipboard created");

	return clipboard;
}

void wlf_clipboard_destroy(struct wlf_clipboard *clipboard) {
	if (clipboard == NULL) {
		return;
	}

	wlf_log(WLF_DEBUG, "Destroying clipboard");

	wlf_signal_emit_mutable(&clipboard->events.destroy, NULL);
	clipboard->impl->destroy(clipboard);
	free(clipboard);
}

bool wlf_clipboard_set_data(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type,
		const void *data, size_t size) {
	assert(clipboard);
	assert(mime_type);
	assert(data || size == 0);

	if (!clipboard->impl->set_data(clipboard, mode, mime_type, data, size)) {
		wlf_log(WLF_ERROR, "Failed to set clipboard data for MIME type: %s", mime_type);
		return false;
	}

	// Emit appropriate change signal
	if (mode == WLF_CLIPBOARD_MODE_CLIPBOARD) {
		wlf_signal_emit_mutable(&clipboard->events.changed, NULL);
	} else {
		wlf_signal_emit_mutable(&clipboard->events.selection_changed, NULL);
	}

	wlf_log(WLF_DEBUG, "Clipboard data set for MIME type: %s (mode: %d, size: %zu)",
		mime_type, mode, size);

	return true;
}

void *wlf_clipboard_get_data(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type, size_t *size) {
	assert(clipboard);
	assert(mime_type);

	void *data = clipboard->impl->get_data(clipboard, mode, mime_type, size);
	if (data == NULL) {
		wlf_log(WLF_DEBUG, "No clipboard data available for MIME type: %s", mime_type);
		return NULL;
	}

	wlf_log(WLF_DEBUG, "Retrieved clipboard data for MIME type: %s (size: %zu)",
		mime_type, size ? *size : 0);

	return data;
}

bool wlf_clipboard_set_text(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *text) {
	assert(clipboard);
	assert(text);

	size_t size = strlen(text);
	return wlf_clipboard_set_data(clipboard, mode, "text/plain", text, size);
}

char *wlf_clipboard_text(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode) {
	assert(clipboard);

	size_t size;
	char *data = wlf_clipboard_get_data(clipboard, mode, "text/plain", &size);
	if (data == NULL) {
		// Try alternative text MIME types
		data = wlf_clipboard_get_data(clipboard, mode, "text/plain;charset=utf-8", &size);
		if (data == NULL) {
			data = wlf_clipboard_get_data(clipboard, mode, "TEXT", &size);
		}
		if (data == NULL) {
			data = wlf_clipboard_get_data(clipboard, mode, "STRING", &size);
		}
	}

	if (data == NULL) {
		return NULL;
	}

	// Ensure null-termination
	char *text = malloc(size + 1);
	if (text == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for clipboard text");
		free(data);
		return NULL;
	}

	memcpy(text, data, size);
	text[size] = '\0';
	free(data);

	return text;
}

char **wlf_clipboard_get_mime_types(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, size_t *count) {
	assert(clipboard);
	assert(count);

	return clipboard->impl->get_mime_types(clipboard, mode, count);
}

void wlf_clipboard_clear(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode) {
	assert(clipboard);

	clipboard->impl->clear(clipboard, mode);

	// Emit appropriate change signal
	if (mode == WLF_CLIPBOARD_MODE_CLIPBOARD) {
		wlf_signal_emit_mutable(&clipboard->events.changed, NULL);
	} else {
		wlf_signal_emit_mutable(&clipboard->events.selection_changed, NULL);
	}

	wlf_log(WLF_DEBUG, "Clipboard cleared (mode: %d)", mode);
}

bool wlf_clipboard_has_mime_type(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type) {
	assert(clipboard);
	assert(mime_type);

	size_t count;
	char **mime_types = wlf_clipboard_get_mime_types(clipboard, mode, &count);
	if (mime_types == NULL) {
		return false;
	}

	bool found = false;
	for (size_t i = 0; i < count; i++) {
		if (strcmp(mime_types[i], mime_type) == 0) {
			found = true;
			break;
		}
	}

	// Free the MIME types array
	for (size_t i = 0; i < count; i++) {
		free(mime_types[i]);
	}
	free(mime_types);

	return found;
}
