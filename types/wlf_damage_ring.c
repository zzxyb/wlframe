#include "wlf/types/wlf_damage_ring.h"
#include "wlf/utils/wlf_utils.h"

#include <float.h>
#include <stdlib.h>

static const struct wlf_frect damage_ring_whole_rect = {
	.x = -DBL_MAX / 4.0,
	.y = -DBL_MAX / 4.0,
	.width = DBL_MAX / 2.0,
	.height = DBL_MAX / 2.0,
};

static void damage_ring_region_clear(struct wlf_region *region) {
	wlf_region_fini(region);
	wlf_region_init(region);
}

static void damage_ring_region_copy(struct wlf_region *dst,
	const struct wlf_region *src) {
	damage_ring_region_clear(dst);

	if (src == NULL || wlf_region_is_nil(src)) {
		return;
	}

	wlf_region_union(dst, src);
}

static void damage_ring_buffer_finish(struct wlf_damage_ring_buffer *ring_buffer) {
	wlf_linked_list_remove(&ring_buffer->destroy.link);
	wlf_linked_list_remove(&ring_buffer->link);
	wlf_region_fini(&ring_buffer->damage);
	free(ring_buffer);
}

static void damage_ring_buffer_handle_destroy(struct wlf_listener *listener,
		void *data) {
	struct wlf_damage_ring_buffer *ring_buffer =
		wlf_container_of(listener, ring_buffer, destroy);

	WLF_UNUSED(data);
	damage_ring_buffer_finish(ring_buffer);
}

static struct wlf_damage_ring_buffer *damage_ring_find_buffer(
		struct wlf_damage_ring *ring, struct wlf_buffer *buffer) {
	struct wlf_damage_ring_buffer *ring_buffer;

	wlf_linked_list_for_each(ring_buffer, &ring->buffers, link) {
		if (ring_buffer->buffer == buffer) {
			return ring_buffer;
		}
	}

	return NULL;
}

static struct wlf_damage_ring_buffer *damage_ring_get_or_create_buffer(
		struct wlf_damage_ring *ring, struct wlf_buffer *buffer) {
	struct wlf_damage_ring_buffer *ring_buffer =
		damage_ring_find_buffer(ring, buffer);
	if (ring_buffer != NULL) {
		return ring_buffer;
	}

	ring_buffer = calloc(1, sizeof(*ring_buffer));
	if (ring_buffer == NULL) {
		return NULL;
	}

	ring_buffer->buffer = buffer;
	ring_buffer->ring = ring;
	wlf_region_init(&ring_buffer->damage);
	wlf_region_add_rect(&ring_buffer->damage, &(struct wlf_frect){
		.x = 0.0,
		.y = 0.0,
		.width = buffer->width,
		.height = buffer->height,
	});
	ring_buffer->destroy.notify = damage_ring_buffer_handle_destroy;
	wlf_linked_list_insert(&ring->buffers, &ring_buffer->link);
	wlf_signal_add(&buffer->events.destroy, &ring_buffer->destroy);

	return ring_buffer;
}

void wlf_damage_ring_init(struct wlf_damage_ring *ring) {
	wlf_region_init(&ring->current);
	wlf_linked_list_init(&ring->buffers);
}

void wlf_damage_ring_finish(struct wlf_damage_ring *ring) {
	if (ring == NULL) {
		return;
	}

	while (!wlf_linked_list_empty(&ring->buffers)) {
		struct wlf_damage_ring_buffer *ring_buffer =
			wlf_container_of(ring->buffers.next, ring_buffer, link);
		damage_ring_buffer_finish(ring_buffer);
	}

	wlf_region_fini(&ring->current);
}

void wlf_damage_ring_add_region(struct wlf_damage_ring *ring,
		struct wlf_region *damage) {
	if (ring == NULL || damage == NULL) {
		return;
	}

	wlf_region_union(&ring->current, damage);
}

void wlf_damage_ring_add_box(struct wlf_damage_ring *ring,
		const struct wlf_frect *box) {
	if (ring == NULL || box == NULL) {
		return;
	}

	wlf_region_add_rect(&ring->current, box);
}

void wlf_damage_ring_add_whole(struct wlf_damage_ring *ring) {
	if (ring == NULL) {
		return;
	}

	wlf_region_add_rect(&ring->current, &damage_ring_whole_rect);
}

void wlf_damage_ring_rotate_buffer(struct wlf_damage_ring *ring,
		struct wlf_buffer *buffer, struct wlf_region *damage) {
	struct wlf_damage_ring_buffer *target;
	struct wlf_damage_ring_buffer *ring_buffer;

	if (ring == NULL || buffer == NULL) {
		return;
	}

	target = damage_ring_get_or_create_buffer(ring, buffer);
	if (target == NULL) {
		return;
	}

	if (damage != NULL) {
		damage_ring_region_copy(damage, &ring->current);
		wlf_region_union(damage, &target->damage);
	}

	wlf_linked_list_for_each(ring_buffer, &ring->buffers, link) {
		if (ring_buffer == target) {
			continue;
		}

		wlf_region_union(&ring_buffer->damage, &ring->current);
	}

	damage_ring_region_clear(&target->damage);
	damage_ring_region_clear(&ring->current);
}
