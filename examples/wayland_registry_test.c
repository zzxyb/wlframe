#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/wayland/wlf_wl_compositor.h"

#include <assert.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

struct  simple_client{
	struct wlf_backend *backend;
};

int main(int argc, char *argv[]) {
	struct wlf_wl_display *display = wlf_wl_display_create();
	if (display == NULL) {
		wlf_log(WLF_ERROR, "Failed to create display");
		return -1;
	}
	wlf_wl_display_init_registry(display);
	struct wlf_wl_compositor *compositor = wlf_wl_compositor_create(display);
	if (compositor == NULL) {
		wlf_log(WLF_ERROR, "Failed to create compositor");
		wlf_wl_display_destroy(display);
		return -1;
	}

	assert(compositor->compositor != NULL);
	wlf_log(WLF_INFO, "wl_compositor interface: %p", compositor->compositor);

	return 0;
}
