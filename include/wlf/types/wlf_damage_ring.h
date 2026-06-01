#ifndef TYPES_WLF_DAMAGE_RING_H
#define TYPES_WLF_DAMAGE_RING_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/math/wlf_region.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_damage_ring_buffer {
	struct wlf_buffer *buffer;
	struct wlf_region damage;

	struct wlf_damage_ring *ring;
	struct wlf_linked_list link; // wlf_damage_ring.buffers

	struct wlf_listener destroy;
};

struct wlf_damage_ring {
	// Difference between the current buffer and the previous one
	struct wlf_region current;

	struct wlf_linked_list buffers; // wlf_damage_ring_buffer.link
};

void wlf_damage_ring_init(struct wlf_damage_ring *ring);

void wlf_damage_ring_finish(struct wlf_damage_ring *ring);
void wlf_damage_ring_add_region(struct wlf_damage_ring *ring,
	struct wlf_region *damage);
void wlf_damage_ring_add_box(struct wlf_damage_ring *ring,
	const struct wlf_frect *box);
void wlf_damage_ring_add_whole(struct wlf_damage_ring *ring);
void wlf_damage_ring_rotate_buffer(struct wlf_damage_ring *ring,
	struct wlf_buffer *buffer, struct wlf_region *damage);

#endif // TYPES_WLF_DAMAGE_RING_H