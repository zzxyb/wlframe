#include "wlf/renderer/pixman/renderer.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_log.h"

#include <pixman.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void pixman_renderer_destroy(struct wlf_renderer *render) {
	struct wlf_pixman_renderer *pixman_render =
		wlf_pixman_renderer_from_renderer(render);
	if (pixman_render == NULL) {
		return;
	}

	free(pixman_render);
}

static const struct wlf_renderer_impl pixman_renderer_impl = {
	.destroy = pixman_renderer_destroy,
};

bool wlf_renderer_is_pixman(const struct wlf_renderer *renderer) {
	return renderer->impl == &pixman_renderer_impl;
}

struct wlf_pixman_renderer *wlf_pixman_renderer_from_renderer(
		struct wlf_renderer *renderer) {
	if (!wlf_renderer_is_pixman(renderer)) {
		return NULL;
	}

	struct wlf_pixman_renderer *pixman_renderer =
		wlf_container_of(renderer, pixman_renderer, base);

	return pixman_renderer;
}

struct wlf_renderer *wlf_pixman_renderer_create_from_backend(
		struct wlf_backend *backend) {
	struct wlf_pixman_renderer *renderer = malloc(sizeof(*renderer));
	if (renderer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_pixman_renderer");
		return NULL;
	}

	wlf_renderer_init(&renderer->base, &pixman_renderer_impl);
	renderer->base.type = CPU;
	renderer->backend = backend;

	return &renderer->base;
}
