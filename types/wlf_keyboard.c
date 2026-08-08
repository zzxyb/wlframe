#include "wlf/types/wlf_keyboard.h"

#include <assert.h>
#include <stdlib.h>

void wlf_keyboard_init(struct wlf_keyboard *keyboard,
		const struct wlf_keyboard_impl *impl) {
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);

	*keyboard = (struct wlf_keyboard){
		.impl = impl,
	};

	wlf_signal_init(&keyboard->events.destroy);
	wlf_signal_init(&keyboard->events.keymap);
	wlf_signal_init(&keyboard->events.enter);
	wlf_signal_init(&keyboard->events.leave);
	wlf_signal_init(&keyboard->events.key);
	wlf_signal_init(&keyboard->events.modifiers);
	wlf_signal_init(&keyboard->events.repeat_info);
}

void wlf_keyboard_destroy(struct wlf_keyboard *keyboard) {
	if (keyboard == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&keyboard->events.destroy, keyboard);

	assert(wlf_linked_list_empty(&keyboard->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&keyboard->events.keymap.listener_list));
	assert(wlf_linked_list_empty(&keyboard->events.enter.listener_list));
	assert(wlf_linked_list_empty(&keyboard->events.leave.listener_list));
	assert(wlf_linked_list_empty(&keyboard->events.key.listener_list));
	assert(wlf_linked_list_empty(&keyboard->events.modifiers.listener_list));
	assert(wlf_linked_list_empty(&keyboard->events.repeat_info.listener_list));

	keyboard->impl->destroy(keyboard);
}
