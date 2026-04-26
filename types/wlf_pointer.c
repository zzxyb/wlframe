#include "wlf/types/wlf_pointer.h"

#include <assert.h>
#include <stdlib.h>

void wlf_pointer_init(struct wlf_pointer *pointer,
		const struct wlf_pointer_impl *impl) {
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);

	*pointer = (struct wlf_pointer){
		.impl = impl,
	};

	wlf_signal_init(&pointer->events.destroy);

	wlf_signal_init(&pointer->events.motion);
	wlf_signal_init(&pointer->events.motion_absolute);
	wlf_signal_init(&pointer->events.button);
	wlf_signal_init(&pointer->events.axis);
	wlf_signal_init(&pointer->events.frame);

	wlf_signal_init(&pointer->events.swipe_begin);
	wlf_signal_init(&pointer->events.swipe_update);
	wlf_signal_init(&pointer->events.swipe_end);

	wlf_signal_init(&pointer->events.pinch_begin);
	wlf_signal_init(&pointer->events.pinch_update);
	wlf_signal_init(&pointer->events.pinch_end);

	wlf_signal_init(&pointer->events.hold_begin);
	wlf_signal_init(&pointer->events.hold_end);
}

void wlf_pointer_destroy(struct wlf_pointer *pointer) {
	if (pointer == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&pointer->events.destroy, pointer);

	assert(wlf_linked_list_empty(&pointer->events.destroy.listener_list));

	assert(wlf_linked_list_empty(&pointer->events.motion.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.motion_absolute.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.button.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.axis.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.frame.listener_list));

	assert(wlf_linked_list_empty(&pointer->events.swipe_begin.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.swipe_update.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.swipe_end.listener_list));

	assert(wlf_linked_list_empty(&pointer->events.pinch_begin.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.pinch_update.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.pinch_end.listener_list));

	assert(wlf_linked_list_empty(&pointer->events.hold_begin.listener_list));
	assert(wlf_linked_list_empty(&pointer->events.hold_end.listener_list));

	if (pointer->impl && pointer->impl->destroy) {
		pointer->impl->destroy(pointer);
	} else {
		free(pointer);
	}
}
