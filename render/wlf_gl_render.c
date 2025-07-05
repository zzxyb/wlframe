#include "wlf/render/wlf_gl_render.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/platform/wlf_backend_wayland.h"
#include "wlf/render/wlf_egl.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

static void handle_gl_destroy(struct wlf_render *render)
{

}

static const struct wlf_render_impl gl_render_impl = {
	.type = WLF_RENDER_GLES,
	.destroy = handle_gl_destroy,
};

struct wlf_render *wlf_gl_render_create(struct wlf_backend *backend)
{
	struct wlf_backend_wayland *wayland_backend = wlf_backend_wayland_from_backend(backend);
	if (wayland_backend == NULL) {
		wlf_log(WLF_ERROR, "Failed to get Wayland backend from backend");
		return NULL;
	}
	struct wlf_egl *egl = wlf_egl_create(wayland_backend->display);
	if (egl == NULL) {
		wlf_log(WLF_ERROR, "Failed to create EGL context");
		return NULL;
	}

	if (!wlf_egl_make_current(egl, NULL)) {
		return NULL;
	}

	const char *exts_str = (const char *)glGetString(GL_EXTENSIONS);
	if (exts_str == NULL) {
		wlf_log(WLF_ERROR, "Failed to get GL_EXTENSIONS");
		return NULL;
	}

	struct wlf_gl_render *render = calloc(1, sizeof(struct wlf_gl_render));
	if (render == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlr_gles2_render failed!");
		return NULL;
	}

	wlf_render_init(&render->base, &gl_render_impl);
	render->egl = egl;
	wlf_log(WLF_INFO, "Creating GLES2 renderer");
	wlf_log(WLF_INFO, "Using %s", glGetString(GL_VERSION));
	wlf_log(WLF_INFO, "GL vendor: %s", glGetString(GL_VENDOR));
	wlf_log(WLF_INFO, "GL renderer: %s", glGetString(GL_RENDERER));
	wlf_log(WLF_INFO, "Supported GLES2 extensions: %s", exts_str);

	return  &render->base;
};

bool wlf_gl_render_check_ext(struct wlf_render *render, const char *ext) {
	return false;
}
