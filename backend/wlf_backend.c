#include "wlf/backend/wlf_backend.h"
#include "wlf/backend/wlf_muti_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_backend.h"
#include "wlf/utils/wlf_signal.h"

#include <stdlib.h>

struct wlf_backend *wlf_backend_autocreate(void) {
    struct wlf_backend *multi = wlf_multi_backend_create();

    if (!multi) {
        wlf_log(WLF_ERROR, "Failed to allocate multibackend");
        return NULL;
    }

	if (getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET")) {
		struct wlf_backend *wl_backend = wlf_wl_backend_create();
		if (!wl_backend) {
			goto error;
		}
		wlf_multi_backend_add(multi, wl_backend);
		return multi;
	}

error:
	wlf_backend_destroy(multi);

    return NULL;
}

void wlf_backend_init(struct wlf_backend *backend,
		const struct wlf_backend_impl *impl) {
	*backend = (struct wlf_backend){
		.impl = impl,
	};
	wlf_signal_init(&backend->events.destroy);
	wlf_signal_init(&backend->events.new_input);
	wlf_signal_init(&backend->events.new_output);
}

void wlf_backend_finish(struct wlf_backend *backend) {
    wlf_signal_emit_mutable(&backend->events.destroy, backend);
}

bool wlf_backend_start(struct wlf_backend *backend) {
	if (backend->impl && backend->impl->start) {
		return backend->impl->start(backend);
	}

	return false;
}

void wlf_backend_destroy(struct wlf_backend *backend) {
	if (!backend) {
        wlf_log(WLF_ERROR, "backend is NULL!");
		return;
	}

	if (backend->impl && backend->impl->destroy) {
		backend->impl->destroy(backend);
	} else {
		free(backend);
	}
}

int wlf_backend_get_drm_fd(struct wlf_backend *backend) {
	if (!backend->impl->get_drm_fd) {
		return -1;
	}
	return backend->impl->get_drm_fd(backend);
}

uint32_t wlf_backend_get_buffer_caps(struct wlf_backend *backend) {
    	if (!backend->impl->get_buffer_caps) {
		return 0;
	}

	return backend->impl->get_buffer_caps(backend);
}
