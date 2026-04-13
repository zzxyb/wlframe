#include "wlf/shapes/wlf_shape.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static float wlf_shape_clampf(float v) {
	if (v < 0.0f) {
		return 0.0f;
	}
	if (v > 1.0f) {
		return 1.0f;
	}
	return v;
}

void wlf_shape_state_init(struct wlf_shape_state *paint) {
	if (paint == NULL) {
		return;
	}

	memset(paint, 0, sizeof(*paint));
	paint->fill_color = WLF_COLOR_BLACK;
	paint->stroke_color = WLF_COLOR_BLACK;
	paint->stroke_width = 1.0f;
	paint->opacity = 1.0f;
	paint->fill_opacity = 1.0f;
	paint->stroke_opacity = 1.0f;
	paint->has_fill = 1;
	paint->has_stroke = 0;
	paint->fill_gradient = NULL;
	paint->stroke_gradient = NULL;
}

float wlf_shape_state_fill_alpha(const struct wlf_shape_state *paint) {
	if (paint == NULL) {
		return 1.0f;
	}
	return wlf_shape_clampf(paint->opacity * paint->fill_opacity);
}

float wlf_shape_state_stroke_alpha(const struct wlf_shape_state *paint) {
	if (paint == NULL) {
		return 1.0f;
	}
	return wlf_shape_clampf(paint->opacity * paint->stroke_opacity);
}

void wlf_shape_init(struct wlf_shape *shape,
		const struct wlf_shape_impl *impl) {
	assert(impl);
	assert(impl->destroy);

	shape->impl = impl;
	wlf_linked_list_init(&shape->link);

	wlf_signal_init(&shape->events.destroy);
}

void wlf_shape_destroy(struct wlf_shape *shape) {
	if (shape == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&shape->events.destroy, shape);

	assert(wlf_linked_list_empty(&shape->events.destroy.listener_list));

	if (shape->impl && shape->impl->destroy) {
		shape->impl->destroy(shape);
	} else {
		free(shape);
	}
}

struct wlf_shape *wlf_shape_clone(struct wlf_shape *shape) {
	if (shape->impl->clone == NULL) {
		return NULL;
	}

	return shape->impl->clone(shape);
}
