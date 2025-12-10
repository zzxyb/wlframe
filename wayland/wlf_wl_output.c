#include "wlf/wayland/wlf_wl_output.h"
#include "wlf/types/wlf_output.h"
#include "wlf/utils/wlf_log.h"
#include "protocols/xdg-output-unstable-v1-client-protocol.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void wl_output_geometry(void *data, struct wl_output *wl_output,int32_t x,
		int32_t y, int32_t physical_width, int32_t physical_height,
		int32_t subpixel, const char *make, const char *model,
		int32_t transform) {

}

static void wl_output_mode(void *data, struct wl_output *wl_output,
		uint32_t flags, int32_t width, int32_t height, int32_t refresh) {

}

static void wl_output_done(void *data, struct wl_output *wl_output) {

}

static void wl_output_scale(void *data, struct wl_output *wl_output,
		int32_t factor) {

}

static 	void wl_output_name(void *data, struct wl_output *wl_output,
		const char *name) {

}

static 	void wl_output_description(void *data, struct wl_output *wl_output,
		const char *description) {

}

static const struct wl_output_listener wl_output_listener = {
	.geometry = wl_output_geometry,
	.mode = wl_output_mode,
	.done = wl_output_done,
	.scale = wl_output_scale,
	.name = wl_output_name,
	.description = wl_output_description,
};

static void output_destroy(struct wlf_output *output) {
	struct wlf_wl_output *wl_output = wlf_wl_output_from_backend(output);
	wl_output_destroy(wl_output->output);
	zxdg_output_v1_destroy(wl_output->xdg_output);

	free(wl_output);
}

static const struct wlf_output_impl wlf_wl_output_impl = {
	.type = WLF_OUTPUT,
	.destroy = output_destroy,
};

struct wlf_output *wlf_output_create_from_wl_registry(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_output *output = malloc(sizeof(struct wlf_wl_output));
	if (output == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_output");
		return NULL;
	}

	output->output = NULL;
	output->xdg_output = NULL;

	uint32_t bind_version = version;
	if (version > (uint32_t)wl_output_interface.version) {
		wlf_log(WLF_DEBUG, "Server wl_output version %u is higher than client version %u, "
				"using client version", version, (uint32_t)wl_output_interface.version);
		bind_version = (uint32_t)wl_output_interface.version;
	}

	output->output = wl_registry_bind(wl_registry, name, &wl_output_interface, bind_version);
	if (output->output == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_output interface with name %u", name);
		free(output);
		return NULL;
	}
	wl_output_add_listener(output->output, &wl_output_listener, output);
	wlf_output_init(&output->base, &wlf_wl_output_impl);
	wlf_log(WLF_DEBUG, "Successfully bound wl_output interface (name: %u, version: %u)",
			name, bind_version);

	return &output->base;
}

bool wlf_output_is_wayland(const struct wlf_output *output) {
	return (output && output->impl == &wlf_wl_output_impl);
}

struct wlf_wl_output *wlf_wl_output_from_backend(struct wlf_output *output) {
	assert(output && output->impl == &wlf_wl_output_impl);
	struct wlf_wl_output *wl_output = wlf_container_of(output, wl_output, base);

	return wl_output;
}

static void output_manager_destroy(struct wlf_output_manager *manager) {
	struct wlf_wl_output_manager *wl_output_manager =
		wlf_wl_output_manager_from_backend(manager);
	zxdg_output_manager_v1_destroy(wl_output_manager->manager);

	free(wl_output_manager);
}

static const struct wlf_output_manager_impl wlf_wl_output_manager_impl = {
	.destroy = output_manager_destroy,
};

struct wlf_output_manager *wlf_output_manager_create_from_wl_registry(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_output_manager *manager = malloc(sizeof(struct wlf_wl_output_manager));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_output_manager");
		return NULL;
	}

	manager->manager = NULL;

	uint32_t bind_version = version;
	if (version > (uint32_t)zxdg_output_manager_v1_interface.version) {
		wlf_log(WLF_DEBUG, "Server zxdg_output_manager_v1 version %u is higher than client version %u, "
				"using client version", version, (uint32_t)zxdg_output_manager_v1_interface.version);
		bind_version = (uint32_t)zxdg_output_manager_v1_interface.version;
	}

	manager->manager = wl_registry_bind(wl_registry, name, &zxdg_output_manager_v1_interface, bind_version);
	if (manager->manager == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind zxdg_output_manager_v1 interface with name %u", name);
		free(manager);
		return NULL;
	}

	wlf_output_manager_init(&manager->base, &wlf_wl_output_manager_impl);
	wlf_log(WLF_DEBUG, "Successfully bound zxdg_output_manager_v1 interface (name: %u, version: %u)",
			name, bind_version);

	return &manager->base;
}

bool wlf_wl_output_manager_is_wayland(const struct wlf_output_manager *manager) {
	return (manager && manager->impl == &wlf_wl_output_manager_impl);
}

struct wlf_wl_output_manager *wlf_wl_output_manager_from_backend(
		struct wlf_output_manager *manager) {
	assert(manager && manager->impl == &wlf_wl_output_manager_impl);
	struct wlf_wl_output_manager *wl_output_manager = wlf_container_of(
		manager, wl_output_manager, base);

	return wl_output_manager;
}
