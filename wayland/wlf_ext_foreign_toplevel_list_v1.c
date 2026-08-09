/**
 * @file        wlf_ext_foreign_toplevel_list_v1.c
 * @brief       Wayland ext_foreign_toplevel_list_v1 protocol wrapper for
 *              wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#include "wlf/wayland/wlf_ext_foreign_toplevel_list_v1.h"
#include "wayland/protocols/ext-foreign-toplevel-list-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Handle listeners
 * ---------------------------------------------------------------------- */

static void handle_closed(void *data,
	struct ext_foreign_toplevel_handle_v1 *base)
{
	struct wlf_ext_foreign_toplevel_handle_v1 *handle = data;
	(void)base;
	wlf_signal_emit_mutable(&handle->events.closed, handle);
}

static void handle_done(void *data,
	struct ext_foreign_toplevel_handle_v1 *base)
{
	struct wlf_ext_foreign_toplevel_handle_v1 *handle = data;
	(void)base;
	wlf_signal_emit_mutable(&handle->events.done, handle);
}

static void handle_title(void *data,
	struct ext_foreign_toplevel_handle_v1 *base, const char *title)
{
	struct wlf_ext_foreign_toplevel_handle_v1 *handle = data;
	(void)base;
	free(handle->title);
	handle->title = title ? strdup(title) : NULL;
}

static void handle_app_id(void *data,
	struct ext_foreign_toplevel_handle_v1 *base, const char *app_id)
{
	struct wlf_ext_foreign_toplevel_handle_v1 *handle = data;
	(void)base;
	free(handle->app_id);
	handle->app_id = app_id ? strdup(app_id) : NULL;
}

static void handle_identifier(void *data,
	struct ext_foreign_toplevel_handle_v1 *base, const char *identifier)
{
	struct wlf_ext_foreign_toplevel_handle_v1 *handle = data;
	(void)base;
	free(handle->identifier);
	handle->identifier = identifier ? strdup(identifier) : NULL;
}

static const struct ext_foreign_toplevel_handle_v1_listener handle_listener = {
	.closed     = handle_closed,
	.done       = handle_done,
	.title      = handle_title,
	.app_id     = handle_app_id,
	.identifier = handle_identifier,
};

/* -------------------------------------------------------------------------
 * Manager listeners
 * ---------------------------------------------------------------------- */

static void list_handle_toplevel(void *data,
	struct ext_foreign_toplevel_list_v1 *base,
	struct ext_foreign_toplevel_handle_v1 *tl_base)
{
	struct wlf_ext_foreign_toplevel_list_v1 *list = data;
	(void)base;

	struct wlf_ext_foreign_toplevel_handle_v1 *handle =
		calloc(1, sizeof(*handle));
	if (!handle) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_ext_foreign_toplevel_handle_v1");
		ext_foreign_toplevel_handle_v1_destroy(tl_base);
		return;
	}

	handle->base = tl_base;
	wlf_signal_init(&handle->events.done);
	wlf_signal_init(&handle->events.closed);
	wlf_signal_init(&handle->events.destroy);

	ext_foreign_toplevel_handle_v1_add_listener(tl_base, &handle_listener,
		handle);

	wlf_signal_emit_mutable(&list->events.toplevel, handle);
}

static void list_handle_finished(void *data,
	struct ext_foreign_toplevel_list_v1 *base)
{
	struct wlf_ext_foreign_toplevel_list_v1 *list = data;
	(void)base;
	wlf_signal_emit_mutable(&list->events.finished, list);
}

static const struct ext_foreign_toplevel_list_v1_listener list_listener = {
	.toplevel = list_handle_toplevel,
	.finished = list_handle_finished,
};

/* -------------------------------------------------------------------------
 * Manager
 * ---------------------------------------------------------------------- */

struct wlf_ext_foreign_toplevel_list_v1 *
wlf_ext_foreign_toplevel_list_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver = ext_foreign_toplevel_list_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
		wlf_log(WLF_DEBUG,
			"ext_foreign_toplevel_list_v1: binding version %u "
			"(server advertised %u, interface supports %u)",
			bind_ver, version,
			ext_foreign_toplevel_list_v1_interface.version);
	}

	struct wlf_ext_foreign_toplevel_list_v1 *list = calloc(1, sizeof(*list));
	if (!list) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_ext_foreign_toplevel_list_v1");
		return NULL;
	}

	list->base = wl_registry_bind(wl_registry, name,
		&ext_foreign_toplevel_list_v1_interface, bind_ver);
	if (!list->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for ext_foreign_toplevel_list_v1");
		free(list);
		return NULL;
	}

	wlf_signal_init(&list->events.toplevel);
	wlf_signal_init(&list->events.finished);
	wlf_signal_init(&list->events.destroy);

	ext_foreign_toplevel_list_v1_add_listener(list->base, &list_listener, list);
	return list;
}

void wlf_ext_foreign_toplevel_list_v1_stop(
	struct wlf_ext_foreign_toplevel_list_v1 *list)
{
	assert(list);
	ext_foreign_toplevel_list_v1_stop(list->base);
}

void wlf_ext_foreign_toplevel_list_v1_destroy(
	struct wlf_ext_foreign_toplevel_list_v1 *list)
{
	assert(list);
	wlf_signal_emit_mutable(&list->events.destroy, list);
	ext_foreign_toplevel_list_v1_destroy(list->base);
	free(list);
}

/* -------------------------------------------------------------------------
 * Handle
 * ---------------------------------------------------------------------- */

void wlf_ext_foreign_toplevel_handle_v1_destroy(
	struct wlf_ext_foreign_toplevel_handle_v1 *handle)
{
	assert(handle);
	wlf_signal_emit_mutable(&handle->events.destroy, handle);
	ext_foreign_toplevel_handle_v1_destroy(handle->base);
	free(handle->title);
	free(handle->app_id);
	free(handle->identifier);
	free(handle);
}
