#ifndef WLF_SURFACE_H
#define WLF_SURFACE_H

#include <pixman.h>

#include "wlf/types/wlf_backend.h"
#include "wlf/types/wlf_buffer.h"
#include "wlf/render/wlf_swapchain.h"
#include "wlf/render/wlf_renderer.h"

enum wlf_surface_state {
	WLF_SURFACE_NORMAL,
	WLF_SURFACE_MINIMIZED,
	WLF_SURFACE_MAXIMIZED,
	WLF_SURFACE_FULLSCREEN,
	WLF_SURFACE_ACTIVE,
}

enum wlf_surface_type {
	WLF_SURFACE_TOPLEVEL,
	WLF_POPUP_SURFACE,
}

struct wlf_surface_state {
	pixman_region32_t damage;

	struct wlf_buffer *buffer;
}

struct wlf_surface {
	struct wlf_backend *backend;
	float scale;

	struct {
		struct wlf_signal frame;
		struct wlf_signal damage;
		struct wlf_signal precommit;
		struct wlf_signal commit;
		struct wlf_signal destroy;
	} events;

	struct wlf_allocator *allocator;
	struct wlf_renderer *renderer;
	struct wlf_swapchain *swapchain;

	void *data;
}

#endif
