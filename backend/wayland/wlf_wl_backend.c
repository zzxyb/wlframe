#include "wlf/wayland/wlf_wl_backend.h"
#include "wlf/util/wlf_log.h"

#include <stdlib.h>
#include <assert.h>

static bool backend_start(struct wlf_backend *backend) {
	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	wlf_log(WLF_INFO, "Starting Wayland backend");

	wl->started = true;

	// struct wlf_wl_seat *seat;
	// wlf_double_list_for_each(seat, &wl->seats, link) {
	// 	if (seat->wl_keyboard) {
	// 		init_seat_keyboard(seat);
	// 	}

	// 	if (seat->wl_touch) {
	// 		init_seat_touch(seat);
	// 	}

	// 	if (wl->tablet_manager) {
	// 		init_seat_tablet(seat);
	// 	}
	// }

	// for (size_t i = 0; i < wl->requested_outputs; ++i) {
	// 	wlf_wl_output_create(&wl->backend);
	// }

	return true;
}

static void backend_destroy(struct wlf_backend *backend) {
	if (!backend) {
		return;
	}

	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	if (wl->remote_display) {
		wl_display_disconnect(wl->remote_display);
	}

	free(backend);
};

static int backend_get_drm_fd(struct wlf_backend *backend) {
	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	return wl->drm_fd;
}

static uint32_t get_buffer_caps(struct wlf_backend *backend) {
	// struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	// return (wl->zwp_linux_dmabuf_v1 ? WLR_BUFFER_CAP_DMABUF : 0)
	// 	| (wl->shm ? WLR_BUFFER_CAP_SHM : 0);
	return 0;
}

static const struct wlf_backend_impl backend_impl = {
	.start = backend_start,
	.destroy = backend_destroy,
	.get_drm_fd = backend_get_drm_fd,
	.get_buffer_caps = get_buffer_caps,
};

struct wlf_backend *wlf_wl_backend_create(void) {
	wlf_log(WLF_INFO, "Creating wayland backend");

	struct wlf_wl_backend *wl = calloc(1, sizeof(*wl));
	if (!wl) {
		wlf_log_errno(WLF_ERROR, "Allocation failed");
		return NULL;
	}

	wlf_backend_init(&wl->backend, &backend_impl);

	// wl->event_loop = loop;
	wlf_double_list_init(&wl->outputs);
	// wlf_double_list_init(&wl->seats);
	wlf_double_list_init(&wl->buffers);
	// wlf_double_list_init(&wl->drm_syncobj_timelines);

	wl->remote_display = wl_display_connect(NULL);
	if (!wl->remote_display) {
		wlf_log_errno(WLF_ERROR, "Could not connect to remote display");
		goto error_wl;
	}

	return &wl->backend;

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