#include "wlf/platform/wlf_backend.h"
#include "wlf/config.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_signal.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/wayland/backend.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

void wlf_backend_init(struct wlf_backend *backend,
		const struct wlf_backend_impl *impl) {
	assert(impl->destroy);

	*backend = (struct wlf_backend){
		.impl = impl,
	};

	wlf_signal_init(&backend->events.destroy);
	wlf_signal_init(&backend->events.output_added);
	wlf_signal_init(&backend->events.output_removed);
	wlf_linked_list_init(&backend->outputs);
}

struct wlf_backend *wlf_backend_autocreate(void) {
	struct wlf_backend *backend = NULL;
	const char *backend_options[] = {
		"auto",
		"wayland",
		NULL
	};
	const char *render_name = backend_options[wlf_env_parse_switch("WLF_BACNEND",
		backend_options)];

#if WLF_HAS_LINUX_PLATFORM
	bool is_auto = strcmp(render_name, "auto") == 0;
	if ((is_auto ||
			strcmp(render_name, "wayland") == 0) &&
			strcmp(wlf_get_env("XDG_SESSION_TYPE"), "wayland") == 0) {
		backend = wayland_backend_create();
		if (backend == NULL) {
			wlf_log(WLF_ERROR, "Failed to create Wayland backend");
			return NULL;
		}
	}
#endif

	WLF_UNUSED(render_name);
	return backend;
}

void wlf_backend_destroy(struct wlf_backend *backend) {
	if (backend == NULL) {
		return;
	}

	wlf_log(WLF_DEBUG, "Destroying backend %s", backend->impl->name);

	wlf_signal_emit_mutable(&backend->events.destroy, backend);

	if (backend->impl && backend->impl->destroy) {
		backend->impl->destroy(backend);
	} else {
		free(backend);
	}
}
