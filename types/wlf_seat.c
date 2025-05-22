#include "wlf/types/wlf_seat.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

struct wlf_seat *wlf_seat_create(const char *name) {
	struct wlf_seat *seat =  malloc(sizeof(struct wlf_seat));
	if (seat == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_seat failed!");
		return NULL;
	}

	seat->name = strdup(name);
	wlf_signal_init(&seat->events.destroy);
	return seat;
}

void wlf_seat_destroy(struct wlf_seat *seat) {
	if (seat == NULL) {
		return;
	}

	if (seat->name) {
		free(seat->name);
	}

	wlf_signal_emit(&seat->events.destroy, seat);
	free(seat);
}
