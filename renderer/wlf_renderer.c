#include "wlf/renderer/wlf_renderer.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/config.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/renderer/vulkan/renderer.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct wlf_renderer *wlf_renderer_autocreate(struct wlf_backend *backend) {
	struct wlf_renderer *render = NULL;
#if WLF_HAS_LINUX_PLATFORM
	const char *render_options[] = {
		"auto",
		"vulkan",
		NULL
	};
	const char *render_name = render_options[wlf_env_parse_switch("WLF_RENDERER",
		render_options)];
	bool is_auto = strcmp(render_name, "auto") == 0;
	if (is_auto || strcmp(render_name, "vulkan") == 0) {
		render = wlf_vk_renderer_create_from_backend(backend);
		if (render == NULL) {
			wlf_log(WLF_ERROR, "Failed to create Vulkan render");
			return NULL;
		}
	}
#endif

	return render;
}

void wlf_renderer_destroy(struct wlf_renderer *render) {
	if (render == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&render->events.destroy, render);

	assert(wlf_linked_list_empty(&render->events.destroy.listener_list));

	if (render->impl && render->impl->destroy) {
		render->impl->destroy(render);
	} else {
		free(render);
	}
}

void wlf_renderer_init(struct wlf_renderer *render,
		const struct wlf_renderer_impl *impl) {
	assert(impl);
	assert(impl->destroy);

	render->impl = impl;

	wlf_signal_init(&render->events.destroy);
}
