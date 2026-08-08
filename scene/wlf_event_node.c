#include "wlf/scene/wlf_event_node.h"

#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static void event_node_destroy(struct wlf_scene_node *base) {
	struct wlf_event_node *node = wlf_event_node_from_node(base);
	if (base->window != NULL) {
		if (base->window->pointer_event_node == node) {
			base->window->pointer_event_node = NULL;
		}
		if (base->window->keyboard_event_node == node) {
			base->window->keyboard_event_node = NULL;
		}
		if (base->window->touch_event_node == node) {
			base->window->touch_event_node = NULL;
		}
	}
	struct wlf_signal *signals[] = {
		&node->events.pointer_enter, &node->events.pointer_leave,
		&node->events.pointer_motion, &node->events.pointer_button,
		&node->events.pointer_axis, &node->events.pointer_frame,
		&node->events.keyboard_enter, &node->events.keyboard_leave,
		&node->events.keyboard_keymap, &node->events.keyboard_key,
		&node->events.keyboard_modifiers, &node->events.keyboard_repeat_info,
		&node->events.tablet, &node->events.touch_down,
		&node->events.touch_up, &node->events.touch_motion,
		&node->events.touch_cancel, &node->events.touch_frame,
		&node->events.touch_shape, &node->events.touch_orientation,
	};
	for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
		assert(wlf_linked_list_empty(&signals[i]->listener_list));
	}
	pixman_region32_fini(&node->input_region);
	free(node);
}

static void event_node_get_size(struct wlf_scene_node *base,
		uint32_t *width, uint32_t *height) {
	struct wlf_event_node *node = wlf_event_node_from_node(base);
	pixman_box32_t *extents = pixman_region32_extents(&node->input_region);
	*width = extents->x2 > extents->x1 ?
		(uint32_t)(extents->x2 - extents->x1) : 0;
	*height = extents->y2 > extents->y1 ?
		(uint32_t)(extents->y2 - extents->y1) : 0;
}

static bool event_node_invisible(struct wlf_scene_node *base) {
	(void)base;
	return true;
}

static struct wlf_scene_node *event_node_at(struct wlf_scene_node *base,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_event_node *node = wlf_event_node_from_node(base);
	if (!base->state.enabled || !pixman_region32_contains_point(
			&node->input_region, (int)lx, (int)ly, NULL)) {
		return NULL;
	}
	if (nx != NULL) {
		*nx = lx;
	}
	if (ny != NULL) {
		*ny = ly;
	}
	return base;
}

static bool event_node_in_box(struct wlf_scene_node *base,
		struct wlf_frect *box, scene_node_box_iterator_func_t iterator,
		void *data) {
	int x = 0, y = 0;
	if (!base->state.enabled || !wlf_scene_node_coords(base, &x, &y)) {
		return false;
	}
	struct wlf_event_node *node = wlf_event_node_from_node(base);
	if (box->width <= 1.0 && box->height <= 1.0) {
		if (!pixman_region32_contains_point(&node->input_region,
				(int)floor(box->x - x), (int)floor(box->y - y), NULL)) {
			return false;
		}
		return iterator(base, x, y, data);
	}
	pixman_box32_t local = {
		.x1 = (int32_t)floor(box->x - x),
		.y1 = (int32_t)floor(box->y - y),
		.x2 = (int32_t)ceil(box->x + box->width - x),
		.y2 = (int32_t)ceil(box->y + box->height - y),
	};
	if (pixman_region32_contains_rectangle(&node->input_region, &local) ==
			PIXMAN_REGION_OUT) {
		return false;
	}
	return iterator(base, x, y, data);
}

static const struct wlf_scene_node_impl event_node_impl = {
	.destroy = event_node_destroy,
	.get_size = event_node_get_size,
	.invisible = event_node_invisible,
	.at = event_node_at,
	.in_box = event_node_in_box,
};

struct wlf_event_node *wlf_event_node_create(struct wlf_scene_node *parent,
		int x, int y, uint32_t width, uint32_t height) {
	if (parent == NULL) {
		return NULL;
	}
	struct wlf_event_node *node = calloc(1, sizeof(*node));
	if (node == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_event_node");
		return NULL;
	}
	wlf_scene_node_init(&node->base, &event_node_impl, parent);
	node->base.state.x = x;
	node->base.state.y = y;
	pixman_region32_init_rect(&node->input_region, 0, 0, width, height);

	struct wlf_signal *signals[] = {
		&node->events.pointer_enter, &node->events.pointer_leave,
		&node->events.pointer_motion, &node->events.pointer_button,
		&node->events.pointer_axis, &node->events.pointer_frame,
		&node->events.keyboard_enter, &node->events.keyboard_leave,
		&node->events.keyboard_keymap, &node->events.keyboard_key,
		&node->events.keyboard_modifiers, &node->events.keyboard_repeat_info,
		&node->events.tablet, &node->events.touch_down,
		&node->events.touch_up, &node->events.touch_motion,
		&node->events.touch_cancel, &node->events.touch_frame,
		&node->events.touch_shape, &node->events.touch_orientation,
	};
	for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
		wlf_signal_init(signals[i]);
	}
	return node;
}

void wlf_event_node_set_input_region(struct wlf_event_node *node,
		const pixman_region32_t *region) {
	assert(node != NULL);
	if (region == NULL) {
		pixman_region32_clear(&node->input_region);
	} else {
		pixman_region32_copy(&node->input_region,
			(pixman_region32_t *)region);
	}
}

void wlf_event_node_notify_pointer_enter(struct wlf_event_node *node,
		const struct wlf_event_pointer_focus_event *event) {
	assert(node != NULL);
	if (node->pointer_inside) {
		return;
	}
	node->pointer_inside = true;
	wlf_signal_emit_mutable(&node->events.pointer_enter, (void *)event);
}

void wlf_event_node_notify_pointer_leave(struct wlf_event_node *node,
		const struct wlf_event_pointer_focus_event *event) {
	assert(node != NULL);
	if (!node->pointer_inside) {
		return;
	}
	node->pointer_inside = false;
	wlf_signal_emit_mutable(&node->events.pointer_leave, (void *)event);
}

bool wlf_scene_node_is_event(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &event_node_impl;
}

struct wlf_event_node *wlf_event_node_from_node(struct wlf_scene_node *node) {
	assert(wlf_scene_node_is_event(node));
	struct wlf_event_node *event_node =
		wlf_container_of(node, event_node, base);
	return event_node;
}
