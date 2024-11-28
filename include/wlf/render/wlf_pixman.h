#ifndef WLF_PIXMAN_H
#define WLF_PIXMAN_H

#include <pixman.h>

#include "wlf/types/wlf_buffer.h"
#include "wlf/types/wlf_texture.h"
#include "wlf/render/wlf_drm_format_set.h"
#include "wlf/render/wlf_renderer.h"

/**
 * @brief A structure representing a pixel format for Pixman
 */
struct wlf_pixman_pixel_format {
	uint32_t drm_format;          /**< DRM format identifier */
	pixman_format_code_t pixman_format; /**< Pixman format code */
};

struct wlf_pixman_buffer;

/**
 * @brief A structure representing a Pixman renderer
 */
struct wlf_pixman_renderer {
	struct wlf_renderer wlf_renderer; /**< Base renderer structure */

	struct wlf_double_list buffers;  /**< List of Pixman buffers (wlf_pixman_buffer.link) */
	struct wlf_double_list textures;  /**< List of Pixman textures (wlf_pixman_texture.link) */

	struct wlf_drm_format_set drm_formats; /**< Set of DRM formats supported by the renderer */
};

/**
 * @brief A structure representing a Pixman buffer
 */
struct wlf_pixman_buffer {
	struct wlf_buffer *buffer; /**< Pointer to the associated buffer */
	struct wlf_pixman_renderer *renderer; /**< Pointer to the associated Pixman renderer */

	pixman_image_t *image; /**< Pointer to the Pixman image associated with the buffer */

	struct wlf_double_listener buffer_destroy; /**< Listener for buffer destruction events */
	struct wlf_double_list link; /**< Link for the buffer in the renderer's list (wlf_pixman_renderer.buffers) */
};

/**
 * @brief A structure representing a Pixman texture
 */
struct wlf_pixman_texture {
	struct wlf_texture wlf_texture; /**< Base texture structure */
	struct wlf_pixman_renderer *renderer; /**< Pointer to the associated Pixman renderer */
	struct wlf_double_list link; /**< Link for the texture in the renderer's list (wlf_pixman_renderer.textures) */

	pixman_image_t *image; /**< Pointer to the Pixman image associated with the texture */
	pixman_format_code_t format; /**< Pixman format code for the texture */
	const struct wlf_pixel_format_info *format_info; /**< Pointer to format information */

	void *data; /**< Pointer to data if created via texture_from_pixels */
	struct wlf_buffer *buffer; /**< Pointer to the associated buffer if created via texture_from_buffer */
};

/**
 * @brief A structure representing a Pixman render pass
 */
struct wlf_pixman_render_pass {
	struct wlf_render_pass base; /**< Base render pass structure */
	struct wlf_pixman_buffer *buffer; /**< Pointer to the associated Pixman buffer */
};

/**
 * @brief Gets the Pixman format code from a DRM format
 * @param fmt The DRM format to convert
 * @return The corresponding Pixman format code
 */
pixman_format_code_t get_pixman_format_from_drm(uint32_t fmt);

/**
 * @brief Gets the DRM format from a Pixman format code
 * @param fmt The Pixman format code to convert
 * @return The corresponding DRM format
 */
uint32_t get_drm_format_from_pixman(pixman_format_code_t fmt);

/**
 * @brief Gets the list of Pixman DRM formats
 * @param len Pointer to a size_t to store the length of the format array
 * @return Pointer to the array of Pixman DRM formats
 */
const uint32_t *get_pixman_drm_formats(size_t *len);

/**
 * @brief Begins access to the Pixman data pointer for a buffer
 * @param buffer Pointer to the buffer to access
 * @param image_ptr Pointer to store the Pixman image pointer
 * @param flags Access flags
 * @return true if access was successful, false otherwise
 */
bool begin_pixman_data_ptr_access(struct wlf_buffer *buffer, pixman_image_t **image_ptr,
	uint32_t flags);

/**
 * @brief Begins a Pixman render pass with the specified buffer
 * @param buffer Pointer to the Pixman buffer to use
 * @return Pointer to the newly created wlf_pixman_render_pass structure
 */
struct wlf_pixman_render_pass *begin_pixman_render_pass(
	struct wlf_pixman_buffer *buffer);

/**
 * @brief Creates a new Pixman renderer
 * @return Pointer to the newly created wlf_renderer structure
 */
struct wlf_renderer *wlf_pixman_renderer_create(void);

/**
 * @brief Checks if a renderer is a Pixman renderer
 * @param wlf_renderer Pointer to the base renderer
 * @return true if the renderer is a Pixman renderer, false otherwise
 */
bool wlf_renderer_is_pixman(struct wlf_renderer *wlf_renderer);

/**
 * @brief Checks if a texture is a Pixman texture
 * @param texture Pointer to the base texture
 * @return true if the texture is a Pixman texture, false otherwise
 */
bool wlf_texture_is_pixman(struct wlf_texture *texture);

/**
 * @brief Gets the Pixman image associated with a buffer in the Pixman renderer
 * @param wlf_renderer Pointer to the base renderer
 * @param wlf_buffer Pointer to the base buffer
 * @return Pointer to the associated Pixman image
 */
pixman_image_t *wlf_pixman_renderer_get_buffer_image(
	struct wlf_renderer *wlf_renderer, struct wlf_buffer *wlf_buffer);

/**
 * @brief Gets the Pixman image associated with a texture
 * @param wlf_texture Pointer to the base texture
 * @return Pointer to the associated Pixman image
 */
pixman_image_t *wlf_pixman_texture_get_image(struct wlf_texture *wlf_texture);

#endif

