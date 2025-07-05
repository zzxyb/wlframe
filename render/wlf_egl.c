#include "wlf/render/wlf_egl.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

static enum wlf_log_importance egl_log_importance_to_wlf(EGLint type) {
	switch (type) {
	case EGL_DEBUG_MSG_CRITICAL_KHR:
		return WLF_ERROR;
	case EGL_DEBUG_MSG_ERROR_KHR:
		return WLF_ERROR;
	case EGL_DEBUG_MSG_WARN_KHR:
		return WLF_ERROR;
	case EGL_DEBUG_MSG_INFO_KHR:
		return WLF_INFO;
	default:
		return WLF_INFO;
	}
}

static const char *egl_error_str(EGLint error) {
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

static void egl_log(EGLenum error, const char *command, EGLint msg_type,
		EGLLabelKHR thread, EGLLabelKHR obj, const char *msg) {
	wlf_log(egl_log_importance_to_wlf(msg_type),
		"[EGL] command: %s, error: %s (0x%x), message: \"%s\"",
		command, egl_error_str(error), error, msg);
}

static bool check_egl_ext(const char *exts, const char *ext) {
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

static void load_egl_proc(void *proc_ptr, const char *name) {
	void *proc = (void *)eglGetProcAddress(name);
	if (proc == NULL) {
		wlf_log(WLF_ERROR, "eglGetProcAddress(%s) failed", name);
		abort();
	}
	*(void **)proc_ptr = proc;
}

struct wlf_egl *wlf_egl_create(NativeDisplayType native_display) {
	struct  wlf_egl *egl = malloc(sizeof(struct wlf_egl));
	if (egl == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for wlf_egl");
		return NULL;
	}

	egl->display = eglGetDisplay(native_display);
	if (egl->display == EGL_NO_DISPLAY) {
		wlf_log(WLF_ERROR, "Failed to get EGL display: %s",
			egl_error_str(eglGetError()));
		free(egl);
		return NULL;
	}

	EGLint major, minor;
	if (!eglInitialize(egl->display, &major, &minor)) {
		wlf_log(WLF_ERROR, "Failed to initialize EGL: %s",
			egl_error_str(eglGetError()));
		free(egl);
		return NULL;
	}

	wlf_log(WLF_INFO, "EGL version: %d.%d", major, minor);

	const char *client_exts_str = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (client_exts_str == NULL) {
		if (eglGetError() == EGL_BAD_DISPLAY) {
			wlf_log(WLF_ERROR, "EGL_EXT_client_extensions not supported");
		} else {
			wlf_log(WLF_ERROR, "Failed to query EGL client extensions");
		}
		free(egl);
		return NULL;
	}

	wlf_log(WLF_INFO, "Supported EGL client extensions: %s", client_exts_str);

	if (!check_egl_ext(client_exts_str, "EGL_EXT_platform_base")) {
		wlf_log(WLF_ERROR, "EGL_EXT_platform_base not supported");
		free(egl);
		return NULL;
	}

	egl->context = EGL_NO_CONTEXT;
	egl->device = EGL_NO_DEVICE_EXT;
	egl->exts.EXT_device_query = false;
	egl->exts.EXT_platform_device = false;
	egl->exts.KHR_display_reference = false;
	egl->procs.eglGetPlatformDisplayEXT = NULL;
	egl->procs.eglCreateImageKHR = NULL;
	egl->procs.eglDestroyImageKHR = NULL;
	egl->procs.eglQueryDmaBufFormatsEXT = NULL;
	egl->procs.eglQueryDmaBufModifiersEXT = NULL;
	egl->procs.eglDebugMessageControlKHR = NULL;
	egl->procs.eglQueryDisplayAttribEXT = NULL;
	egl->procs.eglQueryDeviceStringEXT = NULL;
	egl->procs.eglQueryDevicesEXT = NULL;
	egl->procs.eglCreateSyncKHR = NULL;
	egl->procs.eglDestroySyncKHR = NULL;
	egl->procs.eglDupNativeFenceFDANDROID = NULL;
	egl->procs.eglWaitSyncKHR = NULL;
	egl->procs.eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
		eglGetProcAddress("eglGetPlatformDisplayEXT");
	if (egl->procs.eglGetPlatformDisplayEXT == NULL) {
		wlf_log(WLF_ERROR, "eglGetPlatformDisplayEXT not supported");
		free(egl);
		return NULL;
	}
	egl->display = egl->procs.eglGetPlatformDisplayEXT(
		EGL_PLATFORM_DEVICE_EXT, native_display, NULL);
	if (egl->display == EGL_NO_DISPLAY) {
		wlf_log(WLF_ERROR, "Failed to get EGL display: %s",
			egl_error_str(eglGetError()));
		free(egl);
		return NULL;
	}
	if (!check_egl_ext(client_exts_str, "EGL_KHR_platform_wayland")) {
		wlf_log(WLF_ERROR, "EGL_KHR_platform_wayland not supported");
		free(egl);
		return NULL;
	}
	egl->exts.KHR_platform_wayland = true;
	if (!check_egl_ext(client_exts_str, "EGL_EXT_device_query")) {
		wlf_log(WLF_ERROR, "EGL_EXT_device_query not supported");
		free(egl);
		return NULL;
	}
	egl->exts.EXT_device_query = true;
	if (!check_egl_ext(client_exts_str, "EGL_EXT_platform_device")) {
		wlf_log(WLF_ERROR, "EGL_EXT_platform_device not supported");
		free(egl);
		return NULL;
	}
	egl->exts.EXT_platform_device = true;
	if (!check_egl_ext(client_exts_str, "EGL_KHR_display_reference")) {
		wlf_log(WLF_ERROR, "EGL_KHR_display_reference not supported");
		free(egl);
		return NULL;
	}
	egl->exts.KHR_display_reference = true;

	egl->procs.eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC)
		eglGetProcAddress("eglQueryDisplayAttribEXT");
	if (egl->procs.eglQueryDisplayAttribEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDisplayAttribEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)
		eglGetProcAddress("eglQueryDeviceStringEXT");
	if (egl->procs.eglQueryDeviceStringEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDeviceStringEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)
		eglGetProcAddress("eglQueryDevicesEXT");
	if (egl->procs.eglQueryDevicesEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDevicesEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC)
		eglGetProcAddress("eglCreateSyncKHR");
	if (egl->procs.eglCreateSyncKHR == NULL) {
		wlf_log(WLF_ERROR, "eglCreateSyncKHR not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)
		eglGetProcAddress("eglDestroySyncKHR");
	if (egl->procs.eglDestroySyncKHR == NULL) {
		wlf_log(WLF_ERROR, "eglDestroySyncKHR not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglDupNativeFenceFDANDROID = (PFNEGLDUPNATIVEFENCEFDANDROIDPROC)
		eglGetProcAddress("eglDupNativeFenceFDANDROID");
	if (egl->procs.eglDupNativeFenceFDANDROID == NULL) {
		wlf_log(WLF_ERROR, "eglDupNativeFenceFDANDROID not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglWaitSyncKHR = (PFNEGLWAITSYNCKHRPROC)
		eglGetProcAddress("eglWaitSyncKHR");
	if (egl->procs.eglWaitSyncKHR == NULL) {
		wlf_log(WLF_ERROR, "eglWaitSyncKHR not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)
		eglGetProcAddress("eglCreateImageKHR");
	if (egl->procs.eglCreateImageKHR == NULL) {
		wlf_log(WLF_ERROR, "eglCreateImageKHR not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)
		eglGetProcAddress("eglDestroyImageKHR");
	if (egl->procs.eglDestroyImageKHR == NULL) {
		wlf_log(WLF_ERROR, "eglDestroyImageKHR not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglQueryDmaBufFormatsEXT = (PFNEGLQUERYDMABUFFORMATSEXTPROC)
		eglGetProcAddress("eglQueryDmaBufFormatsEXT");
	if (egl->procs.eglQueryDmaBufFormatsEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDmaBufFormatsEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglQueryDmaBufModifiersEXT = (PFNEGLQUERYDMABUFMODIFIERSEXTPROC)
		eglGetProcAddress("eglQueryDmaBufModifiersEXT");
	if (egl->procs.eglQueryDmaBufModifiersEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDmaBufModifiersEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)
		eglGetProcAddress("eglQueryDeviceStringEXT");
	if (egl->procs.eglQueryDeviceStringEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDeviceStringEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)
		eglGetProcAddress("eglQueryDevicesEXT");
	if (egl->procs.eglQueryDevicesEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDevicesEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC)
		eglGetProcAddress("eglQueryDisplayAttribEXT");
	if (egl->procs.eglQueryDisplayAttribEXT == NULL) {
		wlf_log(WLF_ERROR, "eglQueryDisplayAttribEXT not supported");
		free(egl);
		return NULL;
	}
	egl->procs.eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
		eglGetProcAddress("eglGetPlatformDisplayEXT");
	if (egl->procs.eglGetPlatformDisplayEXT == NULL) {
		wlf_log(WLF_ERROR, "eglGetPlatformDisplayEXT not supported");
		free(egl);
		return NULL;
	}

	if (check_egl_ext(client_exts_str, "EGL_KHR_debug")) {
		load_egl_proc(&egl->procs.eglDebugMessageControlKHR,
			"eglDebugMessageControlKHR");

		static const EGLAttrib debug_attribs[] = {
			EGL_DEBUG_MSG_CRITICAL_KHR, EGL_TRUE,
			EGL_DEBUG_MSG_ERROR_KHR, EGL_TRUE,
			EGL_DEBUG_MSG_WARN_KHR, EGL_TRUE,
			EGL_DEBUG_MSG_INFO_KHR, EGL_TRUE,
			EGL_NONE,
		};
		egl->procs.eglDebugMessageControlKHR(egl_log, debug_attribs);
		egl->exts.KHR_debug = true;
	}

	return egl;
}
