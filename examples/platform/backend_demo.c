#include "wlf/platform/wlf_backend_builtin.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void backend_destroy(struct wlf_listener *listener, void *data) {
	wlf_log(WLF_INFO, "Backend destroyed!");
}

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);
	if (!wlf_backend_builtin_init()) {
		wlf_log(WLF_ERROR, "Failed to initialize backend subsystem");
		return 1;
	}

	wlf_log(WLF_INFO, "=== Auto-create backend ===");
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		wlf_log(WLF_ERROR, "Failed to auto-create backend");
		wlf_backend_builtin_cleanup();
		return 1;
	}

	wlf_log(WLF_INFO, "Auto-created backend: %s", wlf_backend_type_name(wlf_backend_get_type(backend)));

	struct wlf_listener destroy_listener;
	destroy_listener.notify = backend_destroy;
	wlf_signal_add(&backend->events.destroy, &destroy_listener);
	if (!wlf_backend_start(backend)) {
		wlf_log(WLF_ERROR, "Failed to start backend");
		wlf_backend_destroy(backend);
		wlf_backend_builtin_cleanup();
		return 1;
	}

	wlf_log(WLF_INFO, "Backend started successfully");
	wlf_backend_destroy(backend);
	wlf_log(WLF_INFO, "=== Create Wayland backend ===");
	struct wlf_backend_create_args args = {
		.type = WLF_BACKEND_WAYLAND,
		.wayland = {
			.display = NULL,
		}
	};

	struct wlf_backend *wayland = wlf_backend_create(&args);
	if (wayland == NULL) {
		wlf_log(WLF_ERROR, "Failed to create Wayland backend");
		wlf_backend_builtin_cleanup();
		return 1;
	}

	wlf_log(WLF_INFO, "Created Wayland backend");

	if (wlf_backend_start(wayland)) {
		wlf_log(WLF_INFO, "Wayland backend started successfully");
	} else {
		wlf_log(WLF_ERROR, "Failed to start Wayland backend (may not be available)");
	}

	wlf_backend_destroy(wayland);
	wlf_backend_builtin_cleanup();

	wlf_log(WLF_INFO, "=== Demo completed successfully ===");
	return 0;
}
