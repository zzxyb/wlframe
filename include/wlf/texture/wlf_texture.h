#ifndef WLF_TEXTURE_H
#define WLF_TEXTURE_H

#include "wlf/render/wlf_buffer.h"
#include "wlf/math/wlf_rect.h"

#include <pixman.h>

struct wlf_buffer;
struct wlf_renderer;
struct wlf_texture_impl;

/**
 * @brief A structure representing a texture
 */
struct wlf_texture {
	const struct wlf_texture_impl *impl;  /**< Pointer to the texture implementation */
	uint32_t width;                        /**< Width of the texture */
	uint32_t height;                       /**< Height of the texture */
	struct wlf_renderer *renderer;         /**< Pointer to the associated renderer */
};

/**
 * @brief A structure representing options for reading pixels from a texture
 */
struct wlf_texture_read_pixels_options {
	void *data;        /**< Memory location to read pixels into */
	uint32_t format;   /**< Format used for writing the pixel data */
	uint32_t stride;   /**< Stride in bytes for the data */
	uint32_t dst_x;    /**< Destination X offset */
	uint32_t dst_y;    /**< Destination Y offset */
	const struct wlf_frect src_box; /**< Source box of the texture to read from. If empty, the full texture is assumed. */
};

/**
 * @brief Reads pixels from a texture into a specified memory location
 * @param texture Pointer to the texture to read from
 * @param options Pointer to the options for reading pixels
 * @return true if the operation was successful, false otherwise
 */
bool wlf_texture_read_pixels(struct wlf_texture *texture,
	const struct wlf_texture_read_pixels_options *options);

/**
 * @brief Gets the preferred read format for a texture
 * @param texture Pointer to the texture to query
 * @return The preferred read format for the texture
 */
uint32_t wlf_texture_preferred_read_format(struct wlf_texture *texture);

/**
 * @brief Creates a new texture from raw pixel data
 * @param renderer Pointer to the renderer to associate with the texture
 * @param fmt Format of the pixel data
 * @param stride Stride in bytes for the pixel data
 * @param width Width of the texture
 * @param height Height of the texture
 * @param data Pointer to the raw pixel data
 * @return Pointer to the newly created wlf_texture structure
 */
struct wlf_texture *wlf_texture_from_pixels(struct wlf_renderer *renderer,
	uint32_t fmt, uint32_t stride, 
	uint32_t width, uint32_t height,
	const void *data);

/**
 * @brief Creates a new texture from a DMA-BUF
 * @param renderer Pointer to the renderer to associate with the texture
 * @param attribs Pointer to the DMA-BUF attributes
 * @return Pointer to the newly created wlf_texture structure
 */
struct wlf_texture *wlf_texture_from_dmabuf(struct wlf_renderer *renderer,
	struct wlf_dmabuf_attributes *attribs);

/**
 * @brief Updates a texture with the contents of a buffer
 * @param texture Pointer to the texture to update
 * @param buffer Pointer to the buffer containing the new contents
 * @param damage Pointer to the region that needs to be updated
 * @return true if the update was successful, false otherwise
 */
bool wlf_texture_update_from_buffer(struct wlf_texture *texture,
	struct wlf_buffer *buffer, 
	const pixman_region32_t *damage);

/**
 * @brief Destroys a texture and frees associated resources
 * @param texture Pointer to the texture to destroy
 */
void wlf_texture_destroy(struct wlf_texture *texture);

/**
 * @brief Creates a new texture from a buffer
 * @param renderer Pointer to the renderer to associate with the texture
 * @param buffer Pointer to the buffer containing the texture data
 * @return Pointer to the newly created wlf_texture structure
 */
struct wlf_texture *wlf_texture_from_buffer(struct wlf_renderer *renderer,
	struct wlf_buffer *buffer);

#endif
