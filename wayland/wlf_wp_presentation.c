#include "wlf/wayland/wlf_wp_presentation.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_utils.h"
#include "wayland/protocols/presentation-time-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void presentation_clock_id(void *data,
		struct wp_presentation *wp_presentation, uint32_t clk_id) {
	WLF_UNUSED(wp_presentation);

	struct wlf_wp_presentation *presentation = data;
	presentation->clk_id = clk_id;
	presentation->has_clock_id = true;
	wlf_signal_emit_mutable(&presentation->events.clock_id, presentation);
}

static const struct wp_presentation_listener presentation_listener = {
	.clock_id = presentation_clock_id,
};

static void feedback_finish(struct wlf_wp_presentation_feedback *feedback) {
	wlf_signal_emit_mutable(&feedback->events.destroy, feedback);
	assert(wlf_linked_list_empty(
		&feedback->events.sync_output.listener_list));
	assert(wlf_linked_list_empty(
		&feedback->events.presented.listener_list));
	assert(wlf_linked_list_empty(
		&feedback->events.discarded.listener_list));
	assert(wlf_linked_list_empty(&feedback->events.destroy.listener_list));

	free(feedback);
}

static void feedback_sync_output(void *data,
		struct wp_presentation_feedback *wp_feedback,
		struct wl_output *output) {
	WLF_UNUSED(wp_feedback);

	struct wlf_wp_presentation_feedback *feedback = data;
	wlf_signal_emit_mutable(&feedback->events.sync_output, output);
}

static void feedback_presented(void *data,
		struct wp_presentation_feedback *wp_feedback,
		uint32_t tv_sec_hi,
		uint32_t tv_sec_lo,
		uint32_t tv_nsec,
		uint32_t refresh,
		uint32_t seq_hi,
		uint32_t seq_lo,
		uint32_t flags) {
	WLF_UNUSED(wp_feedback);

	struct wlf_wp_presentation_feedback *feedback = data;
	feedback->base = NULL;

	feedback->tv_sec = ((uint64_t)tv_sec_hi << 32) | tv_sec_lo;
	feedback->tv_nsec = tv_nsec;
	feedback->refresh = refresh;
	feedback->seq = ((uint64_t)seq_hi << 32) | seq_lo;
	feedback->flags = flags;

	wlf_signal_emit_mutable(&feedback->events.presented, feedback);

	feedback_finish(feedback);
}

static void feedback_discarded(void *data,
		struct wp_presentation_feedback *wp_feedback) {
	WLF_UNUSED(wp_feedback);

	struct wlf_wp_presentation_feedback *feedback = data;
	feedback->base = NULL;

	wlf_signal_emit_mutable(&feedback->events.discarded, feedback);

	feedback_finish(feedback);
}

static const struct wp_presentation_feedback_listener feedback_listener = {
	.sync_output = feedback_sync_output,
	.presented = feedback_presented,
	.discarded = feedback_discarded,
};

struct wlf_wp_presentation *wlf_wp_presentation_create(
		struct wl_registry *wl_registry,
		uint32_t name,
		uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wp_presentation *presentation =
		malloc(sizeof(struct wlf_wp_presentation));
	if (presentation == NULL) {
		wlf_log_errno(
			WLF_ERROR, "Failed to allocate wlf_wp_presentation");
		return NULL;
	}

	presentation->base = NULL;
	presentation->version = 0;
	presentation->clk_id = 0;
	presentation->has_clock_id = false;
	wlf_signal_init(&presentation->events.clock_id);
	wlf_signal_init(&presentation->events.destroy);

	uint32_t bind_version = version;
	if (version > (uint32_t)wp_presentation_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server wp_presentation version %u is higher than "
			"client "
			"version %u, using client version",
			version, (uint32_t)wp_presentation_interface.version);
		bind_version = (uint32_t)wp_presentation_interface.version;
	}
	presentation->version = bind_version;

	presentation->base = wl_registry_bind(
		wl_registry, name, &wp_presentation_interface, bind_version);
	if (presentation->base == NULL) {
		wlf_log(WLF_ERROR,
			"Failed to bind wp_presentation interface (name: %u)",
			name);
		free(presentation);
		return NULL;
	}

	wp_presentation_add_listener(
		presentation->base, &presentation_listener, presentation);

	wlf_log(WLF_DEBUG,
		"Successfully bound wp_presentation (name: %u, version: %u)",
		name, bind_version);

	return presentation;
}

void wlf_wp_presentation_destroy(struct wlf_wp_presentation *presentation) {
	if (presentation == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&presentation->events.destroy, presentation);
	assert(wlf_linked_list_empty(
		&presentation->events.clock_id.listener_list));
	assert(wlf_linked_list_empty(
		&presentation->events.destroy.listener_list));

	if (presentation->base != NULL) {
		wp_presentation_destroy(presentation->base);
	}

	free(presentation);
}

struct wlf_wp_presentation_feedback *wlf_wp_presentation_request_feedback(
		struct wlf_wp_presentation *presentation,
		struct wl_surface *surface) {
	assert(presentation != NULL);
	assert(presentation->base != NULL);
	assert(surface != NULL);

	struct wlf_wp_presentation_feedback *feedback =
		calloc(1, sizeof(struct wlf_wp_presentation_feedback));
	if (feedback == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_wp_presentation_feedback");
		return NULL;
	}

	wlf_signal_init(&feedback->events.sync_output);
	wlf_signal_init(&feedback->events.presented);
	wlf_signal_init(&feedback->events.discarded);
	wlf_signal_init(&feedback->events.destroy);

	feedback->base = wp_presentation_feedback(presentation->base, surface);
	if (feedback->base == NULL) {
		wlf_log(WLF_ERROR, "wp_presentation_feedback() returned NULL");
		free(feedback);
		return NULL;
	}

	feedback->version =
		wl_proxy_get_version((struct wl_proxy *)feedback->base);
	wp_presentation_feedback_add_listener(
		feedback->base, &feedback_listener, feedback);

	return feedback;
}

void wlf_wp_presentation_feedback_destroy(struct wlf_wp_presentation_feedback *feedback) {
	if (feedback == NULL) {
		return;
	}

	if (feedback->base != NULL) {
		wl_proxy_destroy((struct wl_proxy *)feedback->base);
		feedback->base = NULL;
	}
	feedback_finish(feedback);
}
