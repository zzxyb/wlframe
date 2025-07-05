#include "wlf/render/swrast/sw_render.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>

struct wlf_render *wlf_sw_render_create_from_backend(
		struct wlf_backend *backend) {
	struct wlf_sw_render *render = calloc(1, sizeof(struct wlf_sw_render));
	if (render == NULL) {
		wlf_log(WLF_ERROR, "failed to allocate wlf_sw_render");
		return NULL;
	}

	wlf_log(WLF_INFO, "Creating swrast render");

	return &render->base;
}

void wlf_sw_render_destroy(struct wlf_sw_render *sw_render) {

}

bool wlf_render_is_sw(struct wlf_render *wlf_render) {
	if (wlf_render == NULL) {
		return false;
	}

	return wlf_render->type == WLF_RENDER_TYPE_SWRAST;
}

struct wlf_sw_render *wlf_sw_render_from_render(struct wlf_render *wlf_render) {

}
