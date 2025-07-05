#ifndef SWRAST_SW_RENDER_H
#define SWRAST_SW_RENDER_H

#include "wlf/render/wlf_render.h"

#include <stdbool.h>

struct wlf_backend;

struct wlf_sw_render {
	struct wlf_render base;
};

struct wlf_render *wlf_sw_render_create_from_backend(
	struct wlf_backend *backend);
void wlf_sw_render_destroy(struct wlf_sw_render *sw_render);

bool wlf_render_is_sw(struct wlf_render *wlf_render);
struct wlf_sw_render *wlf_sw_render_from_render(struct wlf_render *wlf_render);

#endif // SWRAST_SW_RENDER_H
