#include "wlf/types/wlf_touch.h"

#include <assert.h>
#include <stdlib.h>

void wlf_touch_init(struct wlf_touch *touch,
		const struct wlf_touch_impl *impl) {
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);

	*touch = (struct wlf_touch){
		.impl = impl,
	};

	wlf_signal_init(&touch->events.destroy);

	wlf_signal_init(&touch->events.down);
	wlf_signal_init(&touch->events.up);
	wlf_signal_init(&touch->events.motion);
	wlf_signal_init(&touch->events.cancel);
	wlf_signal_init(&touch->events.frame);
	wlf_signal_init(&touch->events.shape);
	wlf_signal_init(&touch->events.orientation);
}

void wlf_touch_destroy(struct wlf_touch *touch) {
	if (touch == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&touch->events.destroy, touch);

	assert(wlf_linked_list_empty(&touch->events.destroy.listener_list));

	assert(wlf_linked_list_empty(&touch->events.down.listener_list));
	assert(wlf_linked_list_empty(&touch->events.up.listener_list));
	assert(wlf_linked_list_empty(&touch->events.motion.listener_list));
	assert(wlf_linked_list_empty(&touch->events.cancel.listener_list));
	assert(wlf_linked_list_empty(&touch->events.frame.listener_list));
	assert(wlf_linked_list_empty(&touch->events.shape.listener_list));
	assert(wlf_linked_list_empty(&touch->events.orientation.listener_list));

	if (touch->impl && touch->impl->destroy) {
		touch->impl->destroy(touch);
	} else {
		free(touch);
	}
}
