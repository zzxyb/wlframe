#include "wlf/render/wlf_render.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/config.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/render/vulkan/vk_render.h"
#endif

#include <stdlib.h>
#include <string.h>

struct wlf_render *wlf_render_autocreate(struct wlf_backend *backend) {
	const char *render_options[] = {
		"auto",
		"vulkan",
		NULL
	};
	const char *render_name = render_options[wlf_env_parse_switch("WLR_RENDERER",
		render_options)];
	bool is_auto = strcmp(render_name, "auto") == 0;
	struct wlf_render *render = NULL;
#if WLF_HAS_LINUX_PLATFORM
	if (is_auto || strcmp(render_name, "vulkan") == 0) {
		render = wlf_vk_render_create_from_backend(backend);
		if (!render) {
			wlf_log(WLF_ERROR, "Failed to create Vulkan render");
			return NULL;
		}
	}
#endif

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
