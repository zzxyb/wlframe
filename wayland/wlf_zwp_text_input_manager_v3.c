/**
 * @file        wlf_zwp_text_input_manager_v3.c
 * @brief       Wayland text-input-unstable-v3 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zwp_text_input_manager_v3.h"
#include "wayland/protocols/text-input-unstable-v3-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Text-input-v3 listeners
 * ---------------------------------------------------------------------- */

static void text_input_handle_enter(void *data,
	struct zwp_text_input_v3 *base, struct wl_surface *surface)
{
	struct wlf_zwp_text_input_v3 *ti = data;
	(void)base;
	ti->surface = surface;
	wlf_signal_emit_mutable(&ti->events.enter, ti);
}

static void text_input_handle_leave(void *data,
	struct zwp_text_input_v3 *base, struct wl_surface *surface)
{
	struct wlf_zwp_text_input_v3 *ti = data;
	(void)base;
	ti->surface = surface;
	wlf_signal_emit_mutable(&ti->events.leave, ti);
}

static void text_input_handle_preedit_string(void *data,
	struct zwp_text_input_v3 *base, const char *text,
	int32_t cursor_begin, int32_t cursor_end)
{
	struct wlf_zwp_text_input_v3 *ti = data;
	(void)base;
	free(ti->pending.preedit_text);
	ti->pending.preedit_text = text ? strdup(text) : NULL;
	ti->pending.preedit_cursor_begin = cursor_begin;
	ti->pending.preedit_cursor_end   = cursor_end;
	wlf_signal_emit_mutable(&ti->events.preedit_string, ti);
}

static void text_input_handle_commit_string(void *data,
	struct zwp_text_input_v3 *base, const char *text)
{
	struct wlf_zwp_text_input_v3 *ti = data;
	(void)base;
	free(ti->pending.commit_text);
	ti->pending.commit_text = text ? strdup(text) : NULL;
	wlf_signal_emit_mutable(&ti->events.commit_string, ti);
}

static void text_input_handle_delete_surrounding_text(void *data,
	struct zwp_text_input_v3 *base,
	uint32_t before_length, uint32_t after_length)
{
	struct wlf_zwp_text_input_v3 *ti = data;
	(void)base;
	ti->pending.delete_before_length = before_length;
	ti->pending.delete_after_length  = after_length;
	wlf_signal_emit_mutable(&ti->events.delete_surrounding_text, ti);
}

static void text_input_handle_done(void *data,
	struct zwp_text_input_v3 *base, uint32_t serial)
{
	struct wlf_zwp_text_input_v3 *ti = data;
	(void)base;

	free(ti->preedit_text);
	free(ti->commit_text);

	ti->preedit_text         = ti->pending.preedit_text;
	ti->preedit_cursor_begin = ti->pending.preedit_cursor_begin;
	ti->preedit_cursor_end   = ti->pending.preedit_cursor_end;
	ti->commit_text          = ti->pending.commit_text;
	ti->delete_before_length = ti->pending.delete_before_length;
	ti->delete_after_length  = ti->pending.delete_after_length;
	ti->done_serial          = serial;

	ti->pending.preedit_text  = NULL;
	ti->pending.commit_text   = NULL;
	ti->pending.delete_before_length = 0;
	ti->pending.delete_after_length  = 0;
	ti->pending.preedit_cursor_begin = 0;
	ti->pending.preedit_cursor_end   = 0;

	wlf_signal_emit_mutable(&ti->events.done, ti);
}

static const struct zwp_text_input_v3_listener text_input_listener = {
	.enter                   = text_input_handle_enter,
	.leave                   = text_input_handle_leave,
	.preedit_string          = text_input_handle_preedit_string,
	.commit_string           = text_input_handle_commit_string,
	.delete_surrounding_text = text_input_handle_delete_surrounding_text,
	.done                    = text_input_handle_done,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_zwp_text_input_manager_v3 *
wlf_zwp_text_input_manager_v3_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver =
		(uint32_t)zwp_text_input_manager_v3_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwp_text_input_manager_v3 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_text_input_manager_v3");
		return NULL;
	}

	manager->base = wl_registry_bind(registry, name,
		&zwp_text_input_manager_v3_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zwp_text_input_manager_v3 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zwp_text_input_manager_v3 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_zwp_text_input_manager_v3_destroy(
	struct wlf_zwp_text_input_manager_v3 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	zwp_text_input_manager_v3_destroy(manager->base);
	free(manager);
}

struct wlf_zwp_text_input_v3 *
wlf_zwp_text_input_manager_v3_get_text_input(
	struct wlf_zwp_text_input_manager_v3 *manager, struct wl_seat *seat)
{
	assert(manager);
	assert(manager->base);
	assert(seat);

	struct wlf_zwp_text_input_v3 *ti = calloc(1, sizeof(*ti));
	if (!ti) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_text_input_v3");
		return NULL;
	}

	ti->base = zwp_text_input_manager_v3_get_text_input(manager->base, seat);
	if (!ti->base) {
		wlf_log(WLF_ERROR,
			"zwp_text_input_manager_v3_get_text_input() returned NULL");
		free(ti);
		return NULL;
	}

	wlf_signal_init(&ti->events.enter);
	wlf_signal_init(&ti->events.leave);
	wlf_signal_init(&ti->events.preedit_string);
	wlf_signal_init(&ti->events.commit_string);
	wlf_signal_init(&ti->events.delete_surrounding_text);
	wlf_signal_init(&ti->events.done);
	wlf_signal_init(&ti->events.destroy);

	zwp_text_input_v3_add_listener(ti->base, &text_input_listener, ti);

	return ti;
}

void wlf_zwp_text_input_v3_enable(struct wlf_zwp_text_input_v3 *ti)
{
	assert(ti);
	assert(ti->base);
	zwp_text_input_v3_enable(ti->base);
}

void wlf_zwp_text_input_v3_disable(struct wlf_zwp_text_input_v3 *ti)
{
	assert(ti);
	assert(ti->base);
	zwp_text_input_v3_disable(ti->base);
}

void wlf_zwp_text_input_v3_set_surrounding_text(
	struct wlf_zwp_text_input_v3 *ti, const char *text,
	int32_t cursor, int32_t anchor)
{
	assert(ti);
	assert(ti->base);
	zwp_text_input_v3_set_surrounding_text(ti->base, text, cursor, anchor);
}

void wlf_zwp_text_input_v3_set_text_change_cause(
	struct wlf_zwp_text_input_v3 *ti, uint32_t cause)
{
	assert(ti);
	assert(ti->base);
	zwp_text_input_v3_set_text_change_cause(ti->base, cause);
}

void wlf_zwp_text_input_v3_set_content_type(
	struct wlf_zwp_text_input_v3 *ti, uint32_t hint, uint32_t purpose)
{
	assert(ti);
	assert(ti->base);
	zwp_text_input_v3_set_content_type(ti->base, hint, purpose);
}

void wlf_zwp_text_input_v3_set_cursor_rectangle(
	struct wlf_zwp_text_input_v3 *ti,
	int32_t x, int32_t y, int32_t width, int32_t height)
{
	assert(ti);
	assert(ti->base);
	zwp_text_input_v3_set_cursor_rectangle(ti->base, x, y, width, height);
}

void wlf_zwp_text_input_v3_commit(struct wlf_zwp_text_input_v3 *ti)
{
	assert(ti);
	assert(ti->base);
	zwp_text_input_v3_commit(ti->base);
}

void wlf_zwp_text_input_v3_destroy(struct wlf_zwp_text_input_v3 *ti)
{
	if (!ti) {
		return;
	}

	wlf_signal_emit_mutable(&ti->events.destroy, ti);
	free(ti->preedit_text);
	free(ti->commit_text);
	free(ti->pending.preedit_text);
	free(ti->pending.commit_text);
	zwp_text_input_v3_destroy(ti->base);
	free(ti);
}
