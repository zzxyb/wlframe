#include "wlf/platform/wlf_backend_builtin.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/va/wlf_va_display.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);
	if (!wlf_backend_builtin_init()) {
		wlf_log(WLF_ERROR, "Failed to initialize backend subsystem");
		return EXIT_FAILURE;
	}

	struct wlf_backend *backend = wlf_backend_autocreate();
	if (!backend) {
		wlf_log(WLF_ERROR, "Failed to auto-create backend");
		goto error;
	}

	wlf_log(WLF_INFO, "Auto-created backend: %s", wlf_backend_type_name(wlf_backend_get_type(backend)));

	if (!wlf_backend_start(backend)) {
		wlf_log(WLF_ERROR, "Failed to start backend");
		goto error;
	}

	struct wlf_va_display *va_display = wlf_va_display_autocreate(backend);

	wlf_va_display_destroy(va_display);
	wlf_log(WLF_INFO, "Backend started successfully");
	wlf_backend_destroy(backend);

	return EXIT_SUCCESS;

error:
	wlf_backend_builtin_cleanup();
	wlf_backend_destroy(backend);
	return EXIT_FAILURE;
}
