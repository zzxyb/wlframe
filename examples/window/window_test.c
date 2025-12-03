#include "wlf/platform/wlf_backend_builtin.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_log.h"

#include <unistd.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);
	if (!wlf_backend_builtin_init()) {
		wlf_log(WLF_ERROR, "Failed to initialize backend subsystem");
		return 1;
	}

	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		wlf_log(WLF_ERROR, "Failed to auto-create backend");
		wlf_backend_builtin_cleanup();
		return 1;
	}

	wlf_log(WLF_INFO, "Auto-created backend: %s", wlf_backend_type_name(wlf_backend_get_type(backend)));

	if (!wlf_backend_start(backend)) {
		wlf_log(WLF_ERROR, "Failed to start backend");
		wlf_backend_destroy(backend);
		wlf_backend_builtin_cleanup();
		return 1;
	}

	struct wlf_renderer *render = wlf_renderer_autocreate(backend);
	if (render == NULL) {
		wlf_log(WLF_ERROR, "Failed to create render");
		wlf_backend_destroy(backend);
		wlf_backend_builtin_cleanup();
		return 1;
	}

	wlf_log(WLF_INFO, "Backend started successfully");
	wlf_backend_destroy(backend);

	return 0;
}
