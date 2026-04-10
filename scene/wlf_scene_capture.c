#include "wlf/scene/wlf_scene_capture.h"

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"

#include <assert.h>
#include <stdlib.h>

static void scene_capture_frame_consider_destroy(
		struct wlf_scene_capture_frame *frame) {
	if (!frame->dropped || frame->n_locks > 0) {
		return;
	}

	wlf_signal_emit_mutable(&frame->events.destroy, frame);
	wlf_region_fini(&frame->damage);
	if (frame->drop_buffer) {
		wlf_buffer_drop(frame->buffer);
	}
	wlf_buffer_unlock(frame->buffer);

	assert(wlf_linked_list_empty(&frame->events.destroy.listener_list));
	free(frame);
}

static struct wlf_scene_capture_frame *scene_capture_frame_create(
		struct wlf_buffer *buffer, const struct wlf_region *damage,
		const struct timespec *timestamp, bool drop_buffer) {
	if (buffer == NULL) {
		return NULL;
	}

	struct wlf_scene_capture_frame *frame = calloc(1, sizeof(*frame));
	if (frame == NULL) {
		return NULL;
	}

	frame->buffer = wlf_buffer_lock(buffer);
	frame->drop_buffer = drop_buffer;
	frame->timestamp = timestamp != NULL ? *timestamp : (struct timespec){0};

	wlf_signal_init(&frame->events.destroy);
	wlf_region_init(&frame->damage);
	if (damage != NULL && !wlf_region_is_nil(damage)) {
		wlf_region_union(&frame->damage, damage);
	}

	return frame;
}

static void scene_capture_frame_drop(struct wlf_scene_capture_frame *frame) {
	if (frame == NULL) {
		return;
	}

	frame->dropped = true;
	scene_capture_frame_consider_destroy(frame);
}

static void scene_capture_emit_damage(struct wlf_scene_capture *capture,
		struct wlf_scene_node *origin, const struct wlf_region *damage) {
	struct wlf_scene_capture_damage_event event = {
		.capture = capture,
		.node = capture->node,
		.origin = origin,
		.damage = damage,
	};

	wlf_signal_emit_mutable(&capture->events.damage, &event);
}

static bool scene_capture_region_copy(struct wlf_region *dst,
		const struct wlf_region *src) {
	wlf_region_init(dst);
	if (src == NULL || wlf_region_is_nil(src)) {
		return false;
	}

	wlf_region_union(dst, src);
	return !wlf_region_is_nil(dst);
}

static bool scene_capture_region_translate(struct wlf_region *dst,
		const struct wlf_region *src, double dx, double dy) {
	wlf_region_init(dst);
	if (src == NULL || src->data == NULL) {
		return false;
	}

	for (long index = 0; index < src->data->numRects; ++index) {
		const struct wlf_frect *rect = &src->data->rects[index];
		struct wlf_frect translated = {
			.x = rect->x + dx,
			.y = rect->y + dy,
			.width = rect->width,
			.height = rect->height,
		};
		if (!wlf_region_add_rect(dst, &translated)) {
			wlf_region_fini(dst);
			wlf_region_init(dst);
			return false;
		}
	}

	return !wlf_region_is_nil(dst);
}

void wlf_scene_capture_init(struct wlf_scene_capture *capture,
		struct wlf_scene_node *node) {
	assert(capture != NULL);
	assert(node != NULL);

	*capture = (struct wlf_scene_capture){
		.node = node,
	};

	wlf_signal_init(&capture->events.destroy);
	wlf_signal_init(&capture->events.damage);
	wlf_signal_init(&capture->events.frame);
}

void wlf_scene_capture_finish(struct wlf_scene_capture *capture) {
	if (capture == NULL) {
		return;
	}

	if (capture->latest_frame != NULL) {
		capture->latest_frame->capture = NULL;
		scene_capture_frame_drop(capture->latest_frame);
		capture->latest_frame = NULL;
	}

	wlf_signal_emit_mutable(&capture->events.destroy, capture);

	assert(wlf_linked_list_empty(&capture->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&capture->events.damage.listener_list));
	assert(wlf_linked_list_empty(&capture->events.frame.listener_list));
}

struct wlf_scene_capture *wlf_scene_capture_lock(struct wlf_scene_capture *capture) {
	if (capture == NULL) {
		return NULL;
	}

	capture->n_locks++;
	return capture;
}

void wlf_scene_capture_unlock(struct wlf_scene_capture *capture) {
	assert(capture != NULL);
	assert(capture->n_locks > 0);

	capture->n_locks--;
}

struct wlf_scene_capture_frame *wlf_scene_capture_frame_lock(
		struct wlf_scene_capture_frame *frame) {
	if (frame == NULL) {
		return NULL;
	}

	frame->n_locks++;
	return frame;
}

void wlf_scene_capture_frame_unlock(struct wlf_scene_capture_frame *frame) {
	assert(frame != NULL);
	assert(frame->n_locks > 0);

	frame->n_locks--;
	scene_capture_frame_consider_destroy(frame);
}

struct wlf_scene_capture_frame *wlf_scene_capture_get_latest_frame(
		struct wlf_scene_capture *capture) {
	if (capture == NULL) {
		return NULL;
	}

	return wlf_scene_capture_frame_lock(capture->latest_frame);
}

bool wlf_scene_capture_output_frame(struct wlf_scene_capture *capture,
		const struct wlf_scene_capture_output_options *options) {
	if (capture == NULL || options == NULL || options->buffer == NULL) {
		return false;
	}

	struct wlf_scene_capture_frame *frame = scene_capture_frame_create(
		options->buffer, options->damage, options->timestamp,
		options->drop_buffer);
	if (frame == NULL) {
		return false;
	}

	frame->capture = capture;
	frame->sequence = ++capture->sequence;

	struct wlf_scene_capture_frame *old_frame = capture->latest_frame;
	capture->latest_frame = frame;
	if (old_frame != NULL) {
		old_frame->capture = NULL;
		scene_capture_frame_drop(old_frame);
	}

	struct wlf_scene_capture_frame_event event = {
		.capture = capture,
		.frame = frame,
	};
	wlf_signal_emit_mutable(&capture->events.frame, &event);

	return true;
}

void wlf_scene_capture_damage(struct wlf_scene_capture *capture,
		const struct wlf_region *damage) {
	if (capture == NULL || damage == NULL || wlf_region_is_nil(damage)) {
		return;
	}

	scene_capture_emit_damage(capture, capture->node, damage);
}

bool wlf_scene_node_is_captured(const struct wlf_scene_node *node) {
	return node != NULL && node->capture.n_locks > 0;
}

void wlf_scene_node_damage_capture(struct wlf_scene_node *node,
		const struct wlf_region *damage) {
	if (node == NULL || damage == NULL || wlf_region_is_nil(damage)) {
		return;
	}

	struct wlf_region translated;
	if (!scene_capture_region_copy(&translated, damage)) {
		return;
	}

	for (struct wlf_scene_node *iter = node; iter != NULL; iter =
			iter->parent != NULL ? &iter->parent->node : NULL) {
		scene_capture_emit_damage(&iter->capture, node, &translated);

		if (iter->parent == NULL) {
			break;
		}

		struct wlf_region next;
		if (!scene_capture_region_translate(&next, &translated,
				iter->state.x, iter->state.y)) {
			break;
		}

		wlf_region_fini(&translated);
		translated = next;
	}

	wlf_region_fini(&translated);
}

void wlf_scene_node_damage_whole_capture(struct wlf_scene_node *node) {
	if (node == NULL || node->state.width <= 0.0 || node->state.height <= 0.0) {
		return;
	}

	struct wlf_region damage;
	wlf_region_init(&damage);

	struct wlf_frect rect = {
		.x = 0.0,
		.y = 0.0,
		.width = node->state.width,
		.height = node->state.height,
	};
	if (!wlf_region_add_rect(&damage, &rect)) {
		wlf_region_fini(&damage);
		return;
	}

	wlf_scene_node_damage_capture(node, &damage);
	wlf_region_fini(&damage);
}
