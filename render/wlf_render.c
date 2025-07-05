#include "wlf/render/wlf_render.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/render/wlf_gl_render.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"

#include <stdlib.h>
#include <string.h>

struct wlf_render *wlf_render_autocreate(struct wlf_backend *backend) {
	const char *renderer_options[] = {
		"auto",
		"gles2",
		"vulkan",
		"pixman",
		NULL
	};
	const char *renderer_name = renderer_options[wlf_env_parse_switch("WLR_RENDERER", renderer_options)];
	bool is_auto = strcmp(renderer_name, "auto") == 0;
	struct wlf_render *render = NULL;
	if (is_auto || strcmp(renderer_name, "gles2") == 0) {
		render = wlf_gl_render_create(backend);
		if (!render) {
			wlf_log(WLF_ERROR, "Failed to create GLES2 renderer");
			return NULL;
		}
	}

	return render;
}

void wlf_render_destroy(struct wlf_render *render) {
	if (render == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&render->events.destroy, render);

	if (render->impl && render->impl->destroy) {
		render->impl->destroy(render);
	} else {
		free(render);
	}
}

void wlf_render_init(struct wlf_render *render, const struct wlf_render_impl *impl)
{
	if (render == NULL || impl == NULL) {
		return;
	}

	render->impl = impl;
	wlf_signal_init(&render->events.destroy);
}
