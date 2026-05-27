#include "wlf/renderer/gles/renderer.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/renderer/gles/egl.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/config.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void renderer_destroy(struct wlf_renderer *render) {
	struct wlf_gles_renderer *renderer = wlf_gles_renderer_from_renderer(render);
	wlf_egl_unset_current(renderer->egl);
	wlf_egl_destroy(renderer->egl);
	free(renderer);
}

static struct wlf_texture *renderer_texture_from_buffer(
		struct wlf_renderer *renderer, struct wlf_buffer *buffer) {
	return NULL;
}

static const struct wlf_renderer_impl renderer_impl = {
	.destroy = renderer_destroy,
	.texture_from_buffer = renderer_texture_from_buffer,
};

bool wlf_renderer_is_gles(const struct wlf_renderer *renderer) {
	return renderer->impl == &renderer_impl;
}

struct wlf_gles_renderer *wlf_gles_renderer_from_renderer(
		struct wlf_renderer *renderer) {
	assert(renderer->impl == &renderer_impl);

	struct wlf_gles_renderer *gles_renderer =
		wlf_container_of(renderer, gles_renderer, base);

	return gles_renderer;
}

bool wlf_gles_renderer_check_ext(const struct wlf_gles_renderer *renderer,
		const char *ext) {
	if (renderer->exts_str == NULL) {
		return false;
	}
	return wlf_egl_check_ext(renderer->exts_str, ext);
}

const char *wlf_gles_error_str(GLenum error) {
	switch (error) {
	case GL_NO_ERROR:                      return "GL_NO_ERROR";
	case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
	default:                               return "unknown error";
	}
}

static void load_gl_procs(struct wlf_gles_renderer *renderer) {
	if (renderer->exts.OES_egl_image || renderer->exts.OES_egl_image_external) {
		wlf_elg_load_proc(&renderer->procs.glEGLImageTargetTexture2DOES,
			"glEGLImageTargetTexture2DOES");
	}
	if (renderer->exts.KHR_debug) {
		wlf_elg_load_proc(&renderer->procs.glDebugMessageCallbackKHR,
			"glDebugMessageCallbackKHR");
		wlf_elg_load_proc(&renderer->procs.glDebugMessageControlKHR,
			"glDebugMessageControlKHR");
		wlf_elg_load_proc(&renderer->procs.glPopDebugGroupKHR,
			"glPopDebugGroupKHR");
		wlf_elg_load_proc(&renderer->procs.glPushDebugGroupKHR,
			"glPushDebugGroupKHR");
	}
	if (renderer->exts.OES_egl_image) {
		wlf_elg_load_proc(&renderer->procs.glEGLImageTargetRenderbufferStorageOES,
			"glEGLImageTargetRenderbufferStorageOES");
	}
	if (renderer->exts.KHR_debug) {
		wlf_elg_load_proc(&renderer->procs.glGetGraphicsResetStatusKHR,
			"glGetGraphicsResetStatusKHR");
	}
	if (renderer->exts.EXT_disjoint_timer_query) {
		wlf_elg_load_proc(&renderer->procs.glGenQueriesEXT, "glGenQueriesEXT");
		wlf_elg_load_proc(&renderer->procs.glDeleteQueriesEXT, "glDeleteQueriesEXT");
		wlf_elg_load_proc(&renderer->procs.glQueryCounterEXT, "glQueryCounterEXT");
		wlf_elg_load_proc(&renderer->procs.glGetQueryObjectivEXT, "glGetQueryObjectivEXT");
		wlf_elg_load_proc(&renderer->procs.glGetQueryObjectui64vEXT,
			"glGetQueryObjectui64vEXT");
		wlf_elg_load_proc(&renderer->procs.glGetInteger64vEXT, "glGetInteger64vEXT");
	}
}

struct wlf_renderer *wlf_gles_renderer_create_from_backend(
		struct wlf_backend *backend) {
	bool force_sw = wlf_env_parse_bool("WLF_RENDER_FORCE_SOFTWARE");
	if (force_sw) {
#if WLF_HAS_LINUX_PLATFORM
		wlf_log(WLF_INFO, "WLF_RENDER_FORCE_SOFTWARE is set, "
			"forcing GLES software rasterizer");
		wlf_set_env("LIBGL_ALWAYS_SOFTWARE", "true");
#endif
	}

	struct wlf_egl *egl = wlf_egl_create(backend);
	if (egl == NULL) {
		return NULL;
	}

	if (!wlf_egl_make_current(egl, EGL_NO_SURFACE, EGL_NO_SURFACE)) {
		wlf_log(WLF_ERROR, "Failed to make EGL context current");
		wlf_egl_destroy(egl);
		return NULL;
	}

	struct wlf_gles_renderer *renderer = calloc(1, sizeof(*renderer));
	if (renderer == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_gles_renderer");
		wlf_egl_unset_current(egl);
		wlf_egl_destroy(egl);
		return NULL;
	}

	wlf_renderer_init(&renderer->base, &renderer_impl);
	renderer->egl = egl;

	renderer->base.features.damage =
		(egl->exts.KHR_swap_buffers_with_damage || egl->exts.EXT_swap_buffers_with_damage) &&
		egl->exts.EXT_buffer_age;

	renderer->exts_str = (const char *)glGetString(GL_EXTENSIONS);
	if (renderer->exts_str != NULL) {
		renderer->exts.EXT_read_format_bgra =
			wlf_egl_check_ext(renderer->exts_str, "GL_EXT_read_format_bgra");
		renderer->exts.KHR_debug =
			wlf_egl_check_ext(renderer->exts_str, "GL_KHR_debug");
		renderer->exts.OES_egl_image_external =
			wlf_egl_check_ext(renderer->exts_str, "GL_OES_EGL_image_external");
		renderer->exts.OES_egl_image =
			wlf_egl_check_ext(renderer->exts_str, "GL_OES_EGL_image");
		renderer->exts.EXT_texture_type_2_10_10_10_REV =
			wlf_egl_check_ext(renderer->exts_str, "GL_EXT_texture_type_2_10_10_10_REV");
		renderer->exts.OES_texture_half_float_linear =
			wlf_egl_check_ext(renderer->exts_str, "GL_OES_texture_half_float_linear");
		renderer->exts.EXT_texture_norm16 =
			wlf_egl_check_ext(renderer->exts_str, "GL_EXT_texture_norm16");
		renderer->exts.EXT_disjoint_timer_query =
			wlf_egl_check_ext(renderer->exts_str, "GL_EXT_disjoint_timer_query");
	}

	load_gl_procs(renderer);

	const char *gl_renderer = (const char *)glGetString(GL_RENDERER);
	bool is_software = gl_renderer != NULL &&
		(strstr(gl_renderer, "softpipe") != NULL ||
		 strstr(gl_renderer, "Software Rasterizer") != NULL ||
		 strstr(gl_renderer, "llvmpipe") != NULL);
	renderer->base.type = is_software ? CPU : GPU;

	if (force_sw) {
		if (!is_software) {
			wlf_log(WLF_ERROR, "WLF_RENDER_FORCE_SOFTWARE is set but GLES "
				"is running on hardware (%s); "
				"install a Mesa software driver (e.g. llvmpipe)",
				gl_renderer ? gl_renderer : "unknown");
			wlf_egl_unset_current(egl);
			wlf_egl_destroy(egl);
			free(renderer);
			return NULL;
		}
	} else {
		if (is_software) {
			wlf_egl_unset_current(egl);
			wlf_egl_destroy(egl);
			free(renderer);
			return NULL;
		}
	}

	wlf_log(WLF_INFO, "Creating GLES renderer");
	wlf_log(WLF_INFO, "Using %s", glGetString(GL_VERSION));
	wlf_log(WLF_INFO, "GL vendor: %s", glGetString(GL_VENDOR));
	wlf_log(WLF_INFO, "GL GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	wlf_log(WLF_INFO, "GL renderer: %s", gl_renderer);
	wlf_log(WLF_INFO, "Supported GLES extensions: %s", renderer->exts_str);

	return &renderer->base;
}
