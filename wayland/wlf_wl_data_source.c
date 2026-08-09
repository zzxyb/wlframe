#include "wlf/wayland/wlf_wl_data_source.h"
#include "wlf/wayland/wlf_wl_data_device_manager.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void data_source_handle_target(void *data,
		struct wl_data_source *base, const char *mime_type) {
	(void)base;
	struct wlf_wl_data_source *source = data;
	struct wlf_wl_data_source_target_event event = {
		.source = source,
		.mime_type = mime_type,
	};
	wlf_signal_emit_mutable(&source->events.target, &event);
}

static void data_source_handle_send(void *data,
		struct wl_data_source *base, const char *mime_type, int32_t fd) {
	(void)base;
	struct wlf_wl_data_source *source = data;
	struct wlf_wl_data_source_send_event event = {
		.source = source,
		.mime_type = mime_type,
		.fd = fd,
	};
	wlf_signal_emit_mutable(&source->events.send, &event);
}

static void data_source_handle_cancelled(void *data,
		struct wl_data_source *base) {
	(void)base;
	struct wlf_wl_data_source *source = data;
	wlf_signal_emit_mutable(&source->events.cancelled, source);
}

static void data_source_handle_dnd_drop_performed(void *data,
		struct wl_data_source *base) {
	(void)base;
	struct wlf_wl_data_source *source = data;
	wlf_signal_emit_mutable(&source->events.dnd_drop_performed, source);
}

static void data_source_handle_dnd_finished(void *data,
		struct wl_data_source *base) {
	(void)base;
	struct wlf_wl_data_source *source = data;
	wlf_signal_emit_mutable(&source->events.dnd_finished, source);
}

static void data_source_handle_action(void *data,
		struct wl_data_source *base, uint32_t dnd_action) {
	(void)base;
	struct wlf_wl_data_source *source = data;
	struct wlf_wl_data_source_action_event event = {
		.source = source,
		.dnd_action = dnd_action,
	};
	wlf_signal_emit_mutable(&source->events.action, &event);
}

static const struct wl_data_source_listener wl_data_source_listener = {
	.target = data_source_handle_target,
	.send = data_source_handle_send,
	.cancelled = data_source_handle_cancelled,
	.dnd_drop_performed = data_source_handle_dnd_drop_performed,
	.dnd_finished = data_source_handle_dnd_finished,
	.action = data_source_handle_action,
};

struct wlf_wl_data_source *wlf_wl_data_source_create(
		struct wlf_wl_data_device_manager *manager) {
	assert(manager != NULL);

	struct wlf_wl_data_source *source = calloc(1, sizeof(*source));
	if (source == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_data_source");
		return NULL;
	}

	source->wl_data_source = wl_data_device_manager_create_data_source(
		manager->wl_data_device_manager);
	if (source->wl_data_source == NULL) {
		wlf_log(WLF_ERROR, "wl_data_device_manager_create_data_source failed");
		free(source);
		return NULL;
	}

	wlf_signal_init(&source->events.destroy);
	wlf_signal_init(&source->events.target);
	wlf_signal_init(&source->events.send);
	wlf_signal_init(&source->events.cancelled);
	wlf_signal_init(&source->events.dnd_drop_performed);
	wlf_signal_init(&source->events.dnd_finished);
	wlf_signal_init(&source->events.action);

	wl_data_source_add_listener(source->wl_data_source,
		&wl_data_source_listener, source);

	return source;
}

void wlf_wl_data_source_destroy(struct wlf_wl_data_source *source) {
	if (source == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&source->events.destroy, source);
	wl_data_source_destroy(source->wl_data_source);
	free(source);
}

void wlf_wl_data_source_offer(struct wlf_wl_data_source *source,
		const char *mime_type) {
	assert(source != NULL);
	wl_data_source_offer(source->wl_data_source, mime_type);
}

void wlf_wl_data_source_set_actions(struct wlf_wl_data_source *source,
		uint32_t dnd_actions) {
	assert(source != NULL);
	wl_data_source_set_actions(source->wl_data_source, dnd_actions);
}
