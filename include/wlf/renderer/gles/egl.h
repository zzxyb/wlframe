/**
 * @file        egl.h
 * @brief       EGL context management for the wlframe GLES renderer.
 * @details     This file defines the EGL wrapper used by the GLES rendering backend.
 *              It manages the EGL display, context, and framebuffer configuration,
 *              as well as dynamically loaded EGL extension function pointers and
 *              extension availability flags.
 *
 *              The EGL context is responsible for making the OpenGL ES context current,
 *              creating and destroying EGL images for DMA-BUF interop, managing
 *              synchronisation objects (fences/sync FDs), and handling buffer-swap
 *              damage regions.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-27
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2026-05-27, initial version.
 */

#ifndef GLES_EGL_H
#define GLES_EGL_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

struct wlf_backend;

/**
 * @struct wlf_egl
 * @brief EGL context wrapper for the wlframe GLES renderer.
 *
 * Encapsulates the EGL display, rendering context, and framebuffer configuration
 * along with all extension support flags and dynamically loaded function pointers
 * needed for DMA-BUF interop, damage tracking, and GPU synchronisation.
 */
struct wlf_egl {
	EGLDisplay display;    /**< EGL display connection handle. */
	EGLContext context;    /**< EGL rendering context. */
	EGLConfig  config;     /**< EGL framebuffer configuration selected at creation time. */
	EGLDeviceEXT device;   /**< Underlying EGL device (valid when EXT_device_query is supported). */

	struct {
		bool KHR_image_base;              /**< EGL_KHR_image_base: EGLImage creation and destruction. */
		bool KHR_fence_sync;              /**< EGL_KHR_fence_sync: fence-based GPU/CPU sync objects. */
		bool KHR_wait_sync;               /**< EGL_KHR_wait_sync: server-side sync waiting. */
		bool KHR_create_context;          /**< EGL_KHR_create_context: extended context creation attributes. */
		bool KHR_surfaceless_context;     /**< EGL_KHR_surfaceless_context: context without a draw surface. */
		bool KHR_gl_colorspace;           /**< EGL_KHR_gl_colorspace: sRGB / linear colorspace selection. */
		bool KHR_gl_texture_2D_image;     /**< EGL_KHR_gl_texture_2D_image: 2-D texture as EGLImage source. */
		bool KHR_gl_renderbuffer_image;   /**< EGL_KHR_gl_renderbuffer_image: renderbuffer as EGLImage source. */
		bool KHR_reusable_sync;           /**< EGL_KHR_reusable_sync: manually signallable sync objects. */
		bool KHR_context_flush_control;   /**< EGL_KHR_context_flush_control: flush behaviour on context release. */
		bool KHR_partial_update;          /**< EGL_KHR_partial_update: eglSetDamageRegionKHR for partial swap. */
		bool KHR_swap_buffers_with_damage;/**< EGL_KHR_swap_buffers_with_damage: swap with damage rects. */
		bool KHR_display_reference;       /**< EGL_KHR_display_reference: reference-counted display lifetime. */

		bool EXT_image_dma_buf_import;           /**< EGL_EXT_image_dma_buf_import: import DMA-BUF as EGLImage. */
		bool EXT_image_dma_buf_import_modifiers; /**< EGL_EXT_image_dma_buf_import_modifiers: DMA-BUF format modifiers. */
		bool EXT_swap_buffers_with_damage;       /**< EGL_EXT_swap_buffers_with_damage: swap with damage (EXT variant). */
		bool EXT_buffer_age;                     /**< EGL_EXT_buffer_age: buffer age for incremental rendering. */
		bool EXT_present_opaque;                 /**< EGL_EXT_present_opaque: present without alpha blending. */
		bool EXT_device_query;                   /**< EGL_EXT_device_query: query the underlying EGL device. */

		bool IMG_context_priority;        /**< EGL_IMG_context_priority: request high-priority GPU context. */
		bool EXT_create_context_robustness;/**< EGL_EXT_create_context_robustness: robust context creation. */

		bool MESA_image_dma_buf_export;   /**< EGL_MESA_image_dma_buf_export: export EGLImage as DMA-BUF. */
		bool MESA_query_driver;           /**< EGL_MESA_query_driver: query Mesa driver name and config. */

		bool KHR_debug;                   /**< EGL_KHR_debug: EGL debug callbacks. */
		bool EXT_device_enumeration;      /**< EGL_EXT_device_enumeration: enumerate available EGL devices. */
		bool EXT_explicit_device;         /**< EGL_EXT_explicit_device: create display from explicit device. */
		bool EXT_device_drm;              /**< EGL_EXT_device_drm: query DRM device from EGL device. */
		bool EXT_device_drm_render_node;  /**< EGL_EXT_device_drm_render_node: query DRM render node. */
		bool KHR_platform_gbm;            /**< EGL_KHR_platform_gbm: create display on GBM device (KHR). */
		bool MESA_platform_gbm;           /**< EGL_MESA_platform_gbm: create display on GBM device (MESA). */
		bool MESA_platform_surfaceless;   /**< EGL_MESA_platform_surfaceless: surfaceless platform display. */
		bool EXT_platform_device;         /**< EGL_EXT_platform_device: create display from EGL device. */

		union {
			struct {
				bool EXT_platform_wayland; /**< EGL_EXT_platform_wayland: Wayland display support (EXT). */
				bool KHR_platform_wayland; /**< EGL_KHR_platform_wayland: Wayland display support (KHR). */
			} wayland;
		} platform;
	} exts;

	struct {
		PFNEGLGETPLATFORMDISPLAYEXTPROC       eglGetPlatformDisplayEXT;      /**< Create a platform-specific EGL display. */
		PFNEGLCREATEIMAGEKHRPROC              eglCreateImageKHR;             /**< Create an EGLImage from an existing resource. */
		PFNEGLDESTROYIMAGEKHRPROC             eglDestroyImageKHR;            /**< Destroy an EGLImage. */
		PFNEGLQUERYDMABUFFORMATSEXTPROC       eglQueryDmaBufFormatsEXT;      /**< Query supported DMA-BUF pixel formats. */
		PFNEGLQUERYDMABUFMODIFIERSEXTPROC     eglQueryDmaBufModifiersEXT;    /**< Query supported DMA-BUF format modifiers. */
		PFNEGLDEBUGMESSAGECONTROLKHRPROC      eglDebugMessageControlKHR;     /**< Control EGL debug message output. */
		PFNEGLQUERYDISPLAYATTRIBEXTPROC       eglQueryDisplayAttribEXT;      /**< Query a display attribute (e.g. backing device). */
		PFNEGLQUERYDEVICESTRINGEXTPROC        eglQueryDeviceStringEXT;       /**< Query a string attribute of an EGL device. */
		PFNEGLQUERYDEVICESEXTPROC             eglQueryDevicesEXT;            /**< Enumerate available EGL devices. */
		PFNEGLCREATESYNCKHRPROC               eglCreateSyncKHR;              /**< Create a sync object. */
		PFNEGLDESTROYSYNCKHRPROC              eglDestroySyncKHR;             /**< Destroy a sync object. */
		PFNEGLDUPNATIVEFENCEFDANDROIDPROC     eglDupNativeFenceFDANDROID;    /**< Duplicate the native fence FD from a sync object. */
		PFNEGLWAITSYNCKHRPROC                 eglWaitSyncKHR;                /**< Insert a server-side wait on a sync object. */
		PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC    eglSwapBuffersWithDamageKHR;   /**< Swap buffers with damage rectangles (KHR). */
		PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC    eglSwapBuffersWithDamageEXT;   /**< Swap buffers with damage rectangles (EXT). */
		PFNEGLSETDAMAGEREGIONKHRPROC          eglSetDamageRegionKHR;         /**< Set the damage region for partial update. */
		PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC  eglExportDMABUFImageQueryMESA; /**< Query exportable DMA-BUF image parameters. */
		PFNEGLEXPORTDMABUFIMAGEMESAPROC       eglExportDMABUFImageMESA;      /**< Export an EGLImage as DMA-BUF file descriptors. */
		PFNEGLGETDISPLAYDRIVERCONFIGPROC      eglGetDisplayDriverConfig;     /**< Retrieve Mesa driver configuration string. */
		PFNEGLGETDISPLAYDRIVERNAMEPROC        eglGetDisplayDriverName;       /**< Retrieve Mesa driver name string. */
	} procs;
};

/**
 * @brief Creates an EGL display and context for the given backend.
 *
 * Selects an appropriate EGL platform based on the backend type, initialises
 * the EGL display, chooses a framebuffer configuration, and creates an
 * OpenGL ES context.  Extension support flags and function pointers are
 * populated before returning.
 *
 * @param backend Pointer to the backend that will own this EGL context.
 * @return Pointer to the initialised EGL wrapper, or NULL on failure.
 */
struct wlf_egl *wlf_egl_create(struct wlf_backend *backend);

/**
 * @brief Destroys the EGL context and releases all associated resources.
 * @param egl Pointer to the EGL wrapper to destroy.
 */
void wlf_egl_destroy(struct wlf_egl *egl);

/**
 * @brief Returns a human-readable string for an EGL error code.
 * @param error EGL error code returned by eglGetError().
 * @return Pointer to a static string describing the error.
 */
const char *wlf_egl_error_str(EGLint error);

/**
 * @brief Checks whether a specific extension is present in an extension string.
 * @param exts Space-separated EGL/GL extension string.
 * @param ext  Name of the extension to look up.
 * @return true if the extension is present, false otherwise.
 */
bool wlf_egl_check_ext(const char *exts, const char *ext);

/**
 * @brief Loads an EGL or GL extension procedure by name.
 *
 * Uses eglGetProcAddress to resolve @p name and stores the result into
 * the location pointed to by @p proc_ptr.  Aborts if the procedure
 * cannot be resolved.
 *
 * @param proc_ptr Pointer to the function-pointer variable to fill.
 * @param name     Null-terminated name of the procedure to load.
 */
void wlf_egl_load_proc(void *proc_ptr, const char *name);

/**
 * @brief Destroys an EGLImage created through this EGL context.
 * @param egl   Pointer to the EGL wrapper.
 * @param image EGLImage handle to destroy.
 * @return true on success, false if the call fails.
 */
bool wlf_egl_destroy_image(struct wlf_egl *egl, EGLImageKHR image);

/**
 * @brief Makes the EGL context current on the calling thread.
 * @param egl  Pointer to the EGL wrapper.
 * @param draw EGL draw surface, or EGL_NO_SURFACE for surfaceless rendering.
 * @param read EGL read surface, or EGL_NO_SURFACE for surfaceless rendering.
 * @return true on success, false on failure.
 */
bool wlf_egl_make_current(struct wlf_egl *egl, EGLSurface draw, EGLSurface read);

/**
 * @brief Releases the EGL context from the calling thread.
 * @param egl Pointer to the EGL wrapper.
 * @return true on success, false on failure.
 */
bool wlf_egl_unset_current(struct wlf_egl *egl);

/**
 * @brief Creates an EGL sync object, optionally importing a native fence FD.
 *
 * If @p fence_fd is >= 0 the sync object wraps the provided Android native
 * fence; otherwise a plain fence sync is created.
 *
 * @param egl      Pointer to the EGL wrapper.
 * @param fence_fd Native fence file descriptor to import, or -1.
 * @return EGL sync handle, or EGL_NO_SYNC_KHR on failure.
 */
EGLSyncKHR wlf_egl_create_sync(struct wlf_egl *egl, int fence_fd);

/**
 * @brief Destroys an EGL sync object.
 * @param egl  Pointer to the EGL wrapper.
 * @param sync EGL sync handle to destroy.
 */
void wlf_egl_destroy_sync(struct wlf_egl *egl, EGLSyncKHR sync);

/**
 * @brief Duplicates the native fence file descriptor from an EGL sync object.
 * @param egl  Pointer to the EGL wrapper.
 * @param sync EGL sync handle whose fence FD to duplicate.
 * @return Duplicated file descriptor, or -1 on failure.
 */
int wlf_egl_dup_fence_fd(struct wlf_egl *egl, EGLSyncKHR sync);

/**
 * @brief Inserts a server-side wait on an EGL sync object.
 * @param egl  Pointer to the EGL wrapper.
 * @param sync EGL sync handle to wait on.
 * @return true on success, false on failure.
 */
bool wlf_egl_wait_sync(struct wlf_egl *egl, EGLSyncKHR sync);

#endif // GLES_EGL_H
