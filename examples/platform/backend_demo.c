#include "wlf/platform/wlf_backend_builtin.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void on_backend_destroy(struct wlf_listener *listener, void *data) {
	printf("Backend destroyed!\n");
}

int main(int argc, char *argv[]) {
	// Initialize logging
	wlf_log_init(WLF_DEBUG, NULL);

	// Initialize backend subsystem
	if (!wlf_backend_builtin_init()) {
		wlf_log(WLF_ERROR, "Failed to initialize backend subsystem");
		return 1;
	}

	// Demo 1: Auto-create backend
	printf("\n=== Demo 1: Auto-create backend ===\n");
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (!backend) {
		wlf_log(WLF_ERROR, "Failed to auto-create backend");
		wlf_backend_builtin_cleanup();
		return 1;
	}

	printf("Auto-created backend: %s\n", wlf_backend_type_name(wlf_backend_get_type(backend)));

	// Setup event listeners
	struct wlf_listener destroy_listener = { .notify = on_backend_destroy };

	wlf_signal_add(&backend->events.destroy, &destroy_listener);

	// Start the backend
	if (!wlf_backend_start(backend)) {
		wlf_log(WLF_ERROR, "Failed to start backend");
		wlf_backend_destroy(backend);
		wlf_backend_builtin_cleanup();
		return 1;
	}

	printf("Backend started successfully\n");

	// Cleanup
	wlf_backend_destroy(backend);

	// Demo 2: Create Wayland backend explicitly
	printf("\n=== Demo 2: Create Wayland backend ===\n");
	struct wlf_backend_create_args args = {
		.type = WLF_BACKEND_WAYLAND,
		.wayland = {
			.display = NULL,  // Use default display
		}
	};

	struct wlf_backend *wayland = wlf_backend_create(&args);
	if (!wayland) {
		wlf_log(WLF_ERROR, "Failed to create Wayland backend");
		wlf_backend_builtin_cleanup();
		return 1;
	}

	printf("Created Wayland backend\n");

	if (wlf_backend_start(wayland)) {
		printf("Wayland backend started successfully\n");
	} else {
		printf("Failed to start Wayland backend (may not be available)\n");
	}

	wlf_backend_destroy(wayland);

	// Cleanup backend subsystem
	wlf_backend_builtin_cleanup();

	printf("\n=== Demo completed successfully ===\n");
	return 0;
}
