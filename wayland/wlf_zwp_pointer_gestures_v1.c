/**
 * @file        wlf_zwp_pointer_gestures_v1.c
 * @brief       Wayland pointer-gestures-unstable-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zwp_pointer_gestures_v1.h"
#include "wayland/protocols/pointer-gestures-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Swipe gesture listeners
 * ---------------------------------------------------------------------- */

static void swipe_handle_begin(void *data,
	struct zwp_pointer_gesture_swipe_v1 *base,
	uint32_t serial, uint32_t time,
	struct wl_surface *surface, uint32_t fingers)
{
	struct wlf_zwp_pointer_gesture_swipe_v1 *swipe = data;
	(void)base;
	swipe->current.serial  = serial;
	swipe->current.time    = time;
	swipe->current.surface = surface;
	swipe->current.fingers = fingers;
	wlf_signal_emit_mutable(&swipe->events.begin, swipe);
}

static void swipe_handle_update(void *data,
	struct zwp_pointer_gesture_swipe_v1 *base,
	uint32_t time, wl_fixed_t dx, wl_fixed_t dy)
{
	struct wlf_zwp_pointer_gesture_swipe_v1 *swipe = data;
	(void)base;
	swipe->current.time = time;
	swipe->current.dx   = wl_fixed_to_double(dx);
	swipe->current.dy   = wl_fixed_to_double(dy);
	wlf_signal_emit_mutable(&swipe->events.update, swipe);
}

static void swipe_handle_end(void *data,
	struct zwp_pointer_gesture_swipe_v1 *base,
	uint32_t serial, uint32_t time, int32_t cancelled)
{
	struct wlf_zwp_pointer_gesture_swipe_v1 *swipe = data;
	(void)base;
	swipe->current.serial    = serial;
	swipe->current.time      = time;
	swipe->current.cancelled = cancelled;
	wlf_signal_emit_mutable(&swipe->events.end, swipe);
}

static const struct zwp_pointer_gesture_swipe_v1_listener swipe_listener = {
	.begin  = swipe_handle_begin,
	.update = swipe_handle_update,
	.end    = swipe_handle_end,
};

/* -------------------------------------------------------------------------
 * Pinch gesture listeners
 * ---------------------------------------------------------------------- */

static void pinch_handle_begin(void *data,
	struct zwp_pointer_gesture_pinch_v1 *base,
	uint32_t serial, uint32_t time,
	struct wl_surface *surface, uint32_t fingers)
{
	struct wlf_zwp_pointer_gesture_pinch_v1 *pinch = data;
	(void)base;
	pinch->current.serial  = serial;
	pinch->current.time    = time;
	pinch->current.surface = surface;
	pinch->current.fingers = fingers;
	wlf_signal_emit_mutable(&pinch->events.begin, pinch);
}

static void pinch_handle_update(void *data,
	struct zwp_pointer_gesture_pinch_v1 *base,
	uint32_t time, wl_fixed_t dx, wl_fixed_t dy,
	wl_fixed_t scale, wl_fixed_t rotation)
{
	struct wlf_zwp_pointer_gesture_pinch_v1 *pinch = data;
	(void)base;
	pinch->current.time     = time;
	pinch->current.dx       = wl_fixed_to_double(dx);
	pinch->current.dy       = wl_fixed_to_double(dy);
	pinch->current.scale    = wl_fixed_to_double(scale);
	pinch->current.rotation = wl_fixed_to_double(rotation);
	wlf_signal_emit_mutable(&pinch->events.update, pinch);
}

static void pinch_handle_end(void *data,
	struct zwp_pointer_gesture_pinch_v1 *base,
	uint32_t serial, uint32_t time, int32_t cancelled)
{
	struct wlf_zwp_pointer_gesture_pinch_v1 *pinch = data;
	(void)base;
	pinch->current.serial    = serial;
	pinch->current.time      = time;
	pinch->current.cancelled = cancelled;
	wlf_signal_emit_mutable(&pinch->events.end, pinch);
}

static const struct zwp_pointer_gesture_pinch_v1_listener pinch_listener = {
	.begin  = pinch_handle_begin,
	.update = pinch_handle_update,
	.end    = pinch_handle_end,
};

/* -------------------------------------------------------------------------
 * Hold gesture listeners
 * ---------------------------------------------------------------------- */

static void hold_handle_begin(void *data,
	struct zwp_pointer_gesture_hold_v1 *base,
	uint32_t serial, uint32_t time,
	struct wl_surface *surface, uint32_t fingers)
{
	struct wlf_zwp_pointer_gesture_hold_v1 *hold = data;
	(void)base;
	hold->current.serial  = serial;
	hold->current.time    = time;
	hold->current.surface = surface;
	hold->current.fingers = fingers;
	wlf_signal_emit_mutable(&hold->events.begin, hold);
}

static void hold_handle_end(void *data,
	struct zwp_pointer_gesture_hold_v1 *base,
	uint32_t serial, uint32_t time, int32_t cancelled)
{
	struct wlf_zwp_pointer_gesture_hold_v1 *hold = data;
	(void)base;
	hold->current.serial    = serial;
	hold->current.time      = time;
	hold->current.cancelled = cancelled;
	wlf_signal_emit_mutable(&hold->events.end, hold);
}

static const struct zwp_pointer_gesture_hold_v1_listener hold_listener = {
	.begin = hold_handle_begin,
	.end   = hold_handle_end,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_zwp_pointer_gestures_v1 *wlf_zwp_pointer_gestures_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)zwp_pointer_gestures_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwp_pointer_gestures_v1 *gestures =
		calloc(1, sizeof(*gestures));
	if (!gestures) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_pointer_gestures_v1");
		return NULL;
	}

	gestures->base = wl_registry_bind(wl_registry, name,
		&zwp_pointer_gestures_v1_interface, bind_ver);
	if (!gestures->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zwp_pointer_gestures_v1 (name: %u)", name);
		free(gestures);
		return NULL;
	}

	wlf_signal_init(&gestures->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zwp_pointer_gestures_v1 (name: %u, version: %u)",
		name, bind_ver);

	return gestures;
}

void wlf_zwp_pointer_gestures_v1_destroy(
	struct wlf_zwp_pointer_gestures_v1 *gestures)
{
	if (!gestures) {
		return;
	}

	wlf_signal_emit_mutable(&gestures->events.destroy, gestures);
	zwp_pointer_gestures_v1_release(gestures->base);
	free(gestures);
}

struct wlf_zwp_pointer_gesture_swipe_v1 *
wlf_zwp_pointer_gestures_v1_get_swipe_gesture(
	struct wlf_zwp_pointer_gestures_v1 *gestures,
	struct wl_pointer *pointer)
{
	assert(gestures);
	assert(gestures->base);
	assert(pointer);

	struct wlf_zwp_pointer_gesture_swipe_v1 *swipe =
		calloc(1, sizeof(*swipe));
	if (!swipe) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_pointer_gesture_swipe_v1");
		return NULL;
	}

	swipe->base = zwp_pointer_gestures_v1_get_swipe_gesture(
		gestures->base, pointer);
	if (!swipe->base) {
		wlf_log(WLF_ERROR,
			"zwp_pointer_gestures_v1_get_swipe_gesture() returned NULL");
		free(swipe);
		return NULL;
	}

	wlf_signal_init(&swipe->events.begin);
	wlf_signal_init(&swipe->events.update);
	wlf_signal_init(&swipe->events.end);
	wlf_signal_init(&swipe->events.destroy);

	zwp_pointer_gesture_swipe_v1_add_listener(swipe->base, &swipe_listener,
		swipe);

	return swipe;
}

struct wlf_zwp_pointer_gesture_pinch_v1 *
wlf_zwp_pointer_gestures_v1_get_pinch_gesture(
	struct wlf_zwp_pointer_gestures_v1 *gestures,
	struct wl_pointer *pointer)
{
	assert(gestures);
	assert(gestures->base);
	assert(pointer);

	struct wlf_zwp_pointer_gesture_pinch_v1 *pinch =
		calloc(1, sizeof(*pinch));
	if (!pinch) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_pointer_gesture_pinch_v1");
		return NULL;
	}

	pinch->base = zwp_pointer_gestures_v1_get_pinch_gesture(
		gestures->base, pointer);
	if (!pinch->base) {
		wlf_log(WLF_ERROR,
			"zwp_pointer_gestures_v1_get_pinch_gesture() returned NULL");
		free(pinch);
		return NULL;
	}

	wlf_signal_init(&pinch->events.begin);
	wlf_signal_init(&pinch->events.update);
	wlf_signal_init(&pinch->events.end);
	wlf_signal_init(&pinch->events.destroy);

	zwp_pointer_gesture_pinch_v1_add_listener(pinch->base, &pinch_listener,
		pinch);

	return pinch;
}

struct wlf_zwp_pointer_gesture_hold_v1 *
wlf_zwp_pointer_gestures_v1_get_hold_gesture(
	struct wlf_zwp_pointer_gestures_v1 *gestures,
	struct wl_pointer *pointer)
{
	assert(gestures);
	assert(gestures->base);
	assert(pointer);

	struct wlf_zwp_pointer_gesture_hold_v1 *hold =
		calloc(1, sizeof(*hold));
	if (!hold) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_pointer_gesture_hold_v1");
		return NULL;
	}

	hold->base = zwp_pointer_gestures_v1_get_hold_gesture(
		gestures->base, pointer);
	if (!hold->base) {
		wlf_log(WLF_ERROR,
			"zwp_pointer_gestures_v1_get_hold_gesture() returned NULL");
		free(hold);
		return NULL;
	}

	wlf_signal_init(&hold->events.begin);
	wlf_signal_init(&hold->events.end);
	wlf_signal_init(&hold->events.destroy);

	zwp_pointer_gesture_hold_v1_add_listener(hold->base, &hold_listener,
		hold);

	return hold;
}

void wlf_zwp_pointer_gesture_swipe_v1_destroy(
	struct wlf_zwp_pointer_gesture_swipe_v1 *swipe)
{
	if (!swipe) {
		return;
	}

	wlf_signal_emit_mutable(&swipe->events.destroy, swipe);
	zwp_pointer_gesture_swipe_v1_destroy(swipe->base);
	free(swipe);
}

void wlf_zwp_pointer_gesture_pinch_v1_destroy(
	struct wlf_zwp_pointer_gesture_pinch_v1 *pinch)
{
	if (!pinch) {
		return;
	}

	wlf_signal_emit_mutable(&pinch->events.destroy, pinch);
	zwp_pointer_gesture_pinch_v1_destroy(pinch->base);
	free(pinch);
}

void wlf_zwp_pointer_gesture_hold_v1_destroy(
	struct wlf_zwp_pointer_gesture_hold_v1 *hold)
{
	if (!hold) {
		return;
	}

	wlf_signal_emit_mutable(&hold->events.destroy, hold);
	zwp_pointer_gesture_hold_v1_destroy(hold->base);
	free(hold);
}
