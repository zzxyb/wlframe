#include "wlf/platform/wlf_backend.h"
#include "wlf/config.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_signal.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/wayland/backend.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

void wlf_backend_init(struct wlf_backend *backend,
		const struct wlf_backend_impl *impl) {
	assert(impl->destroy);
	assert(impl->name);
	assert(impl->exe);

	*backend = (struct wlf_backend){
		.impl = impl,
	};

	wlf_signal_init(&backend->events.destroy);
	wlf_signal_init(&backend->events.output_added);
	wlf_signal_init(&backend->events.output_removed);
	wlf_linked_list_init(&backend->outputs);
}

struct wlf_backend *wlf_backend_autocreate(void) {
	struct wlf_backend *backend = NULL;

#if WLF_HAS_LINUX_PLATFORM
	if (strcmp(wlf_get_env("XDG_SESSION_TYPE"), "wayland") == 0) {
		backend = wayland_backend_create();
		if (backend == NULL) {
			wlf_log(WLF_ERROR, "Failed to create Wayland backend");
			return NULL;
		}
	}
#endif

	return backend;
}

void wlf_backend_destroy(struct wlf_backend *backend) {
	if (backend == NULL) {
		return;
	}

	wlf_log(WLF_DEBUG, "Destroying backend %s", backend->impl->name);

	wlf_signal_emit_mutable(&backend->events.destroy, backend);
	free(backend->event_sources);
	backend->event_sources = NULL;
	backend->event_source_count = 0;
	backend->event_source_capacity = 0;

	if (backend->impl && backend->impl->destroy) {
		backend->impl->destroy(backend);
	} else {
		free(backend);
	}
}

void wlf_backend_exe(struct wlf_backend *backend) {
	backend->impl->exe(backend);
}

bool wlf_backend_add_event_source(struct wlf_backend *backend,
		int fd, short events,
		wlf_backend_event_source_dispatch_t dispatch,
		void *data) {
	assert(backend != NULL);
	assert(dispatch != NULL);

	if (fd < 0) {
		wlf_log(WLF_ERROR, "Invalid fd for backend event source");
		return false;
	}

	if (backend->event_source_count == backend->event_source_capacity) {
		size_t new_capacity = backend->event_source_capacity == 0 ? 4 : backend->event_source_capacity * 2;
		void *new_sources = realloc(backend->event_sources,
			new_capacity * sizeof(*backend->event_sources));
		if (new_sources == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to grow backend event source array");
			return false;
		}

		backend->event_sources = new_sources;
		backend->event_source_capacity = new_capacity;
	}

	backend->event_sources[backend->event_source_count].fd = fd;
	backend->event_sources[backend->event_source_count].events = events;
	backend->event_sources[backend->event_source_count].dispatch = dispatch;
	backend->event_sources[backend->event_source_count].data = data;
	backend->event_source_count++;

	return true;
}

bool wlf_backend_remove_event_source(struct wlf_backend *backend,
		int fd, void *data) {
	assert(backend != NULL);

	for (size_t i = 0; i < backend->event_source_count; ++i) {
		if (backend->event_sources[i].fd == fd && backend->event_sources[i].data == data) {
			if (i + 1 < backend->event_source_count) {
				memmove(&backend->event_sources[i],
					&backend->event_sources[i + 1],
					(backend->event_source_count - i - 1) * sizeof(*backend->event_sources));
			}

			backend->event_source_count--;
			return true;
		}
	}

	return false;
}

void wlf_backend_quit(struct wlf_backend *backend) {
	backend->running = false;
}
