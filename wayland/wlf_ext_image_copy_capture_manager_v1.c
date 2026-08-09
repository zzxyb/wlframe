/**
 * @file        wlf_ext_image_copy_capture_manager_v1.c
 * @brief       Wayland ext-image-copy-capture-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_ext_image_copy_capture_manager_v1.h"
#include "wlf/wayland/wlf_ext_image_capture_source_manager_v1.h"
#include "wayland/protocols/ext-image-copy-capture-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Session listeners
 * ---------------------------------------------------------------------- */

static void session_handle_buffer_size(void *data,
	struct ext_image_copy_capture_session_v1 *base,
	uint32_t width, uint32_t height)
{
	struct wlf_ext_image_copy_capture_session_v1 *session = data;
	(void)base;
	session->buffer_width  = width;
	session->buffer_height = height;
}

static void session_handle_shm_format(void *data,
	struct ext_image_copy_capture_session_v1 *base, uint32_t format)
{
	struct wlf_ext_image_copy_capture_session_v1 *session = data;
	(void)base;
	session->shm_format = format;
}

static void session_handle_dmabuf_device(void *data,
	struct ext_image_copy_capture_session_v1 *base,
	struct wl_array *device)
{
	(void)data;
	(void)base;
	(void)device;
}

static void session_handle_dmabuf_format(void *data,
	struct ext_image_copy_capture_session_v1 *base,
	uint32_t format, struct wl_array *modifiers)
{
	(void)data;
	(void)base;
	(void)format;
	(void)modifiers;
}

static void session_handle_done(void *data,
	struct ext_image_copy_capture_session_v1 *base)
{
	struct wlf_ext_image_copy_capture_session_v1 *session = data;
	(void)base;
	wlf_signal_emit_mutable(&session->events.done, session);
}

static void session_handle_stopped(void *data,
	struct ext_image_copy_capture_session_v1 *base)
{
	struct wlf_ext_image_copy_capture_session_v1 *session = data;
	(void)base;
	wlf_signal_emit_mutable(&session->events.stopped, session);
}

static const struct ext_image_copy_capture_session_v1_listener session_listener = {
	.buffer_size   = session_handle_buffer_size,
	.shm_format    = session_handle_shm_format,
	.dmabuf_device = session_handle_dmabuf_device,
	.dmabuf_format = session_handle_dmabuf_format,
	.done          = session_handle_done,
	.stopped       = session_handle_stopped,
};

/* -------------------------------------------------------------------------
 * Frame listeners
 * ---------------------------------------------------------------------- */

static void frame_handle_transform(void *data,
	struct ext_image_copy_capture_frame_v1 *base, uint32_t transform)
{
	struct wlf_ext_image_copy_capture_frame_v1 *frame = data;
	(void)base;
	frame->transform = transform;
}

static void frame_handle_damage(void *data,
	struct ext_image_copy_capture_frame_v1 *base,
	int32_t x, int32_t y, int32_t width, int32_t height)
{
	(void)data;
	(void)base;
	(void)x;
	(void)y;
	(void)width;
	(void)height;
}

static void frame_handle_presentation_time(void *data,
	struct ext_image_copy_capture_frame_v1 *base,
	uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec)
{
	struct wlf_ext_image_copy_capture_frame_v1 *frame = data;
	(void)base;
	frame->tv_sec_hi = tv_sec_hi;
	frame->tv_sec_lo = tv_sec_lo;
	frame->tv_nsec   = tv_nsec;
}

static void frame_handle_ready(void *data,
	struct ext_image_copy_capture_frame_v1 *base)
{
	struct wlf_ext_image_copy_capture_frame_v1 *frame = data;
	(void)base;
	wlf_signal_emit_mutable(&frame->events.ready, frame);
}

static void frame_handle_failed(void *data,
	struct ext_image_copy_capture_frame_v1 *base, uint32_t reason)
{
	struct wlf_ext_image_copy_capture_frame_v1 *frame = data;
	(void)base;
	frame->failed_reason = reason;
	wlf_signal_emit_mutable(&frame->events.failed, frame);
}

static const struct ext_image_copy_capture_frame_v1_listener frame_listener = {
	.transform         = frame_handle_transform,
	.damage            = frame_handle_damage,
	.presentation_time = frame_handle_presentation_time,
	.ready             = frame_handle_ready,
	.failed            = frame_handle_failed,
};

/* -------------------------------------------------------------------------
 * Cursor session listeners
 * ---------------------------------------------------------------------- */

static void cursor_session_handle_enter(void *data,
	struct ext_image_copy_capture_cursor_session_v1 *base)
{
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cs = data;
	(void)base;
	cs->entered = true;
	wlf_signal_emit_mutable(&cs->events.enter, cs);
}

static void cursor_session_handle_leave(void *data,
	struct ext_image_copy_capture_cursor_session_v1 *base)
{
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cs = data;
	(void)base;
	cs->entered = false;
	wlf_signal_emit_mutable(&cs->events.leave, cs);
}

static void cursor_session_handle_position(void *data,
	struct ext_image_copy_capture_cursor_session_v1 *base,
	int32_t x, int32_t y)
{
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cs = data;
	(void)base;
	cs->x = x;
	cs->y = y;
	wlf_signal_emit_mutable(&cs->events.position, cs);
}

static void cursor_session_handle_hotspot(void *data,
	struct ext_image_copy_capture_cursor_session_v1 *base,
	int32_t x, int32_t y)
{
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cs = data;
	(void)base;
	cs->hotspot_x = x;
	cs->hotspot_y = y;
	wlf_signal_emit_mutable(&cs->events.hotspot, cs);
}

static const struct ext_image_copy_capture_cursor_session_v1_listener
	cursor_session_listener = {
	.enter    = cursor_session_handle_enter,
	.leave    = cursor_session_handle_leave,
	.position = cursor_session_handle_position,
	.hotspot  = cursor_session_handle_hotspot,
};

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

static struct wlf_ext_image_copy_capture_session_v1 *
session_wrap(struct ext_image_copy_capture_session_v1 *base)
{
	if (!base) {
		return NULL;
	}

	struct wlf_ext_image_copy_capture_session_v1 *session =
		calloc(1, sizeof(*session));
	if (!session) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_ext_image_copy_capture_session_v1");
		ext_image_copy_capture_session_v1_destroy(base);
		return NULL;
	}

	session->base = base;

	wlf_signal_init(&session->events.done);
	wlf_signal_init(&session->events.stopped);
	wlf_signal_init(&session->events.destroy);

	ext_image_copy_capture_session_v1_add_listener(session->base,
		&session_listener, session);

	return session;
}

/* -------------------------------------------------------------------------
 * Public API — manager
 * ---------------------------------------------------------------------- */

struct wlf_ext_image_copy_capture_manager_v1 *
wlf_ext_image_copy_capture_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver =
		(uint32_t)ext_image_copy_capture_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_ext_image_copy_capture_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_ext_image_copy_capture_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(registry, name,
		&ext_image_copy_capture_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"ext_image_copy_capture_manager_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound ext_image_copy_capture_manager_v1 "
		"(name: %u, version: %u)", name, bind_ver);

	return manager;
}

void wlf_ext_image_copy_capture_manager_v1_destroy(
	struct wlf_ext_image_copy_capture_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	ext_image_copy_capture_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_ext_image_copy_capture_session_v1 *
wlf_ext_image_copy_capture_manager_v1_create_session(
	struct wlf_ext_image_copy_capture_manager_v1 *manager,
	struct wlf_ext_image_capture_source_v1 *source,
	enum wlf_ext_image_copy_capture_options_v1 options)
{
	assert(manager);
	assert(manager->base);
	assert(source);
	assert(source->base);

	struct ext_image_copy_capture_session_v1 *base =
		ext_image_copy_capture_manager_v1_create_session(
			manager->base, source->base, (uint32_t)options);

	struct wlf_ext_image_copy_capture_session_v1 *session =
		session_wrap(base);
	if (!session) {
		wlf_log(WLF_ERROR,
			"wlf_ext_image_copy_capture_manager_v1_create_session() "
			"failed");
	}
	return session;
}

struct wlf_ext_image_copy_capture_cursor_session_v1 *
wlf_ext_image_copy_capture_manager_v1_create_pointer_cursor_session(
	struct wlf_ext_image_copy_capture_manager_v1 *manager,
	struct wlf_ext_image_capture_source_v1 *source,
	struct wl_pointer *pointer)
{
	assert(manager);
	assert(manager->base);
	assert(source);
	assert(source->base);
	assert(pointer);

	struct ext_image_copy_capture_cursor_session_v1 *base =
		ext_image_copy_capture_manager_v1_create_pointer_cursor_session(
			manager->base, source->base, pointer);
	if (!base) {
		wlf_log(WLF_ERROR,
			"ext_image_copy_capture_manager_v1_create_pointer_cursor_session()"
			" returned NULL");
		return NULL;
	}

	struct wlf_ext_image_copy_capture_cursor_session_v1 *cs =
		calloc(1, sizeof(*cs));
	if (!cs) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_ext_image_copy_capture_cursor_session_v1");
		ext_image_copy_capture_cursor_session_v1_destroy(base);
		return NULL;
	}

	cs->base = base;

	wlf_signal_init(&cs->events.enter);
	wlf_signal_init(&cs->events.leave);
	wlf_signal_init(&cs->events.position);
	wlf_signal_init(&cs->events.hotspot);
	wlf_signal_init(&cs->events.destroy);

	ext_image_copy_capture_cursor_session_v1_add_listener(cs->base,
		&cursor_session_listener, cs);

	return cs;
}

/* -------------------------------------------------------------------------
 * Public API — session
 * ---------------------------------------------------------------------- */

struct wlf_ext_image_copy_capture_frame_v1 *
wlf_ext_image_copy_capture_session_v1_create_frame(
	struct wlf_ext_image_copy_capture_session_v1 *session)
{
	assert(session);
	assert(session->base);

	struct ext_image_copy_capture_frame_v1 *base =
		ext_image_copy_capture_session_v1_create_frame(session->base);
	if (!base) {
		wlf_log(WLF_ERROR,
			"ext_image_copy_capture_session_v1_create_frame() "
			"returned NULL");
		return NULL;
	}

	struct wlf_ext_image_copy_capture_frame_v1 *frame =
		calloc(1, sizeof(*frame));
	if (!frame) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_ext_image_copy_capture_frame_v1");
		ext_image_copy_capture_frame_v1_destroy(base);
		return NULL;
	}

	frame->base = base;

	wlf_signal_init(&frame->events.ready);
	wlf_signal_init(&frame->events.failed);
	wlf_signal_init(&frame->events.destroy);

	ext_image_copy_capture_frame_v1_add_listener(frame->base,
		&frame_listener, frame);

	return frame;
}

void wlf_ext_image_copy_capture_session_v1_destroy(
	struct wlf_ext_image_copy_capture_session_v1 *session)
{
	if (!session) {
		return;
	}

	wlf_signal_emit_mutable(&session->events.destroy, session);
	ext_image_copy_capture_session_v1_destroy(session->base);
	free(session);
}

/* -------------------------------------------------------------------------
 * Public API — frame
 * ---------------------------------------------------------------------- */

void wlf_ext_image_copy_capture_frame_v1_attach_buffer(
	struct wlf_ext_image_copy_capture_frame_v1 *frame,
	struct wl_buffer *buffer)
{
	assert(frame);
	assert(frame->base);
	assert(buffer);
	ext_image_copy_capture_frame_v1_attach_buffer(frame->base, buffer);
}

void wlf_ext_image_copy_capture_frame_v1_damage_buffer(
	struct wlf_ext_image_copy_capture_frame_v1 *frame,
	int32_t x, int32_t y, int32_t width, int32_t height)
{
	assert(frame);
	assert(frame->base);
	ext_image_copy_capture_frame_v1_damage_buffer(frame->base,
		x, y, width, height);
}

void wlf_ext_image_copy_capture_frame_v1_capture(
	struct wlf_ext_image_copy_capture_frame_v1 *frame)
{
	assert(frame);
	assert(frame->base);
	ext_image_copy_capture_frame_v1_capture(frame->base);
}

void wlf_ext_image_copy_capture_frame_v1_destroy(
	struct wlf_ext_image_copy_capture_frame_v1 *frame)
{
	if (!frame) {
		return;
	}

	wlf_signal_emit_mutable(&frame->events.destroy, frame);
	ext_image_copy_capture_frame_v1_destroy(frame->base);
	free(frame);
}

/* -------------------------------------------------------------------------
 * Public API — cursor session
 * ---------------------------------------------------------------------- */

struct wlf_ext_image_copy_capture_session_v1 *
wlf_ext_image_copy_capture_cursor_session_v1_get_capture_session(
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cursor_session)
{
	assert(cursor_session);
	assert(cursor_session->base);

	struct ext_image_copy_capture_session_v1 *base =
		ext_image_copy_capture_cursor_session_v1_get_capture_session(
			cursor_session->base);

	struct wlf_ext_image_copy_capture_session_v1 *session =
		session_wrap(base);
	if (!session) {
		wlf_log(WLF_ERROR,
			"wlf_ext_image_copy_capture_cursor_session_v1_get_capture_session()"
			" failed");
	}
	return session;
}

void wlf_ext_image_copy_capture_cursor_session_v1_destroy(
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cursor_session)
{
	if (!cursor_session) {
		return;
	}

	wlf_signal_emit_mutable(&cursor_session->events.destroy, cursor_session);
	ext_image_copy_capture_cursor_session_v1_destroy(cursor_session->base);
	free(cursor_session);
}
