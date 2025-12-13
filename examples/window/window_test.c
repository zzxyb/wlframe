#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		wlf_log(WLF_ERROR, "Failed to auto-create backend");
		return EXIT_FAILURE;
	}

	struct wlf_renderer *render = wlf_renderer_autocreate(backend);
	if (render == NULL) {
		wlf_log(WLF_ERROR, "Failed to create render");
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	wlf_log(WLF_INFO, "Backend started successfully");
	wlf_backend_destroy(backend);

	return EXIT_SUCCESS;
}
