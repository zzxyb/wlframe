#ifndef WLF_MULTI_BACKEND_H
#define WLF_MULTI_BACKEND_H

#include "wlf/types/wlf_backend.h"
#include "wlf/util/wlf_double_list.h"
#include "wlf/util/wlf_signal.h"

#include <stdbool.h>

struct wlf_multi_backend {
	struct wlf_backend backend;

	struct wlf_double_list backends;

	struct wlf_double_listener event_loop_destroy;

	struct {
		struct wlf_signal backend_add;
		struct wlf_signal backend_remove;
	} events;
};

/**
 * Creates a multi-backend. Multi-backends wrap an arbitrary number of backends
 * and aggregate their new_output/new_input signals.
 */
struct wlf_backend *wlf_multi_backend_create(void);
/**
 * Adds the given backend to the multi backend. This should be done before the
 * new backend is started.
 */
bool wlf_multi_backend_add(struct wlf_backend *multi,
	struct wlf_backend *backend);

void wlf_multi_backend_remove(struct wlf_backend *multi,
	struct wlf_backend *backend);

bool wlf_backend_is_multi(struct wlf_backend *backend);
bool wlf_multi_is_empty(struct wlf_backend *backend);

void wlf_multi_for_each_backend(struct wlf_backend *backend,
	void (*callback)(struct wlf_backend *backend, void *data), void *data);

#endif
