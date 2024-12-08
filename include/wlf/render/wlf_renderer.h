#ifndef WLF_RENDERER_H
#define WLF_RENDERER_H

#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wlf_renderer_impl;
struct wlf_drm_format_set;
struct wlf_buffer;

/**
 * @brief A structure representing a renderer for basic 2D operations
 */
struct wlf_renderer {
	uint32_t render_buffer_caps; /**< Capabilities required for the buffer used as a render target (bitmask of enum wlf_buffer_cap) */

	struct {
		struct wlf_signal destroy; /**< Signal emitted when the renderer is destroyed */
		/**
		 * @brief Emitted when the GPU is lost, e.g. on GPU reset.
		 *
		 * Compositors should destroy the renderer and re-create it.
		 */
		struct wlf_signal lost; /**< Signal emitted when the GPU is lost */
    } events;

	struct {
		/**
		 * @brief Indicates if the renderer supports color transforms on its output
		 */
		bool output_color_transform;
		/**
		 * @brief Indicates whether wait/signal timelines are supported.
		 *
		 * See struct wlf_drm_syncobj_timeline.
		 */
		bool timeline;
	} features;

	struct {
		const struct wlf_renderer_impl *impl; /**< Pointer to the renderer implementation (private) */
	} WLR_PRIVATE;
};

struct wlf_render_pass {
	const struct wlf_render_pass_impl *impl;
};

struct wlf_render_pass_impl {
	bool (*submit)(struct wlf_render_pass *pass);
	void (*add_texture)(struct wlf_render_pass *pass,
		const struct wlf_render_texture_options *options);
	void (*add_rect)(struct wlf_render_pass *pass,
		const struct wlf_render_rect_options *options);
};

void wlf_render_pass_init(struct wlf_render_pass *pass,
	const struct wlf_render_pass_impl *impl);

/**
 * @brief Automatically creates a new renderer
 *
 * Selects an appropriate renderer type to use depending on the backend,
 * platform, environment, etc.
 * @param backend Pointer to the backend to use
 * @return Pointer to the newly created wlf_renderer structure
 */
struct wlf_renderer *wlf_renderer_autocreate(struct wlf_backend *backend);

/**
 * @brief Gets the formats supporting sampling usage
 *
 * The buffer capabilities must be passed in.
 *
 * Buffers allocated with a format from this list may be passed to
 * wlf_texture_from_buffer().
 * @param r Pointer to the renderer
 * @param buffer_caps Buffer capabilities to check
 * @return Pointer to the set of supported texture formats
 */
const struct wlf_drm_format_set *wlf_renderer_get_texture_formats(
	struct wlf_renderer *r, uint32_t buffer_caps);

/**
 * @brief Initializes wl_shm, linux-dmabuf, and other buffer factory protocols
 * @param r Pointer to the renderer to initialize
 * @param wl_display Pointer to the Wayland display
 * @return true on success, false on failure
 */
bool wlf_renderer_init_wl_display(struct wlf_renderer *r,
	struct wl_display *wl_display);

/**
 * @brief Initializes wl_shm on the provided struct wl_display
 * @param r Pointer to the renderer to initialize
 * @param wl_display Pointer to the Wayland display
 * @return true on success, false on failure
 */
bool wlf_renderer_init_wl_shm(struct wlf_renderer *r,
	struct wl_display *wl_display);

/**
 * @brief Obtains the FD of the DRM device used for rendering
 * @param r Pointer to the renderer
 * @return The DRM file descriptor, or -1 if unavailable
 *
 * The caller doesn't have ownership of the FD; it must not close it.
 */
int wlf_renderer_get_drm_fd(struct wlf_renderer *r);

/**
 * @brief Destroys the renderer
 * @param renderer Pointer to the renderer to destroy
 *
 * Textures must be destroyed separately.
 */
void wlf_renderer_destroy(struct wlf_renderer *renderer);

/**
 * @brief Allocates and initializes a new render timer
 * @param renderer Pointer to the associated renderer
 * @return Pointer to the newly created wlf_render_timer structure
 */
struct wlf_render_timer *wlf_render_timer_create(struct wlf_renderer *renderer);

/**
 * @brief Gets the render duration in nanoseconds from the timer
 * @param timer Pointer to the render timer
 * @return The duration in nanoseconds, or -1 if the duration is unavailable
 */
int wlf_render_timer_get_duration_ns(struct wlf_render_timer *timer);

/**
 * @brief Destroys the render timer
 * @param timer Pointer to the render timer to destroy
 */
void wlf_render_timer_destroy(struct wlf_render_timer *timer);
