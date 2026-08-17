/**
 * @file        buffer.h
 * @brief       EGL-backed wlframe buffer.
 * @details     Declares the buffer wrapper used for EGL window surfaces.
 *              The EGL surface and its native window wrapper are owned by the
 *              buffer, while the EGL display and context remain owned by the
 *              GLES renderer.
 * @author      YaoBing Xiao
 * @date        2026-08-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-17, initial version\n
 */

#ifndef BUFFER_EGL_BUFFER_H
#define BUFFER_EGL_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/types/wlf_format_set.h"

#include <EGL/egl.h>

#include <stdbool.h>
#include <stdint.h>

struct wl_surface;
struct wl_egl_window;
struct wlf_egl;

/**
 * @brief Buffer backed by an EGL window surface.
 *
 * The EGL display is borrowed from the GLES renderer. The EGL surface and,
 * on Wayland, the wl_egl_window wrapper are owned by this buffer.
 */
struct wlf_egl_buffer {
	struct wlf_buffer base; /**< Generic buffer interface. */

	struct wlf_egl *egl; /**< EGL context owner, borrowed. */
	struct wl_egl_window *egl_window; /**< Native Wayland EGL window. */
	EGLConfig config; /**< EGL configuration selected for the surface. */
	EGLSurface surface; /**< EGL window surface. */
	bool swap_interval_configured; /**< Whether the EGL swap interval was set. */
};

/**
 * @brief Creates an EGL-backed buffer for a native Wayland surface.
 * @param egl EGL context used to create the surface.
 * @param surface Native Wayland surface receiving the EGL window.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested EGL render format.
 * @return Newly allocated generic buffer, or NULL on failure.
 */
struct wlf_buffer *wlf_egl_buffer_create(struct wlf_egl *egl,
	struct wl_surface *surface, uint32_t width, uint32_t height,
	const struct wlf_render_format *format);

/**
 * @brief Checks whether a generic buffer is EGL-backed.
 * @param buffer Buffer to inspect.
 * @return true when @p buffer is an EGL buffer, false otherwise.
 */
bool wlf_buffer_is_egl(const struct wlf_buffer *buffer);

/**
 * @brief Casts a generic buffer to an EGL buffer.
 * @param buffer Buffer known to be EGL-backed.
 * @return Enclosing EGL buffer, or NULL when the type does not match.
 */
struct wlf_egl_buffer *wlf_egl_buffer_from_buffer(struct wlf_buffer *buffer);

/**
 * @brief Resizes the native EGL window represented by a buffer.
 * @param buffer EGL buffer to resize.
 * @param width New width in pixels.
 * @param height New height in pixels.
 * @return true on success, false for invalid dimensions or a missing window.
 */
bool wlf_egl_buffer_resize(struct wlf_egl_buffer *buffer, uint32_t width, uint32_t height);

/**
 * @brief Returns the EGL context wrapper associated with a buffer.
 * @param buffer EGL buffer to inspect.
 * @return Borrowed EGL context wrapper, or NULL for a NULL buffer.
 */
struct wlf_egl *wlf_egl_buffer_get_egl(const struct wlf_egl_buffer *buffer);

/**
 * @brief Returns the EGL presentation surface owned by a buffer.
 * @param buffer EGL buffer to inspect.
 * @return EGL surface, or EGL_NO_SURFACE for a NULL buffer.
 */
EGLSurface wlf_egl_buffer_get_surface(const struct wlf_egl_buffer *buffer);

/**
 * @brief Configures the non-blocking EGL swap interval for a buffer.
 * @param buffer EGL buffer whose surface is current or will be used for swap.
 * @return true on success, false when EGL rejects the interval.
 */
bool wlf_egl_buffer_configure_swap_interval(struct wlf_egl_buffer *buffer);

#endif // BUFFER_EGL_BUFFER_H
