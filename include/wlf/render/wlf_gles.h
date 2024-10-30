#ifndef WLF_GLES2_H
#define WLF_GLES2_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/**
 * @brief A function pointer type for retrieving a 64-bit integer from OpenGL ES
 */
typedef void (GL_APIENTRYP PFNGLGETINTEGER64VEXTPROC) (GLenum pname, GLint64 *data);

/**
 * @brief A structure representing a pixel format for OpenGL ES 2
 */
struct wlf_gles2_pixel_format {
	uint32_t drm_format;        /**< DRM format identifier */
	GLint gl_internalformat;    /**< OpenGL internal format; optional field, if empty then internalformat = format */
	GLint gl_format;            /**< OpenGL format */
	GLint gl_type;              /**< OpenGL type */
};

/**
 * @brief A structure representing a texture shader for OpenGL ES 2
 */
struct wlf_gles2_tex_shader {
	GLuint program;             /**< Shader program identifier */
	GLint proj;                 /**< Projection matrix uniform location */
	GLint tex_proj;             /**< Texture projection uniform location */
	GLint tex;                  /**< Texture uniform location */
	GLint alpha;                /**< Alpha uniform location */
	GLint pos_attrib;           /**< Position attribute location */
};

/**
 * @brief A structure representing an OpenGL ES 2 renderer
 */
struct wlf_gles2_renderer {
	struct wlf_renderer wlf_renderer; /**< Base renderer structure */

	struct wlf_egl *egl;              /**< Pointer to the associated EGL context */
	int drm_fd;                       /**< DRM file descriptor */

	struct wlf_drm_format_set shm_texture_formats; /**< Set of shared memory texture formats */

	const char *exts_str;             /**< String of supported extensions */
	struct {
		bool EXT_read_format_bgra;    /**< Indicates support for EXT_read_format_bgra extension */
		bool KHR_debug;               /**< Indicates support for KHR_debug extension */
		bool OES_egl_image_external;  /**< Indicates support for OES_egl_image_external extension */
		bool OES_egl_image;           /**< Indicates support for OES_egl_image extension */
		bool EXT_texture_type_2_10_10_10_REV; /**< Indicates support for EXT_texture_type_2_10_10_10_REV extension */
		bool OES_texture_half_float_linear; /**< Indicates support for OES_texture_half_float_linear extension */
		bool EXT_texture_norm16;      /**< Indicates support for EXT_texture_norm16 extension */
		bool EXT_disjoint_timer_query; /**< Indicates support for EXT_disjoint_timer_query extension */
	} exts;

	struct {
		PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES; /**< Function pointer for glEGLImageTargetTexture2DOES */
		PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR; /**< Function pointer for glDebugMessageCallbackKHR */
		PFNGLDEBUGMESSAGECONTROLKHRPROC glDebugMessageControlKHR; /**< Function pointer for glDebugMessageControlKHR */
		PFNGLPOPDEBUGGROUPKHRPROC glPopDebugGroupKHR; /**< Function pointer for glPopDebugGroupKHR */
		PFNGLPUSHDEBUGGROUPKHRPROC glPushDebugGroupKHR; /**< Function pointer for glPushDebugGroupKHR */
		PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glEGLImageTargetRenderbufferStorageOES; /**< Function pointer for glEGLImageTargetRenderbufferStorageOES */
		PFNGLGETGRAPHICSRESETSTATUSKHRPROC glGetGraphicsResetStatusKHR; /**< Function pointer for glGetGraphicsResetStatusKHR */
		PFNGLGENQUERIESEXTPROC glGenQueriesEXT; /**< Function pointer for glGenQueriesEXT */
		PFNGLDELETEQUERIESEXTPROC glDeleteQueriesEXT; /**< Function pointer for glDeleteQueriesEXT */
		PFNGLQUERYCOUNTEREXTPROC glQueryCounterEXT; /**< Function pointer for glQueryCounterEXT */
		PFNGLGETQUERYOBJECTIVEXTPROC glGetQueryObjectivEXT; /**< Function pointer for glGetQueryObjectivEXT */
		PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT; /**< Function pointer for glGetQueryObjectui64vEXT */
		PFNGLGETINTEGER64VEXTPROC glGetInteger64vEXT; /**< Function pointer for glGetInteger64vEXT */
	} procs;

	bool has_modifiers; /**< Indicates if modifiers are supported */
	struct wlf_drm_format_set dmabuf_texture_formats; /**< Set of DMA-BUF formats for textures */
	struct wlf_drm_format_set dmabuf_render_formats; /**< Set of DMA-BUF formats for rendering */
};

/**
 * @brief A structure representing a render timer for OpenGL ES 2
 */
struct wlf_gles2_render_timer {
	struct wlf_render_timer base; /**< Base render timer structure */
	struct wlf_gles2_renderer *renderer; /**< Pointer to the associated OpenGL ES 2 renderer */
	struct timespec cpu_start; /**< Start time for CPU timing */
	struct timespec cpu_end; /**< End time for CPU timing */
	GLuint id; /**< Identifier for the render timer */
	GLint64 gl_cpu_end; /**< End time for OpenGL ES 2 CPU timing */
};

/**
 * @brief A structure representing a buffer for OpenGL ES 2
 */
struct wlf_gles2_buffer {
	struct wlf_buffer *buffer; /**< Pointer to the associated buffer */
	struct wlf_gles2_renderer *renderer; /**< Pointer to the associated OpenGL ES 2 renderer */
	struct wl_list link; /**< Link for the buffer in the renderer's list */
	bool external_only; /**< Indicates if the buffer is external only */

	EGLImageKHR image; /**< EGL image associated with the buffer */
	GLuint rbo; /**< Renderbuffer object identifier */
	GLuint fbo; /**< Framebuffer object identifier */
	GLuint tex; /**< Texture identifier */

	struct wlf_addon addon; /**< Addon associated with the buffer */
};

/**
 * @brief A structure representing a texture for OpenGL ES 2
 */
struct wlf_gles2_texture {
	struct wlf_texture wlf_texture; /**< Base texture structure */
	struct wlf_gles2_renderer *renderer; /**< Pointer to the associated OpenGL ES 2 renderer */
	struct wl_list link; /**< Link for the texture in the renderer's list */

	GLenum target; /**< Target for the texture */

	// If this texture is imported from a buffer, the texture does not own
	// these states. These cannot be destroyed along with the texture in this
	// case.
	GLuint tex; /**< Texture identifier */
	GLuint fbo; /**< Framebuffer object identifier */

	bool has_alpha; /**< Indicates if the texture has an alpha channel */

	uint32_t drm_format; /**< DRM format for mutable textures only, used to interpret upload data */
	struct wlf_gles2_buffer *buffer; /**< Pointer to the associated buffer for DMA-BUF imports only */
};

/**
 * @brief A structure representing a render pass for OpenGL ES 2
 */
struct wlf_gles2_render_pass {
	struct wlf_render_pass base; /**< Base render pass structure */
	struct wlf_gles2_buffer *buffer; /**< Pointer to the associated OpenGL ES 2 buffer */
	float projection_matrix[9]; /**< Projection matrix for the render pass */
	struct wlf_egl_context prev_ctx; /**< Previous EGL context */
	struct wlf_gles2_render_timer *timer; /**< Pointer to the associated render timer */
	struct wlf_drm_syncobj_timeline *signal_timeline; /**< Pointer to the signal timeline */
	uint64_t signal_point; /**< Signal point for synchronization */
};

/**
 * @brief Checks if a given pixel format is supported by the OpenGL ES 2 renderer
 * @param renderer Pointer to the OpenGL ES 2 renderer
 * @param format Pointer to the pixel format to check
 * @return true if the pixel format is supported, false otherwise
 */
bool is_gles2_pixel_format_supported(const struct wlf_gles2_renderer *renderer,
	const struct wlf_gles2_pixel_format *format);

/**
 * @brief Gets the OpenGL ES 2 pixel format from a DRM format
 * @param fmt The DRM format to convert
 * @return Pointer to the corresponding OpenGL ES 2 pixel format
 */
const struct wlf_gles2_pixel_format *get_gles2_format_from_drm(uint32_t fmt);

/**
 * @brief Gets the OpenGL ES 2 pixel format from OpenGL format and type
 * @param gl_format The OpenGL format
 * @param gl_type The OpenGL type
 * @param alpha Indicates if the format includes an alpha channel
 * @return Pointer to the corresponding OpenGL ES 2 pixel format
 */
const struct wlf_gles2_pixel_format *get_gles2_format_from_gl(
	GLint gl_format, GLint gl_type, bool alpha);

/**
 * @brief Gets the shared memory formats for OpenGL ES 2
 * @param renderer Pointer to the OpenGL ES 2 renderer
 * @param out Pointer to the output format set
 */
void get_gles2_shm_formats(const struct wlf_gles2_renderer *renderer,
	struct wlf_drm_format_set *out);

/**
 * @brief Gets the framebuffer object identifier for a given buffer
 * @param buffer Pointer to the OpenGL ES 2 buffer
 * @return The framebuffer object identifier
 */
GLuint gles2_buffer_get_fbo(struct wlf_gles2_buffer *buffer);

/**
 * @brief Gets the OpenGL ES 2 renderer from a base renderer
 * @param wlf_renderer Pointer to the base renderer
 * @return Pointer to the OpenGL ES 2 renderer
 */
struct wlf_gles2_renderer *gles2_get_renderer(
	struct wlf_renderer *wlf_renderer);

/**
 * @brief Gets the render timer for OpenGL ES 2
 * @param timer Pointer to the base render timer
 * @return Pointer to the OpenGL ES 2 render timer
 */
struct wlf_gles2_render_timer *gles2_get_render_timer(
	struct wlf_render_timer *timer);

/**
 * @brief Gets the OpenGL ES 2 texture from a base texture
 * @param wlf_texture Pointer to the base texture
 * @return Pointer to the OpenGL ES 2 texture
 */
struct wlf_gles2_texture *gles2_get_texture(
	struct wlf_texture *wlf_texture);

/**
 * @brief Gets or creates a buffer for OpenGL ES 2
 * @param renderer Pointer to the OpenGL ES 2 renderer
 * @param wlf_buffer Pointer to the base buffer
 * @return Pointer to the OpenGL ES 2 buffer
 */
struct wlf_gles2_buffer *gles2_buffer_get_or_create(struct wlf_gles2_renderer *renderer,
	struct wlf_buffer *wlf_buffer);

/**
 * @brief Creates a texture from a buffer for OpenGL ES 2
 * @param wlf_renderer Pointer to the base renderer
 * @param buffer Pointer to the base buffer
 * @return Pointer to the created texture
 */
struct wlf_texture *gles2_texture_from_buffer(struct wlf_renderer *wlf_renderer,
	struct wlf_buffer *buffer);

/**
 * @brief Destroys an OpenGL ES 2 texture
 * @param texture Pointer to the OpenGL ES 2 texture to destroy
 */
void gles2_texture_destroy(struct wlf_gles2_texture *texture);

/**
 * @brief Pushes a debug message for OpenGL ES 2
 * @param renderer Pointer to the OpenGL ES 2 renderer
 * @param file The source file name
 * @param func The function name
 */
void push_gles2_debug_(struct wlf_gles2_renderer *renderer,
	const char *file, const char *func);

/**
 * @brief Macro to push a debug message for OpenGL ES 2
 */
#define push_gles2_debug(renderer) push_gles2_debug_(renderer, _WLR_FILENAME, __func__)

/**
 * @brief Pops a debug message for OpenGL ES 2
 * @param renderer Pointer to the OpenGL ES 2 renderer
 */
void pop_gles2_debug(struct wlf_gles2_renderer *renderer);

/**
 * @brief Begins a buffer pass for OpenGL ES 2
 * @param buffer Pointer to the OpenGL ES 2 buffer
 * @param prev_ctx Pointer to the previous EGL context
 * @param timer Pointer to the OpenGL ES 2 render timer
 * @param signal_timeline Pointer to the signal timeline
 * @param signal_point The signal point for synchronization
 * @return Pointer to the created render pass
 */
struct wlf_gles2_render_pass *begin_gles2_buffer_pass(struct wlf_gles2_buffer *buffer,
	struct wlf_egl_context *prev_ctx,
	struct wlf_gles2_render_timer *timer,
	struct wlf_drm_syncobj_timeline *signal_timeline,
	uint64_t signal_point);

/**
 * @brief OpenGL ES 2 renderer.
 *
 * Care must be taken to avoid stepping each other's toes with EGL contexts:
 * the current EGL is global state. The GLES2 renderer operations will save
 * and restore any previous EGL context when called. A render pass is seen as
 * a single operation.
 *
 * The GLES2 renderer doesn't support arbitrarily nested render passes. It
 * supports a subset only: after a nested render pass is created, any parent
 * render pass can't be used before the nested render pass is submitted.
 */

/**
 * @brief Creates a new OpenGL ES 2 renderer with a given DRM file descriptor
 * @param drm_fd The DRM file descriptor to use
 * @return Pointer to the newly created wlf_renderer structure
 */
struct wlf_renderer *wlf_gles2_renderer_create_with_drm_fd(int drm_fd);

/**
 * @brief Creates a new OpenGL ES 2 renderer with a given EGL context
 * @param egl Pointer to the EGL context to use
 * @return Pointer to the newly created wlf_renderer structure
 */
struct wlf_renderer *wlf_gles2_renderer_create(struct wlf_egl *egl);

/**
 * @brief Gets the EGL context associated with the OpenGL ES 2 renderer
 * @param renderer Pointer to the base renderer
 * @return Pointer to the associated EGL context
 */
struct wlf_egl *wlf_gles2_renderer_get_egl(struct wlf_renderer *renderer);

/**
 * @brief Checks if a specific extension is supported by the OpenGL ES 2 renderer
 * @param renderer Pointer to the base renderer
 * @param ext The extension to check for
 * @return true if the extension is supported, false otherwise
 */
bool wlf_gles2_renderer_check_ext(struct wlf_renderer *renderer, const char *ext);

/**
 * @brief Gets the framebuffer object identifier for a buffer in the OpenGL ES 2 renderer
 * @param renderer Pointer to the base renderer
 * @param buffer Pointer to the base buffer
 * @return The framebuffer object identifier
 */
GLuint wlf_gles2_renderer_get_buffer_fbo(struct wlf_renderer *renderer, struct wlf_buffer *buffer);

/**
 * @brief A structure representing texture attributes for OpenGL ES 2
 */
struct wlf_gles2_texture_attribs {
	GLenum target; /**< Either GL_TEXTURE_2D or GL_TEXTURE_EXTERNAL_OES */
	GLuint tex;   /**< Texture identifier */
	bool has_alpha; /**< Indicates if the texture has an alpha channel */
};

/**
 * @brief Checks if a renderer is an OpenGL ES 2 renderer
 * @param wlf_renderer Pointer to the base renderer
 * @return true if the renderer is an OpenGL ES 2 renderer, false otherwise
 */
bool wlf_renderer_is_gles2(struct wlf_renderer *wlf_renderer);

/**
 * @brief Checks if a render timer is for OpenGL ES 2
 * @param timer Pointer to the base render timer
 * @return true if the timer is for OpenGL ES 2, false otherwise
 */
bool wlf_render_timer_is_gles2(struct wlf_render_timer *timer);

/**
 * @brief Checks if a texture is an OpenGL ES 2 texture
 * @param texture Pointer to the base texture
 * @return true if the texture is an OpenGL ES 2 texture, false otherwise
 */
bool wlf_texture_is_gles2(struct wlf_texture *texture);

/**
 * @brief Gets the texture attributes for an OpenGL ES 2 texture
 * @param texture Pointer to the base texture
 * @param attribs Pointer to the structure to fill with texture attributes
 */
void wlf_gles2_texture_get_attribs(struct wlf_texture *texture,
	struct wlf_gles2_texture_attribs *attribs);

#endif
