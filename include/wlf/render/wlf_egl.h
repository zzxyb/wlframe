/**
 * @file    wlf_egl.h
 * @brief   EGL context and resource management for wlframe.
 * @details This file provides structures and functions for managing EGL contexts,
 *          devices, extensions, and related resources. It includes helpers for
 *          context creation, destruction, extension querying, synchronization,
 *          and context switching. The interface is designed to simplify EGL
 *          integration and extension handling in Wayland compositors and clients.
 *
 * @author  YaoBing Xiao
 * @date    2024-05-20
 * @version v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef RENDER_WLF_EGL_H
#define RENDER_WLF_EGL_H

#include <stdbool.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

/**
 * @brief Structure representing an EGL context and its associated resources.
 */
struct wlf_egl {
	EGLDisplay display;  /**< EGL display connection */
	EGLContext context;  /**< EGL rendering context */
	EGLDeviceEXT device; /**< EGL device, may be EGL_NO_DEVICE_EXT */

	struct {
		// Display extensions
		bool KHR_platform_wayland; /**< Indicates support for EGL_KHR_platform_wayland extension */

		// Client extensions
		bool EXT_device_query; /**< Indicates support for EXT_device_query extension */
		bool EXT_platform_device; /**< Indicates support for EXT_platform_device extension */
		bool KHR_display_reference; /**< Indicates support for KHR_display_reference extension */

		// Debug extensions
		bool KHR_debug; /**< Indicates support for KHR_debug extension */
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
};

/**
 * @brief Structure representing an EGL context for rendering.
 */
struct wlf_egl_context {
	EGLDisplay display;      /**< EGL display connection */
	EGLContext context;      /**< EGL rendering context */
	EGLSurface draw_surface; /**< EGL surface for drawing */
	EGLSurface read_surface; /**< EGL surface for reading */
};

/**
 * @brief Create a new wlf_egl object and initialize EGL resources.
 * @param native_display Native display type for EGL initialization.
 * @return Pointer to the newly created wlf_egl structure.
 */
struct wlf_egl *wlf_egl_create(NativeDisplayType native_display);

/**
 * @brief Create a wlf_egl object with an existing EGL display and context.
 * @param display The existing EGL display.
 * @param context The existing EGL context.
 * @return Pointer to the newly created wlf_egl structure.
 *
 * This is typically used by compositors which want to customize EGL initialization.
 */
struct wlf_egl *wlf_egl_create_with_context(EGLDisplay display, EGLContext context);

/**
 * @brief Destroy a wlf_egl object and free all associated EGL resources.
 * @param egl Pointer to the wlf_egl structure to destroy.
 */
void wlf_egl_destroy(struct wlf_egl *egl);

/**
 * @brief Destroy an EGL image created with the given wlf_egl.
 * @param egl Pointer to the wlf_egl structure.
 * @param image The EGL image to destroy.
 * @return true if the destruction was successful, false otherwise.
 */
bool wlf_egl_destroy_image(struct wlf_egl *egl, EGLImageKHR image);

/**
 * @brief Make the EGL context current and save the previous context.
 * @param egl Pointer to the wlf_egl structure.
 * @param save_context Pointer to the wlf_egl_context structure to save the old context.
 * @return true if the operation was successful, false otherwise.
 *
 * The old EGL context is saved. Callers are expected to clear the current
 * context when they are done by calling wlf_egl_restore_context().
 */
bool wlf_egl_make_current(struct wlf_egl *egl, struct wlf_egl_context *save_context);

/**
 * @brief Create a synchronization object for the EGL context.
 * @param egl Pointer to the wlf_egl structure.
 * @param fence_fd The file descriptor for the fence.
 * @return The created EGLSyncKHR object.
 */
EGLSyncKHR wlf_egl_create_sync(struct wlf_egl *egl, int fence_fd);

/**
 * @brief Destroy a synchronization object.
 * @param egl Pointer to the wlf_egl structure.
 * @param sync The EGLSyncKHR object to destroy.
 */
void wlf_egl_destroy_sync(struct wlf_egl *egl, EGLSyncKHR sync);

/**
 * @brief Wait for a synchronization object to be signaled.
 * @param egl Pointer to the wlf_egl structure.
 * @param sync The EGLSyncKHR object to wait on.
 * @return true if the wait was successful, false otherwise.
 */
bool wlf_egl_wait_sync(struct wlf_egl *egl, EGLSyncKHR sync);

/**
 * @brief Get the EGL display used by the wlf_egl structure.
 * @param egl Pointer to the wlf_egl structure.
 * @return The EGL display associated with the wlf_egl structure.
 *
 * This is typically used by compositors which need to perform custom OpenGL operations.
 */
EGLDisplay wlf_egl_get_display(struct wlf_egl *egl);

/**
 * @brief Get the EGL context used by the wlf_egl structure.
 * @param egl Pointer to the wlf_egl structure.
 * @return The EGL context associated with the wlf_egl structure.
 *
 * This is typically used by compositors which need to perform custom OpenGL operations.
 */
EGLContext wlf_egl_get_context(struct wlf_egl *egl);

#endif // RENDER_WLF_EGL_H
