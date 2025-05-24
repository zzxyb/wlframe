#include "wlf/backend/wlf_muti_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdlib.h>
#include <assert.h>

struct subbackend_state {
	struct wlf_backend *backend;
	struct wlf_backend *container;
	struct wlf_listener new_input;
	struct wlf_listener new_output;
	struct wlf_listener destroy;
	struct wlf_linked_list link;
};

static struct wlf_multi_backend *multi_backend_from_backend(
		struct wlf_backend *wlf_backend) {
	assert(wlf_backend_is_multi(wlf_backend));
	struct wlf_multi_backend *backend = wlf_container_of(wlf_backend, backend, backend);
	return backend;
}

static bool multi_backend_start(struct wlf_backend *wlf_backend) {
	struct wlf_multi_backend *backend = multi_backend_from_backend(wlf_backend);
	struct subbackend_state *sub;
	wlf_linked_list_for_each(sub, &backend->backends, link) {
		if (!wlf_backend_start(sub->backend)) {
			wlf_log(WLF_ERROR, "Failed to initialize backend.");
			return false;
		}
	}
	return true;
}

static void subbackend_state_destroy(struct subbackend_state *sub) {
	wlf_linked_list_remove(&sub->new_input.link);
	wlf_linked_list_remove(&sub->new_output.link);
	wlf_linked_list_remove(&sub->destroy.link);
	wlf_linked_list_remove(&sub->link);
	free(sub);
}

static void multi_backend_destroy(struct wlf_backend *wlf_backend) {
	struct wlf_multi_backend *backend = multi_backend_from_backend(wlf_backend);

	wlf_linked_list_remove(&backend->event_loop_destroy.link);

	wlf_backend_finish(wlf_backend);
	while (!wlf_linked_list_empty(&backend->backends)) {
		struct subbackend_state *sub =
			wlf_container_of(backend->backends.next, sub, link);
		wlf_backend_destroy(sub->backend);
	}

	free(backend);
}

static const struct wlf_backend_impl backend_impl = {
	.start = multi_backend_start,
	.destroy = multi_backend_destroy,
};

struct wlf_backend *wlf_multi_backend_create(void) {
	struct wlf_multi_backend *backend = calloc(1, sizeof(*backend));
	if (!backend) {
		wlf_log(WLF_ERROR, "Backend allocation failed");
		return NULL;
	}

	wlf_linked_list_init(&backend->backends);
	wlf_backend_init(&backend->backend, &backend_impl);

	wlf_signal_init(&backend->events.backend_add);
	wlf_signal_init(&backend->events.backend_remove);

	return &backend->backend;
}

bool wlf_backend_is_multi(struct wlf_backend *b) {
	return b->impl == &backend_impl;
}

static void handle_subbackend_destroy(struct wlf_listener *listener,
		void *data) {
	struct subbackend_state *state = wlf_container_of(listener, state, destroy);
	subbackend_state_destroy(state);
}

static struct subbackend_state *multi_backend_get_subbackend(struct wlf_multi_backend *multi,
		struct wlf_backend *backend) {
	struct subbackend_state *sub = NULL;
	wlf_linked_list_for_each(sub, &multi->backends, link) {
		if (sub->backend == backend) {
			return sub;
		}
	}
	return NULL;
}

static void multi_backend_refresh_features(struct wlf_multi_backend *multi) {

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
	wlf_linked_list_insert(multi->backends.prev, &sub->link);

	sub->backend = backend;
	sub->container = &multi->backend;

	wlf_signal_add(&backend->events.destroy, &sub->destroy);
	sub->destroy.notify = handle_subbackend_destroy;

	multi_backend_refresh_features(multi);
	wlf_signal_emit_mutable(&multi->events.backend_add, backend);
	return true;
}

void wlf_multi_backend_remove(struct wlf_backend *_multi,
		struct wlf_backend *backend) {
	struct wlf_multi_backend *multi = multi_backend_from_backend(_multi);

	struct subbackend_state *sub =
		multi_backend_get_subbackend(multi, backend);

	if (sub) {
		wlf_signal_emit_mutable(&multi->events.backend_remove, backend);
		subbackend_state_destroy(sub);
		multi_backend_refresh_features(multi);
	}
}

bool wlf_multi_is_empty(struct wlf_backend *_backend) {
	assert(wlf_backend_is_multi(_backend));
	struct wlf_multi_backend *backend = (struct wlf_multi_backend *)_backend;
	return wlf_linked_list_length(&backend->backends) < 1;
}

void wlf_multi_for_each_backend(struct wlf_backend *_backend,
		void (*callback)(struct wlf_backend *backend, void *data), void *data) {
	assert(wlf_backend_is_multi(_backend));
	struct wlf_multi_backend *backend = (struct wlf_multi_backend *)_backend;
	struct subbackend_state *sub;
	wlf_linked_list_for_each(sub, &backend->backends, link) {
		callback(sub->backend, data);
	}
}
