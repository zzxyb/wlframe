#include "wlf/types/wlf_muti_backend.h"
#include "wlf/util/wlf_log.h"
#include "wlf/types/wlf_buffer.h"

#include <stdlib.h>
#include <assert.h>

#include <wayland-server-core.h>

struct subbackend_state {
	struct wlf_backend *backend;
	struct wlf_backend *container;
	struct wl_listener new_input;
	struct wl_listener new_output;
	struct wl_listener destroy;
	struct wl_list link;
};

static struct wlf_multi_backend *multi_backend_from_backend(
		struct wlf_backend *wlf_backend) {
	assert(wlf_backend_is_multi(wlf_backend));
	struct wlf_multi_backend *backend = wl_container_of(wlf_backend, backend, backend);
	return backend;
}

static bool multi_backend_start(struct wlf_backend *wlf_backend) {
	struct wlf_multi_backend *backend = multi_backend_from_backend(wlf_backend);
	struct subbackend_state *sub;
	wl_list_for_each(sub, &backend->backends, link) {
		if (!wlf_backend_start(sub->backend)) {
			wlf_log(WLF_ERROR, "Failed to initialize backend.");
			return false;
		}
	}
	return true;
}

static void subbackend_state_destroy(struct subbackend_state *sub) {
	wl_list_remove(&sub->new_input.link);
	wl_list_remove(&sub->new_output.link);
	wl_list_remove(&sub->destroy.link);
	wl_list_remove(&sub->link);
	free(sub);
}

static void multi_backend_destroy(struct wlf_backend *wlf_backend) {
	struct wlf_multi_backend *backend = multi_backend_from_backend(wlf_backend);

	wl_list_remove(&backend->event_loop_destroy.link);

	wlf_backend_finish(wlf_backend);
	while (!wl_list_empty(&backend->backends)) {
		struct subbackend_state *sub =
			wl_container_of(backend->backends.next, sub, link);
		wlf_backend_destroy(sub->backend);
	}

	free(backend);
}

static int multi_backend_get_drm_fd(struct wlf_backend *backend) {
	struct wlf_multi_backend *multi = multi_backend_from_backend(backend);

	struct subbackend_state *sub;
	wl_list_for_each(sub, &multi->backends, link) {
		if (sub->backend->impl->get_drm_fd) {
			return wlf_backend_get_drm_fd(sub->backend);
		}
	}

	return -1;
}

static uint32_t multi_backend_get_buffer_caps(struct wlf_backend *backend) {
	struct wlf_multi_backend *multi = multi_backend_from_backend(backend);

	if (wl_list_empty(&multi->backends)) {
		return 0;
	}

	uint32_t caps = WLF_BUFFER_CAP_DATA_PTR | WLF_BUFFER_CAP_DMABUF
			| WLF_BUFFER_CAP_SHM;

	struct subbackend_state *sub;
	wl_list_for_each(sub, &multi->backends, link) {
		uint32_t backend_caps = wlf_backend_get_buffer_caps(sub->backend);
		if (backend_caps != 0) {
			// only count backend capable of presenting a buffer
			caps = caps & backend_caps;
		}
	}

	return caps;
}

static const struct wlf_backend_impl backend_impl = {
	.start = multi_backend_start,
	.destroy = multi_backend_destroy,
	.get_drm_fd = multi_backend_get_drm_fd,
	.get_buffer_caps = multi_backend_get_buffer_caps,
};

static void handle_event_loop_destroy(struct wl_listener *listener, void *data) {
	struct wlf_multi_backend *backend =
		wl_container_of(listener, backend, event_loop_destroy);
	multi_backend_destroy((struct wlf_backend*)backend);
}

struct wlf_backend *wlf_multi_backend_create(struct wl_event_loop *loop) {
	struct wlf_multi_backend *backend = calloc(1, sizeof(*backend));
	if (!backend) {
		wlf_log(WLF_ERROR, "Backend allocation failed");
		return NULL;
	}

	wl_list_init(&backend->backends);
	wlf_backend_init(&backend->backend, &backend_impl);

	wl_signal_init(&backend->events.backend_add);
	wl_signal_init(&backend->events.backend_remove);

	if (loop) {
		backend->event_loop_destroy.notify = handle_event_loop_destroy;
		wl_event_loop_add_destroy_listener(loop, &backend->event_loop_destroy);
	}

	return &backend->backend;
}

bool wlf_backend_is_multi(struct wlf_backend *b) {
	return b->impl == &backend_impl;
}

static void new_input_reemit(struct wl_listener *listener, void *data) {
	struct subbackend_state *state = wl_container_of(listener,
			state, new_input);
	wl_signal_emit_mutable(&state->container->events.new_input, data);
}

static void new_output_reemit(struct wl_listener *listener, void *data) {
	struct subbackend_state *state = wl_container_of(listener,
			state, new_output);
	wl_signal_emit_mutable(&state->container->events.new_output, data);
}

static void handle_subbackend_destroy(struct wl_listener *listener,
		void *data) {
	struct subbackend_state *state = wl_container_of(listener, state, destroy);
	subbackend_state_destroy(state);
}

static struct subbackend_state *multi_backend_get_subbackend(struct wlf_multi_backend *multi,
		struct wlf_backend *backend) {
	struct subbackend_state *sub = NULL;
	wl_list_for_each(sub, &multi->backends, link) {
		if (sub->backend == backend) {
			return sub;
		}
	}
	return NULL;
}

static void multi_backend_refresh_features(struct wlf_multi_backend *multi) {
	multi->backend.features.timeline = true;

	struct subbackend_state *sub = NULL;
	wl_list_for_each(sub, &multi->backends, link) {
		if (wlf_backend_get_buffer_caps(sub->backend) & WLF_BUFFER_CAP_DMABUF) {
			multi->backend.features.timeline = multi->backend.features.timeline &&
				sub->backend->features.timeline;
		}
	}
}

bool wlf_multi_backend_add(struct wlf_backend *_multi,
		struct wlf_backend *backend) {
	assert(_multi && backend);
	assert(_multi != backend);

	struct wlf_multi_backend *multi = multi_backend_from_backend(_multi);

	if (multi_backend_get_subbackend(multi, backend)) {
		return true;
	}

	struct subbackend_state *sub = calloc(1, sizeof(*sub));
	if (sub == NULL) {
		wlf_log(WLF_ERROR, "Could not add backend: allocation failed");
		return false;
	}
	wl_list_insert(multi->backends.prev, &sub->link);

	sub->backend = backend;
	sub->container = &multi->backend;

	wl_signal_add(&backend->events.destroy, &sub->destroy);
	sub->destroy.notify = handle_subbackend_destroy;

	wl_signal_add(&backend->events.new_input, &sub->new_input);
	sub->new_input.notify = new_input_reemit;

	wl_signal_add(&backend->events.new_output, &sub->new_output);
	sub->new_output.notify = new_output_reemit;

	multi_backend_refresh_features(multi);
	wl_signal_emit_mutable(&multi->events.backend_add, backend);
	return true;
}

void wlf_multi_backend_remove(struct wlf_backend *_multi,
		struct wlf_backend *backend) {
	struct wlf_multi_backend *multi = multi_backend_from_backend(_multi);

	struct subbackend_state *sub =
		multi_backend_get_subbackend(multi, backend);

	if (sub) {
		wl_signal_emit_mutable(&multi->events.backend_remove, backend);
		subbackend_state_destroy(sub);
		multi_backend_refresh_features(multi);
	}
}

bool wlf_multi_is_empty(struct wlf_backend *_backend) {
	assert(wlf_backend_is_multi(_backend));
	struct wlf_multi_backend *backend = (struct wlf_multi_backend *)_backend;
	return wl_list_length(&backend->backends) < 1;
}

void wlf_multi_for_each_backend(struct wlf_backend *_backend,
		void (*callback)(struct wlf_backend *backend, void *data), void *data) {
	assert(wlf_backend_is_multi(_backend));
	struct wlf_multi_backend *backend = (struct wlf_multi_backend *)_backend;
	struct subbackend_state *sub;
	wl_list_for_each(sub, &backend->backends, link) {
		callback(sub->backend, data);
	}
}
