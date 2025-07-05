#ifndef SWRAST_SW_BUFFER_H
#define SWRAST_SW_BUFFER_H

#include <pixman.h>

struct wlf_render;
struct wlf_sw_render;

struct wlr_sw_buffer {
	struct wlf_buffer *buffer;
	struct wlf_sw_render *render;

	pixman_image_t *image;

	struct wl_listener buffer_destroy;
	struct wl_list link; // wlr_pixman_renderer.buffers
};

#endif // SWRAST_SW_BUFFER_H
