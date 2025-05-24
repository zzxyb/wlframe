#ifndef BACKEND_WLF_BACKEND_H
#define BACKEND_WLF_BACKEND_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stdbool.h>

struct wlf_backend_impl;

/**
 * A backend provides a set of input and output devices.
 */
struct wlf_backend {
	const struct wlf_backend_impl *impl;

	struct {
		/** Raised when destroyed */
		struct wlf_signal destroy;
	} events;
};

struct wlf_backend_impl {
	bool (*start)(struct wlf_backend *backend);
	void (*destroy)(struct wlf_backend *backend);
};

/**
 * Automatically initializes the most suitable backend given the environment.
 * Will always return a multi-backend. The backend is created but not started.
 * Returns NULL on failure.
 *
 * If session_ptr is not NULL, it's populated with the session which has been
 * created with the backend, if any.
 *
 * The multi-backend will be destroyed if one of the primary underlying
 * backends is destroyed (e.g. if the primary DRM device is unplugged).
 */
struct wlf_backend *wlf_backend_autocreate(void);

void wlf_backend_init(struct wlf_backend *backend,
	const struct wlf_backend_impl *impl);

void wlf_backend_finish(struct wlf_backend *backend);

/**
 * Start the backend. This may signal new_input or new_output immediately, but
 * may also wait until the display's event loop begins. Returns false on
 * failure.
 */
bool wlf_backend_start(struct wlf_backend *backend);
/**
 * Destroy the backend and clean up all of its resources. Normally called
 * automatically when the struct wl_display is destroyed.
 */
void wlf_backend_destroy(struct wlf_backend *backend);

#endif // BACKEND_WLF_BACKEND_H
