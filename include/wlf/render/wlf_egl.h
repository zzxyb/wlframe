#ifndef WLF_RENDER_EGL_H
#define WLF_RENDER_EGL_H

#include "wlf/render/wlf_drm_format_set.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

/**
 * @brief A structure representing an EGL context and its associated resources
 */
struct wlf_egl {
	EGLDisplay display;  /**< EGL display connection */
	EGLContext context;  /**< EGL rendering context */
	EGLDeviceEXT device; /**< EGL device, may be EGL_NO_DEVICE_EXT */
	struct gbm_device *gbm_device; /**< Pointer to the GBM device */

	struct {
		// Display extensions
		bool KHR_image_base; /**< Indicates support for KHR_image_base extension */
		bool EXT_image_dma_buf_import; /**< Indicates support for EXT_image_dma_buf_import extension */
		bool EXT_image_dma_buf_import_modifiers; /**< Indicates support for EXT_image_dma_buf_import_modifiers extension */
		bool IMG_context_priority; /**< Indicates support for IMG_context_priority extension */
		bool EXT_create_context_robustness; /**< Indicates support for EXT_create_context_robustness extension */

		// Device extensions
		bool EXT_device_drm; /**< Indicates support for EXT_device_drm extension */
		bool EXT_device_drm_render_node; /**< Indicates support for EXT_device_drm_render_node extension */

		// Client extensions
		bool EXT_device_query; /**< Indicates support for EXT_device_query extension */
		bool KHR_platform_gbm; /**< Indicates support for KHR_platform_gbm extension */
		bool EXT_platform_device; /**< Indicates support for EXT_platform_device extension */
		bool KHR_display_reference; /**< Indicates support for KHR_display_reference extension */
	} exts;

	struct {
		PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT; /**< Function pointer for eglGetPlatformDisplayEXT */
		PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR; /**< Function pointer for eglCreateImageKHR */
		PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR; /**< Function pointer for eglDestroyImageKHR */
		PFNEGLQUERYDMABUFFORMATSEXTPROC eglQueryDmaBufFormatsEXT; /**< Function pointer for eglQueryDmaBufFormatsEXT */
		PFNEGLQUERYDMABUFMODIFIERSEXTPROC eglQueryDmaBufModifiersEXT; /**< Function pointer for eglQueryDmaBufModifiersEXT */
		PFNEGLDEBUGMESSAGECONTROLKHRPROC eglDebugMessageControlKHR; /**< Function pointer for eglDebugMessageControlKHR */
		PFNEGLQUERYDISPLAYATTRIBEXTPROC eglQueryDisplayAttribEXT; /**< Function pointer for eglQueryDisplayAttribEXT */
		PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT; /**< Function pointer for eglQueryDeviceStringEXT */
		PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT; /**< Function pointer for eglQueryDevicesEXT */
		PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR; /**< Function pointer for eglCreateSyncKHR */
		PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR; /**< Function pointer for eglDestroySyncKHR */
		PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID; /**< Function pointer for eglDupNativeFenceFDANDROID */
		PFNEGLWAITSYNCKHRPROC eglWaitSyncKHR; /**< Function pointer for eglWaitSyncKHR */
	} procs;

	bool has_modifiers; /**< Indicates if modifiers are supported */
	struct wlf_drm_format_set dmabuf_texture_formats; /**< Set of DMA-BUF formats for textures */
	struct wlf_drm_format_set dmabuf_render_formats; /**< Set of DMA-BUF formats for rendering */
};

/**
 * @brief A structure representing an EGL context for rendering
 */
struct wlf_egl_context {
	EGLDisplay display;  /**< EGL display connection */
	EGLContext context;  /**< EGL rendering context */
	EGLSurface draw_surface; /**< EGL surface for drawing */
	EGLSurface read_surface; /**< EGL surface for reading */
};

/**
 * @brief Initializes an EGL context for the given DRM file descriptor
 * @param drm_fd The DRM file descriptor to use
 * @return Pointer to the newly created wlf_egl structure
 *
 * Will attempt to load all possibly required API functions.
 */
struct wlf_egl *wlf_egl_create_with_drm_fd(int drm_fd);

/**
 * @brief Frees all related EGL resources, makes the context not current, and unbinds a bound Wayland display
 * @param egl Pointer to the wlf_egl structure to destroy
 */
void wlf_egl_destroy(struct wlf_egl *egl);

/**
 * @brief Creates an EGL image from the given DMA-BUF attributes
 * @param egl Pointer to the wlf_egl structure
 * @param attributes Pointer to the DMA-BUF attributes
 * @param external_only Pointer to a boolean indicating if only external buffers are allowed
 * @return The created EGLImageKHR
 *
 * Check usability of the DMA-BUF with wlf_egl_check_import_dmabuf once first.
 */
EGLImageKHR wlf_egl_create_image_from_dmabuf(struct wlf_egl *egl,
	struct wlf_dmabuf_attributes *attributes, bool *external_only);

/**
 * @brief Gets DMA-BUF formats suitable for sampling usage
 * @param egl Pointer to the wlf_egl structure
 * @return Pointer to the set of DMA-BUF formats for textures
 */
const struct wlf_drm_format_set *wlf_egl_get_dmabuf_texture_formats(
	struct wlf_egl *egl);

/**
 * @brief Gets DMA-BUF formats suitable for rendering usage
 * @param egl Pointer to the wlf_egl structure
 * @return Pointer to the set of DMA-BUF formats for rendering
 */
const struct wlf_drm_format_set *wlf_egl_get_dmabuf_render_formats(
	struct wlf_egl *egl);

/**
 * @brief Destroys an EGL image created with the given wlf_egl
 * @param egl Pointer to the wlf_egl structure
 * @param image The EGL image to destroy
 * @return true if the destruction was successful, false otherwise
 */
bool wlf_egl_destroy_image(struct wlf_egl *egl, EGLImageKHR image);

/**
 * @brief Duplicates the DRM file descriptor associated with the EGL context
 * @param egl Pointer to the wlf_egl structure
 * @return The duplicated DRM file descriptor
 */
int wlf_egl_dup_drm_fd(struct wlf_egl *egl);

/**
 * @brief Restores an EGL context that was previously saved using wlf_egl_save_current()
 * @param context Pointer to the wlf_egl_context structure to restore
 * @return true if the restoration was successful, false otherwise
 */
bool wlf_egl_restore_context(struct wlf_egl_context *context);

/**
 * @brief Makes the EGL context current
 * @param egl Pointer to the wlf_egl structure
 * @param save_context Pointer to the wlf_egl_context structure to save the old context
 * @return true if the operation was successful, false otherwise
 *
 * The old EGL context is saved. Callers are expected to clear the current
 * context when they are done by calling wlf_egl_restore_context().
 */
bool wlf_egl_make_current(struct wlf_egl *egl, struct wlf_egl_context *save_context);

/**
 * @brief Unsets the current EGL context
 * @param egl Pointer to the wlf_egl structure
 * @return true if the operation was successful, false otherwise
 */
bool wlf_egl_unset_current(struct wlf_egl *egl);

/**
 * @brief Creates a synchronization object for the EGL context
 * @param egl Pointer to the wlf_egl structure
 * @param fence_fd The file descriptor for the fence
 * @return The created EGLSyncKHR object
 */
EGLSyncKHR wlf_egl_create_sync(struct wlf_egl *egl, int fence_fd);

/**
 * @brief Destroys a synchronization object
 * @param egl Pointer to the wlf_egl structure
 * @param sync The EGLSyncKHR object to destroy
 */
void wlf_egl_destroy_sync(struct wlf_egl *egl, EGLSyncKHR sync);

/**
 * @brief Duplicates a fence file descriptor
 * @param egl Pointer to the wlf_egl structure
 * @param sync The EGLSyncKHR object associated with the fence
 * @return The duplicated fence file descriptor
 */
int wlf_egl_dup_fence_fd(struct wlf_egl *egl, EGLSyncKHR sync);

/**
 * @brief Waits for a synchronization object to be signaled
 * @param egl Pointer to the wlf_egl structure
 * @param sync The EGLSyncKHR object to wait on
 * @return true if the wait was successful, false otherwise
 */
bool wlf_egl_wait_sync(struct wlf_egl *egl, EGLSyncKHR sync);

/**
 * @brief Creates a struct wlf_egl with an existing EGL display and context
 * @param display The existing EGL display
 * @param context The existing EGL context
 * @return Pointer to the newly created wlf_egl structure
 *
 * This is typically used by compositors which want to customize EGL
 * initialization.
 */
struct wlf_egl *wlf_egl_create_with_context(EGLDisplay display, EGLContext context);

/**
 * @brief Gets the EGL display used by the struct wlf_egl
 * @param egl Pointer to the wlf_egl structure
 * @return The EGL display associated with the wlf_egl structure
 *
 * This is typically used by compositors which need to perform custom OpenGL
 * operations.
 */
EGLDisplay wlf_egl_get_display(struct wlf_egl *egl);

/**
 * @brief Gets the EGL context used by the struct wlf_egl
 * @param egl Pointer to the wlf_egl structure
 * @return The EGL context associated with the wlf_egl structure
 *
 * This is typically used by compositors which need to perform custom OpenGL
 * operations.
 */
EGLContext wlf_egl_get_context(struct wlf_egl *egl);

#endif
