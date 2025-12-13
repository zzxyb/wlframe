#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_signal.h"

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
}

struct wlf_backend *wlf_backend_autocreate(void) {

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
