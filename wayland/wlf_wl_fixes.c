#include "wlf/wayland/wlf_wl_fixes.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wl_fixes *wlf_wl_fixes_create(struct wl_registry *wl_registry,
		uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_fixes *fixes = calloc(1, sizeof(*fixes));
	if (fixes == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_fixes");
		return NULL;
	}

	uint32_t bind_version = version;
	if (version > (uint32_t)wl_fixes_interface.version) {
		bind_version = (uint32_t)wl_fixes_interface.version;
	}

	fixes->wl_fixes = wl_registry_bind(wl_registry, name,
		&wl_fixes_interface, bind_version);
	if (fixes->wl_fixes == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_fixes");
		free(fixes);
		return NULL;
	}

	wlf_signal_init(&fixes->events.destroy);

	return fixes;
}

void wlf_wl_fixes_destroy(struct wlf_wl_fixes *fixes) {
	if (fixes == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&fixes->events.destroy, fixes);
	wl_fixes_destroy(fixes->wl_fixes);
	free(fixes);
}

void wlf_wl_fixes_destroy_registry(struct wlf_wl_fixes *fixes,
		struct wl_registry *registry) {
	assert(fixes != NULL);
	wl_fixes_destroy_registry(fixes->wl_fixes, registry);
}
