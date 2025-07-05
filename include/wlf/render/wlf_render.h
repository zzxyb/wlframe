#ifndef RENDER_WLF_RENDER_H
#define RENDER_WLF_RENDER_H

#include "wlf/utils/wlf_signal.h"

struct wlf_render;
struct wlf_backend;

/**
 * @brief Renderer backend types
 */
enum wlf_render_type {
	GPU,
	CPU,
};

/**
 * @brief Renderer implementation interface
 */
struct wlf_render_impl {
	void (*destroy)(struct wlf_render *render);
};

struct wlf_render {
	const struct wlf_render_impl *impl;

	struct {
		struct wlf_signal destroy;  /**< Signal emitted when the render is destroyed */
	} events;

	void *data;

	enum wlf_render_type type;
};

/**
 * @brief Create a render with automatic backend detection
 * @param backend Pointer to the backend
 * @return Pointer to the created render, or NULL on failure
 */
struct wlf_render *wlf_render_autocreate(struct wlf_backend *backend);

/**
 * @brief Destroy a render
 * @param render Pointer to the render to destroy
 */
void wlf_render_destroy(struct wlf_render *render);

void wlf_render_init(struct wlf_render *render, const struct wlf_render_impl *impl);

#endif // RENDER_WLF_RENDER_H
