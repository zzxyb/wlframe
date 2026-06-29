#include "wlf/wayland/wlf_wl_data_offer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void data_offer_handle_offer(void *data,
		struct wl_data_offer *base, const char *mime_type) {
	(void)base;
	struct wlf_wl_data_offer *offer = data;
	struct wlf_wl_data_offer_mime_event event = {
		.offer = offer,
		.mime_type = mime_type,
	};
	wlf_signal_emit_mutable(&offer->events.offer, &event);
}

static void data_offer_handle_source_actions(void *data,
		struct wl_data_offer *base, uint32_t source_actions) {
	(void)base;
	struct wlf_wl_data_offer *offer = data;
	offer->source_actions = source_actions;
	wlf_signal_emit_mutable(&offer->events.source_actions, offer);
}

static void data_offer_handle_action(void *data,
		struct wl_data_offer *base, uint32_t dnd_action) {
	(void)base;
	struct wlf_wl_data_offer *offer = data;
	offer->dnd_action = dnd_action;
	wlf_signal_emit_mutable(&offer->events.action, offer);
}

static const struct wl_data_offer_listener wl_data_offer_listener = {
	.offer = data_offer_handle_offer,
	.source_actions = data_offer_handle_source_actions,
	.action = data_offer_handle_action,
};

struct wlf_wl_data_offer *wlf_wl_data_offer_wrap(
		struct wl_data_offer *wl_data_offer) {
	assert(wl_data_offer != NULL);

	struct wlf_wl_data_offer *offer = calloc(1, sizeof(*offer));
	if (offer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_data_offer");
		return NULL;
	}

	offer->wl_data_offer = wl_data_offer;

	wlf_signal_init(&offer->events.destroy);
	wlf_signal_init(&offer->events.offer);
	wlf_signal_init(&offer->events.source_actions);
	wlf_signal_init(&offer->events.action);

	wl_data_offer_add_listener(offer->wl_data_offer, &wl_data_offer_listener, offer);

	return offer;
}

void wlf_wl_data_offer_destroy(struct wlf_wl_data_offer *offer) {
	if (offer == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&offer->events.destroy, offer);
	wl_data_offer_destroy(offer->wl_data_offer);
	free(offer);
}

void wlf_wl_data_offer_accept(struct wlf_wl_data_offer *offer,
		uint32_t serial, const char *mime_type) {
	assert(offer != NULL);
	wl_data_offer_accept(offer->wl_data_offer, serial, mime_type);
}

void wlf_wl_data_offer_receive(struct wlf_wl_data_offer *offer,
		const char *mime_type, int fd) {
	assert(offer != NULL);
	wl_data_offer_receive(offer->wl_data_offer, mime_type, fd);
}

void wlf_wl_data_offer_finish(struct wlf_wl_data_offer *offer) {
	assert(offer != NULL);
	wl_data_offer_finish(offer->wl_data_offer);
}

void wlf_wl_data_offer_set_actions(struct wlf_wl_data_offer *offer,
		uint32_t dnd_actions, uint32_t preferred_action) {
	assert(offer != NULL);
	wl_data_offer_set_actions(offer->wl_data_offer, dnd_actions, preferred_action);
}
