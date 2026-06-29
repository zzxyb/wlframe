#include "wlf/wayland/wlf_wl_buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void buffer_handle_release(void *data, struct wl_buffer *base) {
	(void)base;
	struct wlf_wl_buffer *buffer = data;
	wlf_signal_emit_mutable(&buffer->events.release, buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = buffer_handle_release,
};

struct wlf_wl_buffer *wlf_wl_buffer_wrap(struct wl_buffer *wl_buffer) {
	assert(wl_buffer != NULL);

	struct wlf_wl_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_buffer");
		return NULL;
	}

	buffer->wl_buffer = wl_buffer;

	wlf_signal_init(&buffer->events.destroy);
	wlf_signal_init(&buffer->events.release);

	wl_buffer_add_listener(buffer->wl_buffer, &wl_buffer_listener, buffer);

	return buffer;
}

void wlf_wl_buffer_destroy(struct wlf_wl_buffer *buffer) {
	if (buffer == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&buffer->events.destroy, buffer);
	wl_buffer_destroy(buffer->wl_buffer);
	free(buffer);
}
