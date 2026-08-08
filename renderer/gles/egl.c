#include "wlf/renderer/gles/egl.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/config.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/types/wlf_format_set.h"

#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/wayland/backend.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#if WLF_HAS_LINUX_PLATFORM
struct egl_format_attribs {
	EGLint red_size;
	EGLint green_size;
	EGLint blue_size;
	EGLint alpha_size;
	EGLint native_visual_id;
};

static bool get_egl_format_attribs(const struct wlf_render_format *format,
		struct egl_format_attribs *attribs) {
	switch (format->format) {
	case WLF_FORMAT_ARGB8888:
	case WLF_FORMAT_ABGR8888:
	case WLF_FORMAT_RGBA8888:
	case WLF_FORMAT_BGRA8888:
		*attribs = (struct egl_format_attribs){
			.red_size = 8,
			.green_size = 8,
			.blue_size = 8,
			.alpha_size = 8,
			.native_visual_id = (EGLint)convert_wlf_format_to_wl_shm(format->format),
		};
		return true;
	case WLF_FORMAT_XRGB8888:
	case WLF_FORMAT_XBGR8888:
	case WLF_FORMAT_RGBX8888:
	case WLF_FORMAT_BGRX8888:
		*attribs = (struct egl_format_attribs){
			.red_size = 8,
			.green_size = 8,
			.blue_size = 8,
			.alpha_size = 0,
			.native_visual_id = (EGLint)convert_wlf_format_to_wl_shm(format->format),
		};
		return true;
	case WLF_FORMAT_RGB565:
	case WLF_FORMAT_BGR565:
		*attribs = (struct egl_format_attribs){
			.red_size = 5,
			.green_size = 6,
			.blue_size = 5,
			.alpha_size = 0,
			.native_visual_id = (EGLint)convert_wlf_format_to_wl_shm(format->format),
		};
		return true;
	case WLF_FORMAT_ARGB2101010:
	case WLF_FORMAT_ABGR2101010:
		*attribs = (struct egl_format_attribs){
			.red_size = 10,
			.green_size = 10,
			.blue_size = 10,
			.alpha_size = 2,
			.native_visual_id = (EGLint)convert_wlf_format_to_wl_shm(format->format),
		};
		return true;
	case WLF_FORMAT_XRGB2101010:
	case WLF_FORMAT_XBGR2101010:
		*attribs = (struct egl_format_attribs){
			.red_size = 10,
			.green_size = 10,
			.blue_size = 10,
			.alpha_size = 0,
			.native_visual_id = (EGLint)convert_wlf_format_to_wl_shm(format->format),
		};
		return true;
	default:
		break;
	}

	return false;
}

static bool egl_config_matches_attribs(EGLDisplay display, EGLConfig config,
		const struct egl_format_attribs *attribs) {
	EGLint red_size = 0, green_size = 0, blue_size = 0, alpha_size = 0;
	if (!eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red_size) ||
			!eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green_size) ||
			!eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue_size) ||
			!eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alpha_size)) {
		return false;
	}

	return red_size == attribs->red_size &&
		green_size == attribs->green_size &&
		blue_size == attribs->blue_size &&
		alpha_size == attribs->alpha_size;
}

EGLConfig wlf_egl_choose_config(struct wlf_egl *egl,
		const struct wlf_render_format *format) {
	struct egl_format_attribs format_attribs;
	if (!get_egl_format_attribs(format, &format_attribs)) {
		wlf_log(WLF_ERROR, "unsupported EGL swapchain format 0x%"PRIx32,
			format->format);
		return NULL;
	}

	const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE, format_attribs.red_size,
		EGL_GREEN_SIZE, format_attribs.green_size,
		EGL_BLUE_SIZE, format_attribs.blue_size,
		EGL_ALPHA_SIZE, format_attribs.alpha_size,
		EGL_NONE,
	};

	EGLint config_count = 0;
	if (!eglChooseConfig(egl->display, config_attribs, NULL, 0,
			&config_count) || config_count == 0) {
		wlf_log(WLF_ERROR, "failed to query EGL configs for format 0x%"PRIx32": %s",
			format->format, wlf_egl_error_str(eglGetError()));
		return NULL;
	}

	EGLConfig configs[config_count];
	if (!eglChooseConfig(egl->display, config_attribs, configs,
			config_count, &config_count) || config_count == 0) {
		wlf_log(WLF_ERROR, "failed to choose EGL configs for format 0x%"PRIx32": %s",
			format->format, wlf_egl_error_str(eglGetError()));
		return NULL;
	}

	EGLConfig fallback = NULL;
	for (EGLint i = 0; i < config_count; i++) {
		if (!egl_config_matches_attribs(egl->display, configs[i],
				&format_attribs)) {
			continue;
		}
		if (fallback == NULL) {
			fallback = configs[i];
		}

		EGLint native_visual_id = 0;
		if (eglGetConfigAttrib(egl->display, configs[i],
				EGL_NATIVE_VISUAL_ID, &native_visual_id) &&
				native_visual_id == format_attribs.native_visual_id) {
			return configs[i];
		}
	}

	if (fallback != NULL) {
		return fallback;
	}

	wlf_log(WLF_ERROR, "no EGL config matches format 0x%"PRIx32,
		format->format);
	return NULL;
}
#endif

static EGLDisplay get_egl_display(struct wlf_backend *backend,
		const char *client_exts) {
	void *native_display = backend->impl->native_display(backend);

#if WLF_HAS_LINUX_PLATFORM
	if (wlf_backend_is_wayland(backend) &&
			(!wlf_egl_check_ext(client_exts, "EGL_EXT_platform_base") ||
			!((wlf_egl_check_ext(client_exts, "EGL_EXT_platform_wayland") ||
			 wlf_egl_check_ext(client_exts, "EGL_KHR_platform_wayland"))))) {
		return NULL;
	}
#endif

	return eglGetDisplay((EGLNativeDisplayType)native_display);
}

static void check_egl_exts(struct wlf_egl *egl,
		const char *display_exts, const char *client_exts,
		const struct wlf_backend *backend) {
	if (display_exts != NULL) {
		egl->exts.KHR_image_base =
			wlf_egl_check_ext(display_exts, "EGL_KHR_image_base");
		egl->exts.KHR_fence_sync =
			wlf_egl_check_ext(display_exts, "EGL_KHR_fence_sync");
		egl->exts.KHR_wait_sync =
			wlf_egl_check_ext(display_exts, "EGL_KHR_wait_sync");
		egl->exts.KHR_create_context =
			wlf_egl_check_ext(display_exts, "EGL_KHR_create_context");
		egl->exts.KHR_surfaceless_context =
			wlf_egl_check_ext(display_exts, "EGL_KHR_surfaceless_context");
		egl->exts.KHR_gl_colorspace =
			wlf_egl_check_ext(display_exts, "EGL_KHR_gl_colorspace");
		egl->exts.KHR_gl_texture_2D_image =
			wlf_egl_check_ext(display_exts, "EGL_KHR_gl_texture_2D_image");
		egl->exts.KHR_gl_renderbuffer_image =
			wlf_egl_check_ext(display_exts, "EGL_KHR_gl_renderbuffer_image");
		egl->exts.KHR_reusable_sync =
			wlf_egl_check_ext(display_exts, "EGL_KHR_reusable_sync");
		egl->exts.KHR_context_flush_control =
			wlf_egl_check_ext(display_exts, "EGL_KHR_context_flush_control");
		egl->exts.KHR_partial_update =
			wlf_egl_check_ext(display_exts, "EGL_KHR_partial_update");
		egl->exts.KHR_swap_buffers_with_damage =
			wlf_egl_check_ext(display_exts, "EGL_KHR_swap_buffers_with_damage");
		egl->exts.KHR_display_reference =
			wlf_egl_check_ext(display_exts, "EGL_KHR_display_reference");

		egl->exts.EXT_image_dma_buf_import =
			wlf_egl_check_ext(display_exts, "EGL_EXT_image_dma_buf_import");
		egl->exts.EXT_image_dma_buf_import_modifiers =
			wlf_egl_check_ext(display_exts, "EGL_EXT_image_dma_buf_import_modifiers");
		egl->exts.EXT_swap_buffers_with_damage =
			wlf_egl_check_ext(display_exts, "EGL_EXT_swap_buffers_with_damage");
		egl->exts.EXT_buffer_age =
			wlf_egl_check_ext(display_exts, "EGL_EXT_buffer_age");
		egl->exts.EXT_present_opaque =
			wlf_egl_check_ext(display_exts, "EGL_EXT_present_opaque");
		egl->exts.EXT_device_query =
			wlf_egl_check_ext(display_exts, "EGL_EXT_device_query");
		egl->exts.ANDROID_native_fence_sync =
			wlf_egl_check_ext(display_exts, "EGL_ANDROID_native_fence_sync");

		egl->exts.IMG_context_priority =
			wlf_egl_check_ext(display_exts, "EGL_IMG_context_priority");
		egl->exts.EXT_create_context_robustness =
			wlf_egl_check_ext(display_exts, "EGL_EXT_create_context_robustness");

		egl->exts.MESA_image_dma_buf_export =
			wlf_egl_check_ext(display_exts, "EGL_MESA_image_dma_buf_export");
		egl->exts.MESA_query_driver =
			wlf_egl_check_ext(display_exts, "EGL_MESA_query_driver");
	}
	if (client_exts != NULL) {
		egl->exts.KHR_debug =
			wlf_egl_check_ext(client_exts, "EGL_KHR_debug");
		egl->exts.EXT_platform_base =
			wlf_egl_check_ext(client_exts, "EGL_EXT_platform_base");
		egl->exts.EXT_device_enumeration =
			wlf_egl_check_ext(client_exts, "EGL_EXT_device_enumeration");
		egl->exts.EXT_explicit_device =
			wlf_egl_check_ext(client_exts, "EGL_EXT_explicit_device");
		egl->exts.EXT_device_drm =
			wlf_egl_check_ext(client_exts, "EGL_EXT_device_drm");
		egl->exts.EXT_device_drm_render_node =
			wlf_egl_check_ext(client_exts, "EGL_EXT_device_drm_render_node");
		egl->exts.KHR_platform_gbm =
			wlf_egl_check_ext(client_exts, "EGL_KHR_platform_gbm");
		egl->exts.MESA_platform_gbm =
			wlf_egl_check_ext(client_exts, "EGL_MESA_platform_gbm");
		egl->exts.MESA_platform_surfaceless =
			wlf_egl_check_ext(client_exts, "EGL_MESA_platform_surfaceless");
		egl->exts.EXT_platform_device =
			wlf_egl_check_ext(client_exts, "EGL_EXT_platform_device");

#if WLF_HAS_LINUX_PLATFORM
		if (wlf_backend_is_wayland(backend)) {
			egl->exts.platform.wayland.EXT_platform_wayland =
				wlf_egl_check_ext(client_exts, "EGL_EXT_platform_wayland");
			egl->exts.platform.wayland.KHR_platform_wayland =
				wlf_egl_check_ext(client_exts, "EGL_KHR_platform_wayland");
		}
#endif
	}
}

static void load_egl_procs(struct wlf_egl *egl,
		const char *display_exts, const char *client_exts) {
	if (client_exts != NULL) {
		if (wlf_egl_check_ext(client_exts, "EGL_EXT_platform_base")) {
			wlf_egl_load_proc(&egl->procs.eglGetPlatformDisplayEXT,
				"eglGetPlatformDisplayEXT");
			wlf_egl_load_proc(&egl->procs.eglCreatePlatformWindowSurfaceEXT,
				"eglCreatePlatformWindowSurfaceEXT");
		}
		if (wlf_egl_check_ext(client_exts, "EGL_EXT_device_enumeration")) {
			wlf_egl_load_proc(&egl->procs.eglQueryDevicesEXT,
				"eglQueryDevicesEXT");
		}
		if (wlf_egl_check_ext(client_exts, "EGL_KHR_debug")) {
			wlf_egl_load_proc(&egl->procs.eglDebugMessageControlKHR,
				"eglDebugMessageControlKHR");
		}
	}
	if (display_exts == NULL) {
		return;
	}
	if (wlf_egl_check_ext(display_exts, "EGL_KHR_image_base")) {
		wlf_egl_load_proc(&egl->procs.eglCreateImageKHR, "eglCreateImageKHR");
		wlf_egl_load_proc(&egl->procs.eglDestroyImageKHR, "eglDestroyImageKHR");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_EXT_image_dma_buf_import")) {
		wlf_egl_load_proc(&egl->procs.eglQueryDmaBufFormatsEXT,
			"eglQueryDmaBufFormatsEXT");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_EXT_image_dma_buf_import_modifiers")) {
		wlf_egl_load_proc(&egl->procs.eglQueryDmaBufModifiersEXT,
			"eglQueryDmaBufModifiersEXT");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_EXT_device_query")) {
		wlf_egl_load_proc(&egl->procs.eglQueryDisplayAttribEXT,
			"eglQueryDisplayAttribEXT");
		wlf_egl_load_proc(&egl->procs.eglQueryDeviceStringEXT,
			"eglQueryDeviceStringEXT");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_KHR_fence_sync")) {
		wlf_egl_load_proc(&egl->procs.eglCreateSyncKHR, "eglCreateSyncKHR");
		wlf_egl_load_proc(&egl->procs.eglDestroySyncKHR, "eglDestroySyncKHR");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_KHR_wait_sync")) {
		wlf_egl_load_proc(&egl->procs.eglWaitSyncKHR, "eglWaitSyncKHR");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_ANDROID_native_fence_sync")) {
		wlf_egl_load_proc(&egl->procs.eglDupNativeFenceFDANDROID,
			"eglDupNativeFenceFDANDROID");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_KHR_swap_buffers_with_damage")) {
		wlf_egl_load_proc(&egl->procs.eglSwapBuffersWithDamageKHR,
			"eglSwapBuffersWithDamageKHR");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_EXT_swap_buffers_with_damage")) {
		wlf_egl_load_proc(&egl->procs.eglSwapBuffersWithDamageEXT,
			"eglSwapBuffersWithDamageEXT");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_KHR_partial_update")) {
		wlf_egl_load_proc(&egl->procs.eglSetDamageRegionKHR,
			"eglSetDamageRegionKHR");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_MESA_image_dma_buf_export")) {
		wlf_egl_load_proc(&egl->procs.eglExportDMABUFImageQueryMESA,
			"eglExportDMABUFImageQueryMESA");
		wlf_egl_load_proc(&egl->procs.eglExportDMABUFImageMESA,
			"eglExportDMABUFImageMESA");
	}
	if (wlf_egl_check_ext(display_exts, "EGL_MESA_query_driver")) {
		wlf_egl_load_proc(&egl->procs.eglGetDisplayDriverConfig,
			"eglGetDisplayDriverConfig");
		wlf_egl_load_proc(&egl->procs.eglGetDisplayDriverName,
			"eglGetDisplayDriverName");
	}
}

static enum wlf_log_importance egl_log_importance_to_wlf(EGLint type) {
	switch (type) {
	case EGL_DEBUG_MSG_CRITICAL_KHR: return WLF_ERROR;
	case EGL_DEBUG_MSG_ERROR_KHR:    return WLF_ERROR;
	case EGL_DEBUG_MSG_WARN_KHR:     return WLF_ERROR;
	case EGL_DEBUG_MSG_INFO_KHR:     return WLF_INFO;
	default:                         return WLF_INFO;
	}
}

static void egl_log(EGLenum error, const char *command, EGLint msg_type,
		EGLLabelKHR thread, EGLLabelKHR obj, const char *msg) {
	_wlf_log(egl_log_importance_to_wlf(msg_type),
		"[EGL] command: %s, error: %s (0x%x), message: \"%s\"",
		command, wlf_egl_error_str(error), error, msg);
}

struct wlf_egl *wlf_egl_create(struct wlf_backend *backend) {
	const char *client_exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	EGLDisplay egl_display = get_egl_display(backend, client_exts);
	if (egl_display == EGL_NO_DISPLAY) {
		wlf_log(WLF_ERROR, "Failed to get EGLDisplay: %s",
			wlf_egl_error_str(eglGetError()));
		return NULL;
	}

	EGLint major, minor;
	if (eglInitialize(egl_display, &major, &minor) != EGL_TRUE) {
		wlf_log(WLF_ERROR, "Failed to initialise EGLDisplay: %s",
			wlf_egl_error_str(eglGetError()));
		eglTerminate(egl_display);
		eglReleaseThread();
		return NULL;
	}

	struct wlf_egl *egl = calloc(1, sizeof(*egl));
	if (egl == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_egl");
		eglTerminate(egl_display);
		eglReleaseThread();
		return NULL;
	}
	egl->display = egl_display;

	const char *display_exts = eglQueryString(egl_display, EGL_EXTENSIONS);
	wlf_log(WLF_INFO, "Supported EGL display extensions: %s",
		display_exts ? display_exts : "(none)");
	wlf_log(WLF_INFO, "Supported EGL client extensions: %s",
		client_exts ? client_exts : "(none)");
	check_egl_exts(egl, display_exts, client_exts, backend);
	load_egl_procs(egl, display_exts, client_exts);
	if (wlf_env_parse_bool("WLF_RENDER_DEBUG") &&
			egl->procs.eglDebugMessageControlKHR != NULL) {
		static const EGLAttrib debug_attribs[] = {
			EGL_DEBUG_MSG_CRITICAL_KHR, EGL_TRUE,
			EGL_DEBUG_MSG_ERROR_KHR, EGL_TRUE,
			EGL_DEBUG_MSG_WARN_KHR, EGL_TRUE,
			EGL_DEBUG_MSG_INFO_KHR, EGL_TRUE,
			EGL_NONE,
		};
		egl->procs.eglDebugMessageControlKHR(egl_log, debug_attribs);
	}

	if (!egl->exts.KHR_surfaceless_context) {
		wlf_log(WLF_ERROR, "EGL_KHR_surfaceless_context is required");
		goto failed;
	}

	const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE,
	};
	EGLint matched = 0;
	if (!eglChooseConfig(egl_display, config_attribs, &egl->config, 1, &matched) ||
			matched == 0) {
		wlf_log(WLF_ERROR, "Failed to choose EGL config: %s",
			wlf_egl_error_str(eglGetError()));
		goto failed;
	}

	if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
		wlf_log(WLF_ERROR, "Failed to bind OpenGL ES API: %s",
			wlf_egl_error_str(eglGetError()));
		goto failed;
	}

	const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
	};
	egl->context = eglCreateContext(egl_display, egl->config, EGL_NO_CONTEXT,
		context_attribs);
	if (egl->context == EGL_NO_CONTEXT) {
		wlf_log(WLF_ERROR, "Failed to create EGL context: %s",
			wlf_egl_error_str(eglGetError()));
		goto failed;
	}

	wlf_log(WLF_INFO, "EGL %d.%d initialized", major, minor);
	return egl;

failed:
	wlf_egl_destroy(egl);
	return NULL;
}

void wlf_egl_destroy(struct wlf_egl *egl) {
	if (egl == NULL) {
		return;
	}

	if (egl->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (egl->context != EGL_NO_CONTEXT) {
			eglDestroyContext(egl->display, egl->context);
		}
		eglTerminate(egl->display);
	}

	eglReleaseThread();
	free(egl);
}

const char *wlf_egl_error_str(EGLint error) {
	switch (error) {
	case EGL_SUCCESS:
		return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED:
		return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS:
		return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC:
		return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE:
		return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONTEXT:
		return "EGL_BAD_CONTEXT";
	case EGL_BAD_CONFIG:
		return "EGL_BAD_CONFIG";
	case EGL_BAD_CURRENT_SURFACE:
		return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY:
		return "EGL_BAD_DISPLAY";
	case EGL_BAD_DEVICE_EXT:
		return "EGL_BAD_DEVICE_EXT";
	case EGL_BAD_SURFACE:
		return "EGL_BAD_SURFACE";
	case EGL_BAD_MATCH:
		return "EGL_BAD_MATCH";
	case EGL_BAD_PARAMETER:
		return "EGL_BAD_PARAMETER";
	case EGL_BAD_NATIVE_PIXMAP:
		return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW:
		return "EGL_BAD_NATIVE_WINDOW";
	case EGL_CONTEXT_LOST:
		return "EGL_CONTEXT_LOST";
	}
	return "unknown error";
}

bool wlf_egl_check_ext(const char *exts, const char *ext) {
	if (exts == NULL || ext == NULL || *ext == '\0') {
		return false;
	}

	size_t extlen = strlen(ext);
	const char *end = exts + strlen(exts);

	while (exts < end) {
		if (*exts == ' ') {
			exts++;
			continue;
		}
		size_t n = strcspn(exts, " ");
		if (n == extlen && strncmp(ext, exts, n) == 0) {
			return true;
		}
		exts += n;
	}
	return false;
}

void wlf_egl_load_proc(void *proc_ptr, const char *name) {
	void *proc = (void *)eglGetProcAddress(name);
	if (proc == NULL) {
		wlf_log(WLF_ERROR, "eglGetProcAddress(%s) failed", name);
		abort();
	}
	*(void **)proc_ptr = proc;
}

bool wlf_egl_destroy_image(struct wlf_egl *egl, EGLImageKHR image) {
	if (egl->procs.eglDestroyImageKHR == NULL) {
		return false;
	}
	if (!egl->procs.eglDestroyImageKHR(egl->display, image)) {
		wlf_log(WLF_ERROR, "eglDestroyImageKHR failed: %s",
			wlf_egl_error_str(eglGetError()));
		return false;
	}
	return true;
}

bool wlf_egl_make_current(struct wlf_egl *egl, EGLSurface draw, EGLSurface read) {
	if (!eglMakeCurrent(egl->display, draw, read, egl->context)) {
		wlf_log(WLF_ERROR, "eglMakeCurrent failed: %s",
			wlf_egl_error_str(eglGetError()));
		return false;
	}
	return true;
}

bool wlf_egl_unset_current(struct wlf_egl *egl) {
	if (!eglMakeCurrent(egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl->context)) {
		wlf_log(WLF_ERROR, "eglMakeCurrent (unset) failed: %s",
			wlf_egl_error_str(eglGetError()));
		return false;
	}
	return true;
}

EGLSyncKHR wlf_egl_create_sync(struct wlf_egl *egl, int fence_fd) {
	if (fence_fd >= 0) {
		if (egl->procs.eglCreateSyncKHR == NULL ||
				!egl->exts.ANDROID_native_fence_sync) {
			return EGL_NO_SYNC_KHR;
		}
		EGLint attribs[] = {
			EGL_SYNC_NATIVE_FENCE_FD_ANDROID, fence_fd,
			EGL_NONE,
		};
		return egl->procs.eglCreateSyncKHR(egl->display,
			EGL_SYNC_NATIVE_FENCE_ANDROID, attribs);
	}
	if (egl->procs.eglCreateSyncKHR == NULL) {
		return EGL_NO_SYNC_KHR;
	}
	return egl->procs.eglCreateSyncKHR(egl->display, EGL_SYNC_FENCE_KHR, NULL);
}

void wlf_egl_destroy_sync(struct wlf_egl *egl, EGLSyncKHR sync) {
	assert(sync != EGL_NO_SYNC_KHR);
	assert(egl->procs.eglDestroySyncKHR != NULL);

	if (egl->procs.eglDestroySyncKHR(egl->display, sync) != EGL_TRUE) {
		wlf_log(WLF_ERROR, "eglDestroySyncKHR failed: %s",
			wlf_egl_error_str(eglGetError()));
	}
}

int wlf_egl_dup_fence_fd(struct wlf_egl *egl, EGLSyncKHR sync) {
	if (egl->procs.eglDupNativeFenceFDANDROID == NULL) {
		return -1;
	}

	int fd = egl->procs.eglDupNativeFenceFDANDROID(egl->display, sync);
	if (fd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
		wlf_log(WLF_ERROR, "eglDupNativeFenceFDANDROID failed: %s",
			wlf_egl_error_str(eglGetError()));
		return -1;
	}

	return fd;
}

bool wlf_egl_wait_sync(struct wlf_egl *egl, EGLSyncKHR sync) {
	if (egl->procs.eglWaitSyncKHR == NULL) {
		wlf_log(WLF_ERROR, "eglWaitSyncKHR is NULL procs");

		return false;
	}

	if (egl->procs.eglWaitSyncKHR(egl->display, sync, 0) != EGL_TRUE) {
		wlf_log(WLF_ERROR, "eglWaitSyncKHR failed: %s",
			wlf_egl_error_str(eglGetError()));
		return false;
	}
	return true;
}
