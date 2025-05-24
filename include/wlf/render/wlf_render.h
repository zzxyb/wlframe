#ifndef RENDER_WLF_RENDER_H
#define RENDER_WLF_RENDER_H

#include "wlf/utils/wlf_signal.h"

struct wlf_renderer;
struct wlf_backend;

struct wlf_renderer_impl {
	void (*destroy)(struct wlf_renderer *renderer);
};

struct wlf_renderer {
	struct {
		struct wlf_signal destroy;
		/**
		 * Emitted when the GPU is lost, e.g. on GPU reset.
		 */
		struct wlf_signal lost;
	} events;
};

void wlf_renderer_init(struct wlf_renderer *renderer, const struct wlf_renderer_impl *impl);
struct wlf_renderer *wlr_renderer_autocreate(struct wlf_backend *backend);
void wlr_renderer_destroy(struct wlf_renderer *renderer);

#endif // RENDER_WLF_RENDER_H
