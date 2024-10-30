#include "wlf/util/wlf_log.h"
#include "wlf/types/wlf_backend.h"

#include <wayland-server-core.h>
#include <wayland-client-core.h>

struct  simple_client{
	struct wl_display *display;
	struct wlf_backend *backend;
};

int main(int argc, char *argv[]) {
	struct simple_client client = {0};
	client.display = wl_display_create();
	client.backend = wlf_backend_autocreate(wl_display_get_event_loop(client.display));
	if (client.backend == NULL) {
		wlf_log(WLF_ERROR, "failed to create wlr_backend");
		return 1;
	}
	
	if (!wlf_backend_start(client.backend)) {
		wlf_backend_destroy(client.backend);
		wl_display_destroy(client.display);
		return 1;
	}

	const char *socket = wl_display_add_socket_auto(client.display);
	if (!socket) {
		wlf_backend_destroy(client.backend);
		return 1;
	}
	wlf_log(WLF_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
			socket);
	wl_display_run(client.display);

	wl_display_destroy_clients(client.display);
	wlf_backend_destroy(client.backend);
	wl_display_destroy(client.display);
	return 0;
}
