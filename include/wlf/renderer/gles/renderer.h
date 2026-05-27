/**
 * @file        renderer.h
 * @brief       GLES renderer for wlframe.
 * @details     This file defines the OpenGL ES rendering backend for wlframe.
 *              It wraps the base renderer interface and adds GLES-specific state:
 *              an EGL context, supported GL extension flags, and dynamically loaded
 *              GL extension function pointers.
 *
 *              The GLES renderer targets OpenGL ES 2.0 as the minimum version and
 *              optionally uses higher-version or extension features when available.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-27
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2026-05-27, initial version.
 */

#ifndef GLES_RENDERER_H
#define GLES_RENDERER_H

#include "wlf/renderer/wlf_renderer.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

struct wlf_backend;
struct wlf_egl;

/**
 * @struct wlf_gles_renderer
 * @brief OpenGL ES renderer backend for wlframe.
 *
 * Embeds the base @ref wlf_renderer and extends it with GLES-specific resources:
 * an EGL context for surface management, a set of supported GL extension flags,
 * and dynamically loaded function pointers for optional GL extensions.
 */
struct wlf_gles_renderer {
	struct wlf_renderer base; /**< Embedded base renderer; must be the first member. */

	struct wlf_egl *egl;      /**< EGL context used by this renderer. */
	const char *exts_str;     /**< Raw GL extensions string returned by glGetString(GL_EXTENSIONS). */

	struct {
		bool EXT_read_format_bgra;          /**< GL_EXT_read_format_bgra: read pixels in BGRA order. */
		bool KHR_debug;                     /**< GL_KHR_debug: debug message callbacks and groups. */
		bool OES_egl_image_external;        /**< GL_OES_EGL_image_external: external texture from EGLImage. */
		bool OES_egl_image;                 /**< GL_OES_EGL_image: 2-D texture and renderbuffer from EGLImage. */
		bool EXT_texture_type_2_10_10_10_REV; /**< GL_EXT_texture_type_2_10_10_10_REV: packed texture type. */
		bool OES_texture_half_float_linear; /**< GL_OES_texture_half_float_linear: linear filtering for fp16 textures. */
		bool EXT_texture_norm16;            /**< GL_EXT_texture_norm16: 16-bit normalised texture formats. */
		bool EXT_disjoint_timer_query;      /**< GL_EXT_disjoint_timer_query: GPU timestamp queries. */
	} exts;

	struct {
		PFNGLEGLIMAGETARGETTEXTURE2DOESPROC           glEGLImageTargetTexture2DOES;          /**< Attach a 2-D EGLImage as a texture. */
		PFNGLDEBUGMESSAGECALLBACKKHRPROC              glDebugMessageCallbackKHR;             /**< Register a debug message callback. */
		PFNGLDEBUGMESSAGECONTROLKHRPROC               glDebugMessageControlKHR;              /**< Filter which debug messages are generated. */
		PFNGLPOPDEBUGGROUPKHRPROC                     glPopDebugGroupKHR;                    /**< Pop the active debug group. */
		PFNGLPUSHDEBUGGROUPKHRPROC                    glPushDebugGroupKHR;                   /**< Push a named debug group. */
		PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glEGLImageTargetRenderbufferStorageOES;/**< Attach an EGLImage as renderbuffer storage. */
		PFNGLGETGRAPHICSRESETSTATUSKHRPROC            glGetGraphicsResetStatusKHR;           /**< Query whether the GL context has been reset. */
		PFNGLGENQUERIESEXTPROC                        glGenQueriesEXT;                       /**< Generate timer query objects. */
		PFNGLDELETEQUERIESEXTPROC                     glDeleteQueriesEXT;                    /**< Delete timer query objects. */
		PFNGLQUERYCOUNTEREXTPROC                      glQueryCounterEXT;                     /**< Record a GPU timestamp into a query object. */
		PFNGLGETQUERYOBJECTIVEXTPROC                  glGetQueryObjectivEXT;                 /**< Retrieve a signed integer query result. */
		PFNGLGETQUERYOBJECTUI64VEXTPROC               glGetQueryObjectui64vEXT;              /**< Retrieve a 64-bit unsigned integer query result. */
		PFNGLGETINTEGER64VEXTPROC                     glGetInteger64vEXT;                    /**< Retrieve a 64-bit integer GL parameter. */
	} procs;
};

/**
 * @brief Creates a GLES renderer for the given backend.
 *
 * Initialises an EGL context, queries GL capabilities, loads extension
 * function pointers, and returns a fully initialised GLES renderer.
 * If @c WLF_RENDER_FORCE_SOFTWARE is set, the function forces Mesa software
 * rasterisation and fails if the resulting driver is hardware-backed.
 *
 * @param backend Pointer to the backend to create the renderer for.
 * @return Pointer to the base renderer interface, or NULL on failure.
 */
struct wlf_renderer *wlf_gles_renderer_create_from_backend(
	struct wlf_backend *backend);

/**
 * @brief Returns true if the renderer is a GLES renderer.
 * @param renderer Pointer to the renderer to test.
 * @return true if @p renderer was created by wlf_gles_renderer_create_from_backend().
 */
bool wlf_renderer_is_gles(const struct wlf_renderer *renderer);

/**
 * @brief Downcasts a base renderer pointer to a GLES renderer pointer.
 * @param renderer Pointer to the base renderer.
 * @return Pointer to the enclosing wlf_gles_renderer, or NULL if the renderer
 *         is not a GLES renderer.
 */
struct wlf_gles_renderer *wlf_gles_renderer_from_renderer(struct wlf_renderer *renderer);

/**
 * @brief Checks whether a GL extension is supported by this renderer.
 *
 * Searches @c renderer->exts_str for @p ext using whole-word matching.
 * This is useful for querying extensions not covered by the hardcoded
 * flags in @ref wlf_gles_renderer::exts.
 *
 * @param renderer Pointer to the GLES renderer.
 * @param ext      Name of the GL extension to look up (e.g. "GL_KHR_debug").
 * @return true if the extension is present, false if it is absent or if
 *         the extension string was not available at renderer creation time.
 */
bool wlf_gles_renderer_check_ext(const struct wlf_gles_renderer *renderer,
	const char *ext);

/**
 * @brief Returns a human-readable string for a GL error code.
 * @param error GL error code returned by glGetError().
 * @return Pointer to a static string describing the error,
 *         or "unknown error" for unrecognised codes.
 */
const char *wlf_gles_error_str(GLenum error);

#endif // GLES_RENDERER_H
