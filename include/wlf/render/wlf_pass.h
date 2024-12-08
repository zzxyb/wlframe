#ifndef WLF_RENDER_PASS_H
#define WLF_RENDER_PASS_H

#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_signal.h"

#include <pixman.h>
#include <stdint.h>

struct wlf_renderer;
struct wlf_buffer;
struct wlf_render_pass;
struct wlf_render_timer;

/**
 * @brief A structure representing options for a buffer pass
 */
struct wlf_buffer_pass_options {
	struct wlf_render_timer *timer; /**< Timer to measure the duration of the render pass */
	struct wlf_color_transform *color_transform; /**< Color transform to apply to the output of the render pass, leave NULL to indicate sRGB/no custom transform */

    /**
     * @brief Signal a timeline synchronization point when the render pass completes.
     *
     * When a compositor provides a signal timeline, the renderer may skip
     * implicit signal synchronization.
     *
     * Support for this feature is advertised by features.timeline in
     * struct wlf_renderer.
     */
	struct wlf_drm_syncobj_timeline *signal_timeline; /**< Pointer to the signal timeline */
	uint64_t signal_point; /**< Signal point for synchronization */
};

/**
 * @brief Begins a new render pass with the supplied destination buffer
 * @param renderer Pointer to the renderer
 * @param buffer Pointer to the destination buffer
 * @param options Pointer to the buffer pass options
 * @return Pointer to the newly created wlf_render_pass structure
 *
 * Callers must call wlf_render_pass_submit() once they are done with the
 * render pass.
 */
struct wlf_render_pass *wlf_renderer_begin_buffer_pass(struct wlf_renderer *renderer,
	struct wlf_buffer *buffer,
	const struct wlf_buffer_pass_options *options);

/**
 * @brief Submits the render pass
 * @param render_pass Pointer to the render pass to submit
 * @return true if the submission was successful, false otherwise
 *
 * The render pass cannot be used after this function is called.
 */
bool wlf_render_pass_submit(struct wlf_render_pass *render_pass);

/**
 * @brief Enumeration of blend modes
 */
enum wlf_render_blend_mode {
	WLF_RENDER_BLEND_MODE_PREMULTIPLIED, /**< Pre-multiplied alpha (default) */
	WLF_RENDER_BLEND_MODE_NONE,           /**< Blending is disabled */
};

/**
 * @brief Enumeration of filter modes
 */
enum wlf_scale_filter_mode {
	WLF_SCALE_FILTER_BILINEAR, /**< Bilinear texture filtering (default) */
	WLF_SCALE_FILTER_NEAREST,   /**< Nearest texture filtering */
};

/**
 * @brief A structure representing options for rendering a texture
 */
struct wlf_render_texture_options {
	struct wlf_texture *texture; /**< Source texture */
	struct wlf_fbox src_box;     /**< Source coordinates, leave empty to render the whole texture */
	struct wlf_box dst_box;      /**< Destination coordinates, width/height default to the texture size */
	const float *alpha;          /**< Opacity between 0 (transparent) and 1 (opaque), leave NULL for opaque */
	const pixman_region32_t *clip; /**< Clip region, leave NULL to disable clipping */
	enum wl_output_transform transform; /**< Transform applied to the source texture */
	enum wlf_scale_filter_mode filter_mode; /**< Filtering mode */
	enum wlf_render_blend_mode blend_mode; /**< Blend mode */

    /**
     * @brief Wait for a timeline synchronization point before texturing.
     *
     * When a compositor provides a wait timeline, the renderer may skip
     * implicit wait synchronization.
     *
     * Support for this feature is advertised by features.timeline in
     * struct wlf_renderer.
     */
	struct wlf_drm_syncobj_timeline *wait_timeline; /**< Pointer to the wait timeline */
	uint64_t wait_point; /**< Wait point for synchronization */
};

/**
 * @brief Renders a texture
 * @param render_pass Pointer to the render pass
 * @param options Pointer to the texture rendering options
 */
void wlf_render_pass_add_texture(struct wlf_render_pass *render_pass,
	const struct wlf_render_texture_options *options);

/**
 * @brief A structure representing a color value
 *
 * Each channel has values between 0 and 1 inclusive. The R, G, B
 * channels need to be pre-multiplied by A.
 */
struct wlf_render_color {
	float r; /**< Red channel */
	float g; /**< Green channel */
	float b; /**< Blue channel */
	float a; /**< Alpha channel */
};

/**
 * @brief A structure representing options for rendering a rectangle
 */
struct wlf_render_rect_options {
	struct wlf_box box; /**< Rectangle coordinates */
	struct wlf_render_color color; /**< Source color */
	const pixman_region32_t *clip; /**< Clip region, leave NULL to disable clipping */
	enum wlf_render_blend_mode blend_mode; /**< Blend mode */
};

/**
 * @brief Renders a rectangle
 * @param render_pass Pointer to the render pass
 * @param options Pointer to the rectangle rendering options
 */
void wlf_render_pass_add_rect(struct wlf_render_pass *render_pass,
	const struct wlf_render_rect_options *options);

#endif

