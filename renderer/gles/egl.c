#include "wlf/renderer/gles/egl.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/config.h"
#include "wlf/utils/wlf_env.h"

#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/wayland/backend.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
	void *native_display = backend->impl->native_display(backend);
	EGLDisplay egl_display = eglGetDisplay((EGLNativeDisplayType)native_display);
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
	const char *client_exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
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

	const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE,
	};
	EGLint matched = 0;
	if (!eglChooseConfig(egl_display, config_attribs, &egl->config, 1, &matched) ||
			matched == 0) {
		wlf_log(WLF_ERROR, "Failed to choose EGL config: %s",
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
				egl->procs.eglDupNativeFenceFDANDROID == NULL) {
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
	assert(egl->procs.eglWaitSyncKHR != NULL);

	if (egl->procs.eglWaitSyncKHR(egl->display, sync, 0) != EGL_TRUE) {
		wlf_log(WLF_ERROR, "eglWaitSyncKHR failed: %s",
			wlf_egl_error_str(eglGetError()));
		return false;
	}
	return true;
}
