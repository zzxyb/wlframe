#include "wlf/platform/macos/backend.h"
#include "wlf/types/wlf_output.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/**
 * @brief macOS output manager specific data
 */
struct wlf_macos_output_manager {
	struct wlf_output_manager base;  /**< Base output manager structure */
};

static void macos_output_manager_destroy_impl(struct wlf_output_manager *manager) {
	struct wlf_macos_output_manager *macos_manager =
		(struct wlf_macos_output_manager *)manager;
	free(macos_manager);
}

static const struct wlf_output_manager_impl macos_output_manager_impl = {
	.destroy = macos_output_manager_destroy_impl,
};

static struct wlf_output_manager *macos_output_manager_create(void) {
	struct wlf_macos_output_manager *manager = calloc(1, sizeof(struct wlf_macos_output_manager));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate macOS output manager");
		return NULL;
	}

	wlf_output_manager_init(&manager->base, &macos_output_manager_impl);
	wlf_log(WLF_DEBUG, "Created macOS output manager");

	return &manager->base;
}

static void macos_output_manager_destroy(struct wlf_listener *listener, void *data) {
	struct wlf_backend_macos *backend =
		wlf_container_of(listener, backend, listeners.output_manager_destroy);
	wlf_output_manager_destroy(backend->base.output_manager);
	backend->base.output_manager = NULL;
}

static bool macos_backend_start(struct wlf_backend *backend) {
	struct wlf_backend_macos *macos = (struct wlf_backend_macos *)backend;

	if (macos->started) {
		wlf_log(WLF_INFO, "macOS backend already started");
		return true;
	}

	// Create output manager for macOS
	backend->output_manager = macos_output_manager_create();
	if (backend->output_manager == NULL) {
		wlf_log(WLF_ERROR, "Failed to create output manager for macOS backend");
		return false;
	}

	macos->listeners.output_manager_destroy.notify = macos_output_manager_destroy;
	wlf_signal_add(&backend->output_manager->events.destroy, &macos->listeners.output_manager_destroy);

	// TODO: Enumerate displays and create outputs
	// This would involve using NSScreen or CGDisplay APIs

	macos->started = true;
	wlf_log(WLF_INFO, "macOS backend started successfully");

	return true;
}

static void macos_backend_stop(struct wlf_backend *backend) {
	struct wlf_backend_macos *macos = (struct wlf_backend_macos *)backend;

	if (!macos->started) {
		return;
	}

	wlf_log(WLF_DEBUG, "Stopping macOS backend");

	if (backend->output_manager) {
		wlf_output_manager_destroy(backend->output_manager);
		backend->output_manager = NULL;
	}

	macos->started = false;
}

static void macos_backend_destroy(struct wlf_backend *backend) {
	struct wlf_backend_macos *macos = (struct wlf_backend_macos *)backend;
	wlf_log(WLF_DEBUG, "Destroying macOS backend");

	macos_backend_stop(backend);
	wlf_linked_list_remove(&macos->listeners.output_manager_destroy.link);

	free(macos);
}

static const struct wlf_backend_impl macos_backend_impl = {
	.start = macos_backend_start,
	.stop = macos_backend_stop,
	.destroy = macos_backend_destroy,
};

static struct wlf_backend *macos_backend_create(void *args) {
	struct wlf_backend_macos *backend = calloc(1, sizeof(struct wlf_backend_macos));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate macOS backend");
		return NULL;
	}

	backend->base.impl = &macos_backend_impl;
	backend->base.type = WLF_BACKEND_MACOS;
	backend->base.data = backend;
	wlf_signal_init(&backend->base.events.destroy);

	backend->started = false;

	wlf_log(WLF_INFO, "Created macOS backend");
	return &backend->base;
}

static bool macos_backend_is_available(void) {
	// macOS backend is available if we're running on macOS
#ifdef __APPLE__
	return true;
#else
	return false;
#endif
}

static struct wlf_backend_registry_entry entry = {
	.type = WLF_BACKEND_MACOS,
	.name = "macos",
	.priority = 100,
	.create = macos_backend_create,
	.is_available = macos_backend_is_available,
	.handle = NULL,
};

bool wlf_backend_macos_register(void) {
	return wlf_backend_register(&entry);
}

bool wlf_backend_is_macos(struct wlf_backend *backend) {
	return (backend && backend->impl == &macos_backend_impl &&
			backend->type == WLF_BACKEND_MACOS);
}

struct wlf_backend_macos *wlf_backend_macos_from_backend(struct wlf_backend *wlf_backend) {
	assert(wlf_backend && wlf_backend->impl == &macos_backend_impl);
	struct wlf_backend_macos *backend = wlf_container_of(wlf_backend, backend, base);

	return backend;
}
