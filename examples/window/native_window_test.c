#include "wlf/platform/wlf_backend.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"

#include <stdlib.h>

int main(void) {
	wlf_log_init(WLF_DEBUG, NULL);
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		return EXIT_FAILURE;
	}
	struct wlf_window *window = wlf_window_create_toplevel(backend, 640, 400);
	if (window == NULL) {
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_window_set_title(window, "wlframe native window");
	struct wlf_color background = wlf_color_from_rgb8(35, 42, 54);
	wlf_window_set_background_color(window, &background);
	wlf_window_show(window);
	wlf_backend_exe(backend);
	wlf_window_destroy(window);
	wlf_backend_destroy(backend);
	return EXIT_SUCCESS;
}
