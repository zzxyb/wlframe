#include "wlf/wayland/wlf_wl_surface.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>

#include <wayland-client-protocol.h>

static void surface_handle_enter(void *data, struct wl_surface *base,
		struct wl_output *output) {
	WLF_UNUSED(base);

	struct wlf_wl_surface *surface = data;
	struct wlf_wl_surface_output_event event = {
		.surface = surface,
		.output = output,
	};
	wlf_signal_emit_mutable(&surface->events.enter, &event);
}

static void surface_handle_leave(void *data, struct wl_surface *base,
		struct wl_output *output) {
	WLF_UNUSED(base);

	struct wlf_wl_surface *surface = data;
	struct wlf_wl_surface_output_event event = {
		.surface = surface,
		.output = output,
	};
	wlf_signal_emit_mutable(&surface->events.leave, &event);
}

static void surface_handle_preferred_buffer_scale(void *data,
		struct wl_surface *base, int32_t factor) {
	WLF_UNUSED(base);

	struct wlf_wl_surface *surface = data;
	surface->preferred_buffer_scale = factor;
	wlf_signal_emit_mutable(&surface->events.preferred_buffer_scale, surface);
}

static void surface_handle_preferred_buffer_transform(void *data,
		struct wl_surface *base, uint32_t transform) {
	WLF_UNUSED(base);

	struct wlf_wl_surface *surface = data;
	surface->preferred_buffer_transform = transform;
	wlf_signal_emit_mutable(&surface->events.preferred_buffer_transform, surface);
}

static const struct wl_surface_listener wl_surface_listener = {
	.enter = surface_handle_enter,
	.leave = surface_handle_leave,
	.preferred_buffer_scale = surface_handle_preferred_buffer_scale,
	.preferred_buffer_transform = surface_handle_preferred_buffer_transform,
};

struct wlf_wl_surface *wlf_wl_surface_create(struct wlf_wl_compositor *compositor) {
	assert(compositor != NULL);

	struct wlf_wl_surface *surface = calloc(1, sizeof(*surface));
	if (surface == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_surface");
		return NULL;
	}

	surface->wl_surface = wl_compositor_create_surface(compositor->base);
	if (surface->wl_surface == NULL) {
		wlf_log(WLF_ERROR, "wl_compositor_create_surface failed");
		free(surface);
		return NULL;
	}

	surface->version =
		wl_proxy_get_version((struct wl_proxy *)surface->wl_surface);
	surface->preferred_buffer_scale = 1;
	surface->preferred_buffer_transform = 0;
	surface->wl_compositor = compositor->base;

	wlf_signal_init(&surface->events.destroy);
	wlf_signal_init(&surface->events.enter);
	wlf_signal_init(&surface->events.leave);
	wlf_signal_init(&surface->events.preferred_buffer_scale);
	wlf_signal_init(&surface->events.preferred_buffer_transform);

	wl_surface_add_listener(surface->wl_surface, &wl_surface_listener, surface);

	return surface;
}

void wlf_wl_surface_destroy(struct wlf_wl_surface *surface) {
	if (surface == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&surface->events.destroy, surface);

	assert(wlf_linked_list_empty(&surface->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&surface->events.enter.listener_list));
	assert(wlf_linked_list_empty(&surface->events.leave.listener_list));
	assert(wlf_linked_list_empty(&surface->events.preferred_buffer_scale.listener_list));
	assert(wlf_linked_list_empty(&surface->events.preferred_buffer_transform.listener_list));

	if (surface->wl_surface != NULL) {
		wl_surface_destroy(surface->wl_surface);
	}

	free(surface);
}

void wlf_wl_surface_attach(struct wlf_wl_surface *surface,
		struct wl_buffer *buffer, int32_t x, int32_t y) {
	assert(surface != NULL);

	wl_surface_attach(surface->wl_surface, buffer, x, y);
}

void wlf_wl_surface_damage(struct wlf_wl_surface *surface,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	assert(surface != NULL);

	wl_surface_damage(surface->wl_surface, x, y, width, height);
}

void wlf_wl_surface_damage_buffer(struct wlf_wl_surface *surface,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	if (surface->version < WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION) {
		return;
	}

	assert(surface != NULL);

	wl_surface_damage_buffer(surface->wl_surface, x, y, width, height);
}

struct wl_callback *wlf_wl_surface_frame(struct wlf_wl_surface *surface) {
	assert(surface != NULL);

	return wl_surface_frame(surface->wl_surface);
}

void wlf_wl_surface_set_opaque_region(struct wlf_wl_surface *surface,
		struct wlf_region *region) {
	assert(surface != NULL);

	struct wl_region *wl_region = NULL;

	if (!wlf_region_is_nil(region)) {
		wl_region = wl_compositor_create_region(surface->wl_compositor);
		for (long i = 0; i < region->data->numRects; ++i) {
			const struct wlf_frect *r = &region->data->rects[i];
			wl_region_add(wl_region, r->x, r->y, r->width, r->height);
		}
	}

	wl_surface_set_opaque_region(surface->wl_surface, wl_region);

	if (wl_region != NULL) {
		wl_region_destroy(wl_region);
	}
}

void wlf_wl_surface_set_input_region(struct wlf_wl_surface *surface,
		struct wlf_region *region) {
	assert(surface != NULL);

	struct wl_region *wl_region = NULL;
	if (!wlf_region_is_nil(region)) {
		wl_region = wl_compositor_create_region(surface->wl_compositor);
		for (long i = 0; i < region->data->numRects; ++i) {
			const struct wlf_frect *r = &region->data->rects[i];
			wl_region_add(wl_region, r->x, r->y, r->width, r->height);
		}
	}

	wl_surface_set_input_region(surface->wl_surface, wl_region);

	if (wl_region != NULL) {
		wl_region_destroy(wl_region);
	}
}

void wlf_wl_surface_set_buffer_transform(struct wlf_wl_surface *surface,
		int32_t transform) {
	if (surface->version < WL_SURFACE_SET_BUFFER_TRANSFORM_SINCE_VERSION) {
		return;
	}

	assert(surface != NULL);

	wl_surface_set_buffer_transform(surface->wl_surface, transform);
}

void wlf_wl_surface_set_buffer_scale(struct wlf_wl_surface *surface,
		int32_t scale) {
	if (surface->version < WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION) {
		return;
	}

	assert(surface != NULL);

	wl_surface_set_buffer_scale(surface->wl_surface, scale);
}

void wlf_wl_surface_offset(struct wlf_wl_surface *surface,
		int32_t x, int32_t y) {
	if (surface->version < WL_SURFACE_OFFSET_SINCE_VERSION) {
		return;
	}

	assert(surface != NULL);

	wl_surface_offset(surface->wl_surface, x, y);
}

void wlf_wl_surface_commit(struct wlf_wl_surface *surface) {
	assert(surface != NULL);

	wl_surface_commit(surface->wl_surface);
}
