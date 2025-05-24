#include "wlf/backend/wlf_wl_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_display.h"

#include <stdlib.h>
#include <assert.h>

static bool backend_start(struct wlf_backend *backend) {
	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	wlf_log(WLF_INFO, "Starting Wayland backend");

	wl->started = true;
	return true;
}

static void backend_destroy(struct wlf_backend *backend) {
	if (!backend) {
		return;
	}

	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	if (wl->display) {
		wlf_wl_display_destroy(wl->display);
	}

	free(backend);
};

static const struct wlf_backend_impl backend_impl = {
	.start = backend_start,
	.destroy = backend_destroy,
};

struct wlf_backend *wlf_wl_backend_create(void) {
	wlf_log(WLF_INFO, "Creating wayland backend");

	struct wlf_wl_backend *wl = calloc(1, sizeof(*wl));
	if (!wl) {
		wlf_log(WLF_ERROR, "Allocation failed");
		return NULL;
	}

	wlf_backend_init(&wl->backend, &backend_impl);

	wl->display = wlf_wl_display_create();
	if (!wl->display) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_wl_display failed!");
		goto error_wl;
	}

	// init build-in protocols

	if (!wlf_wl_display_init_registry(wl->display)) {
		wlf_log(WLF_ERROR, "wlf_wl_display_init_registry failed");
		goto error_display;
	}
	return &wl->backend;

error_display:
	wlf_wl_display_destroy(wl->display);
error_wl:
	wlf_backend_finish(&wl->backend);
	free(wl);
	return NULL;
}

bool wlf_backend_is_wl(struct wlf_backend *backend) {
	return backend->impl == &backend_impl;
}

struct wlf_wl_backend *get_wl_backend_from_backend(struct wlf_backend *wlf_backend) {
	assert(wlf_backend_is_wl(wlf_backend));
	struct wlf_wl_backend *backend = wlf_container_of(wlf_backend, backend, backend);
	return backend;
}
