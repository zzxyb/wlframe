/**
 * @file        texture.h
 * @brief       GLES texture implementation and pixel-format mapping.
 * @details     Describes how wlframe formats map to GLES upload parameters
 *              and exposes the GLES texture wrapper.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef TEXTURE_GLES_TEXTURE_H
#define TEXTURE_GLES_TEXTURE_H

#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_linked_list.h"

#include <GLES2/gl2.h>
#include <stdbool.h>
#include <stdint.h>

struct wlf_gles_renderer;

/**
 * @brief Describes how a wlframe pixel format is uploaded to GLES.
 *
 * The mapping is used both when creating storage and when uploading pixel
 * data with glTexImage2D.
 */
struct wlf_gles_pixel_format {
	uint32_t format; /**< wlframe pixel-format identifier. */
	GLint gl_internalformat; /**< GLES internal texture format. */
	GLint gl_format; /**< GLES source pixel layout. */
	GLint gl_type; /**< GLES source component type. */
};

/**
 * @brief GLES texture wrapper.
 *
 * The wrapper owns the GLES texture object and is linked into its renderer's
 * texture list for cleanup.
 */
struct wlf_gles_texture {
	struct wlf_texture base; /**< Generic texture interface. */
	struct wlf_gles_renderer *renderer; /**< Renderer owning the GL object. */
	struct wlf_linked_list link; /**< Link in the renderer texture list. */
	GLuint tex; /**< GLES texture object. */
};

/**
 * @brief Checks whether a texture is backed by GLES.
 * @param texture Generic texture to inspect.
 * @return true when @p texture is GLES-backed, false otherwise.
 */
bool wlf_texture_is_gles(const struct wlf_texture *texture);

/**
 * @brief Casts a generic texture to a GLES texture.
 * @param texture Texture known to be GLES-backed.
 * @return Enclosing GLES texture, or NULL when the type does not match.
 */
struct wlf_gles_texture *wlf_gles_texture_from_texture(
	struct wlf_texture *texture);

/**
 * @brief Creates a GLES texture from a compatible wlframe buffer.
 * @param renderer GLES renderer that owns the texture.
 * @param buffer Buffer containing the source pixels.
 * @return Newly created generic texture, or NULL on failure.
 */
struct wlf_texture *wlf_gles_texture_from_buffer(
	struct wlf_gles_renderer *renderer, struct wlf_buffer *buffer);

/**
 * @brief Looks up GLES upload parameters for a wlframe format.
 * @param format wlframe pixel-format identifier.
 * @return Static mapping, or NULL when the format is not known.
 */
const struct wlf_gles_pixel_format *wlf_gles_pixel_format_from_wlf(
	uint32_t format);

/**
 * @brief Checks whether a mapped pixel format is supported by @p renderer.
 * @param renderer GLES renderer to query.
 * @param format Pixel-format mapping to test.
 * @return true when the renderer supports the mapping, false otherwise.
 */
bool wlf_gles_pixel_format_is_supported(
	const struct wlf_gles_renderer *renderer,
	const struct wlf_gles_pixel_format *format);

/**
 * @brief Looks up a wlframe format from GLES source parameters.
 * @param gl_format GLES source pixel layout.
 * @param gl_type GLES source component type.
 * @param alpha Whether the source contains alpha.
 * @return Static mapping, or NULL when no wlframe format matches.
 */
const struct wlf_gles_pixel_format *wlf_gles_pixel_format_from_gl(
	GLint gl_format, GLint gl_type, bool alpha);

#endif // TEXTURE_GLES_TEXTURE_H
