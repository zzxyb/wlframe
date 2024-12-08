#include "wlf/seat/wlf_pointer.h"
#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_time.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>

struct wlf_pointer *wlf_pointer_from_input_device(
		struct wlf_input_device *input_device) {
		return wlf_container_of(input_device, (struct wlf_pointer *)NULL, base);
	}

void wlf_pointer_init(struct wlf_pointer *pointer,
		const struct wlf_pointer_impl *impl, const char *name,
		enum wlf_input_device_type type) {
	*pointer = (struct wlf_pointer) {
			.impl = impl,
		};

	wlf_input_device_init(&pointer->base, type, name,
		WLF_INPUT_DEVICE_CAP_POINTER);

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

void wlf_pointer_finish(struct wlf_pointer *pointer) {
	int64_t time_msec = get_current_time_msec();
	while (pointer->button_count > 0) {
		struct wlf_pointer_button_event event = {
			.pointer = pointer,
			.time_msec = time_msec,
			.button = pointer->button_count - 1,
			.state = WLF_BUTTON_RELEASED,
		};
		wlf_pointer_notify_button(pointer, &event);
	}

	wlf_input_device_fnish(&pointer->base);
	free(pointer);
}

void wlf_pointer_notify_button(struct wlf_pointer *pointer,
		struct wlf_pointer_button_event *event) {
	if (event->state == WLF_BUTTON_PRESSED) {
		set_add(pointer->buttons, &pointer->button_count,
			WLF_POINTER_BUTTONS_CAP, event->button);
	} else {
		set_remove(pointer->buttons, &pointer->button_count,
			WLF_POINTER_BUTTONS_CAP, event->button);
	}

	wlf_signal_emit_mutable(&pointer->events.button, event);
}
