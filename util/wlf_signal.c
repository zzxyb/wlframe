#include "wlf/utils/wlf_signal.h"

#include <stdlib.h>

static void handle_noop(struct wlf_listener *listener, void *data) {
	/* Do nothing */
}

void wlf_signal_init(struct wlf_signal *signal) {
	wlf_double_list_init(&signal->listener_list);
}

void wlf_signal_add(struct wlf_signal *signal, struct wlf_listener *listener) {
	wlf_double_list_insert(signal->listener_list.prev, &listener->link);
}

struct wlf_listener *wlf_signal_get(
		struct wlf_signal *signal, wlf_notify_func_t notify) {
	struct wlf_listener *l;

	wlf_double_list_for_each(l, &signal->listener_list, link)
		if (l->notify == notify)
			return l;

	return NULL;

}

void wlf_signal_emit(struct wlf_signal *signal, void *data) {
	struct wlf_listener *l, *next;

	wlf_double_list_for_each_safe(l, next, &signal->listener_list, link)
		l->notify(l, data);

}

void wlf_signal_emit_mutable(struct wlf_signal *signal, void *data) {
	struct wlf_listener cursor;
	struct wlf_listener end;

	wlf_double_list_insert(&signal->listener_list, &cursor.link);
	cursor.notify = handle_noop;
	wlf_double_list_insert(signal->listener_list.prev, &end.link);
	end.notify = handle_noop;

	while (cursor.link.next != &end.link) {
		struct wlf_double_list *pos = cursor.link.next;
		struct wlf_listener *l = wlf_container_of(pos, l, link);

		wlf_double_list_remove(&cursor.link);
		wlf_double_list_insert(pos, &cursor.link);

		l->notify(l, data);
	}

	wlf_double_list_remove(&cursor.link);
	wlf_double_list_remove(&end.link);
}
