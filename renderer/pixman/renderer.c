/**
 * @file        renderer.c
 * @brief       Pixman software renderer implementation for wlframe.
 * @details     This file implements the CPU-based software renderer using the Pixman
 *              library. The Pixman renderer provides a fallback rendering path suitable
 *              for platforms without GPU acceleration.
 *
 *              The renderer creates and manages a pixman_image_t as the rendering target,
 *              and implements the standard wlf_renderer interface.
 *
 * @author      YaoBing Xiao
 * @date        2026-03-08
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2026-03-08, initial version.
 */

#include "wlf/renderer/pixman/renderer.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <pixman.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

/**
 * @brief Default pixman rendering surface width in pixels.
 */
#define WLF_PIXMAN_DEFAULT_WIDTH  1920

/**
 * @brief Default pixman rendering surface height in pixels.
 */
#define WLF_PIXMAN_DEFAULT_HEIGHT 1080

/* Forward declaration */
static void pixman_renderer_destroy(struct wlf_renderer *render);

static const struct wlf_renderer_impl pixman_renderer_impl = {
	.destroy = pixman_renderer_destroy,
};

bool wlf_renderer_is_pixman(const struct wlf_renderer *renderer) {
	if (renderer == NULL) {
		return false;
	}

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

/**
 * @brief Destroys the Pixman renderer and frees all associated resources.
 *
 * @param render Pointer to the base renderer to destroy.
 */
static void pixman_renderer_destroy(struct wlf_renderer *render) {
	struct wlf_pixman_renderer *pixman_render =
		wlf_pixman_renderer_from_renderer(render);
	if (pixman_render == NULL) {
		return;
	}

	if (pixman_render->target_image != NULL) {
		pixman_image_unref(pixman_render->target_image);
		pixman_render->target_image = NULL;
	}

	free(pixman_render);
}

struct wlf_renderer *wlf_pixman_renderer_create_from_backend(
		struct wlf_backend *backend) {
	struct wlf_pixman_renderer *render = calloc(1, sizeof(*render));
	if (render == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_pixman_renderer");
		return NULL;
	}

	render->backend = backend;
	render->width = WLF_PIXMAN_DEFAULT_WIDTH;
	render->height = WLF_PIXMAN_DEFAULT_HEIGHT;

	/* Create the default ARGB32 pixman image as the rendering target */
	render->target_image = pixman_image_create_bits(
		PIXMAN_a8r8g8b8,
		(int)render->width,
		(int)render->height,
		NULL,
		0);
	if (render->target_image == NULL) {
		wlf_log(WLF_ERROR, "Failed to create pixman image for renderer");
		free(render);
		return NULL;
	}

	wlf_renderer_init(&render->base, &pixman_renderer_impl);
	render->base.type = CPU;

	wlf_log(WLF_INFO, "Pixman software renderer created (%ux%u, ARGB32)",
		render->width, render->height);

	return &render->base;
}
