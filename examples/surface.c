#include "wlf/util/wlf_log.h"
#include "wlf/types/wlf_backend.h"

#include <wayland-server-core.h>
#include <wayland-client-core.h>

struct  simple_client{
	struct wlf_backend *backend;
};

int main(int argc, char *argv[]) {
	struct simple_client client = {0};
	client.backend = wlf_backend_autocreate(NULL);
	if (client.backend == NULL) {
		wlf_log(WLF_ERROR, "failed to create wlr_backend");
		return 1;
	}

	if (!wlf_backend_start(client.backend)) {
		wlf_backend_destroy(client.backend);
		return 1;
	}

	return 0;
}
