#include "wlf/util/wlf_signal.h"

#include <stdlib.h>

static void handle_noop(struct wlf_double_listener *listener, void *data) {
	/* Do nothing */
}

void wlf_signal_init(struct wlf_signal *signal) {
	wlf_double_list_init(&signal->listener_list);
}

void wlf_signal_add(struct wlf_signal *signal, struct wlf_double_listener *listener) {
	wlf_double_list_insert(signal->listener_list.prev, &listener->link);
}

struct wlf_double_listener *wlf_signal_get(
		struct wlf_signal *signal, wlf_notify_func_t notify) {
	struct wlf_double_listener *l;

	wlf_double_list_for_each(l, &signal->listener_list, link)
		if (l->notify == notify)
			return l;

	return NULL;

}

void wlf_signal_emit(struct wlf_signal *signal, void *data) {
	struct wlf_double_listener *l, *next;

	wlf_double_list_for_each_safe(l, next, &signal->listener_list, link)
		l->notify(l, data);

}

void wlf_signal_emit_mutable(struct wlf_signal *signal, void *data) {
	struct wlf_double_listener cursor;
	struct wlf_double_listener end;

	/* Add two special markers: one cursor and one end marker. This way, we
	 * know that we've already called listeners on the left of the cursor
	 * and that we don't want to call listeners on the right of the end
	 * marker. The 'it' function can remove any element it wants from the
	 * list without troubles.
	 *
	 * There was a previous attempt that used to steal the whole list of
	 * listeners but then that broke wlf_signal_get().
	 *
	 * wlf_double_list_for_each_safe tries to be safe but it fails: it works fine
	 * if the current item is removed, but not if the next one is. */
	wlf_double_list_insert(&signal->listener_list, &cursor.link);
	cursor.notify = handle_noop;
	wlf_double_list_insert(signal->listener_list.prev, &end.link);
	end.notify = handle_noop;

	while (cursor.link.next != &end.link) {
		struct wlf_double_list *pos = cursor.link.next;
		struct wlf_double_listener *l = wlf_container_of(pos, l, link);

		wlf_double_list_remove(&cursor.link);
		wlf_double_list_insert(pos, &cursor.link);

		l->notify(l, data);
	}

	wlf_double_list_remove(&cursor.link);
	wlf_double_list_remove(&end.link);
}
