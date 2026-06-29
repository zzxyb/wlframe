/**
 * @file        wlf_zwp_tablet_manager_v2.c
 * @brief       Wayland zwp_tablet_manager_v2 protocol wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zwp_tablet_manager_v2.h"
#include "wayland/protocols/tablet-v2-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-util.h>

/* -------------------------------------------------------------------------
 * Tablet (zwp_tablet_v2) listeners
 * ---------------------------------------------------------------------- */

static void tablet_handle_name(void *data,
	struct zwp_tablet_v2 *base, const char *name)
{
	struct wlf_zwp_tablet_v2 *tablet = data;
	(void)base;
	free(tablet->name);
	tablet->name = name ? strdup(name) : NULL;
}

static void tablet_handle_id(void *data,
	struct zwp_tablet_v2 *base, uint32_t vid, uint32_t pid)
{
	struct wlf_zwp_tablet_v2 *tablet = data;
	(void)base;
	tablet->vid = vid;
	tablet->pid = pid;
}

static void tablet_handle_path(void *data,
	struct zwp_tablet_v2 *base, const char *path)
{
	struct wlf_zwp_tablet_v2 *tablet = data;
	(void)base;

	char **new_paths = realloc(tablet->paths,
		(tablet->n_paths + 2) * sizeof(char *));
	if (!new_paths) {
		wlf_log_errno(WLF_ERROR, "failed to grow tablet->paths");
		return;
	}
	tablet->paths = new_paths;
	tablet->paths[tablet->n_paths] = path ? strdup(path) : NULL;
	tablet->n_paths++;
	tablet->paths[tablet->n_paths] = NULL;
}

static void tablet_handle_done(void *data, struct zwp_tablet_v2 *base)
{
	struct wlf_zwp_tablet_v2 *tablet = data;
	(void)base;
	wlf_signal_emit_mutable(&tablet->events.done, tablet);
}

static void tablet_handle_removed(void *data, struct zwp_tablet_v2 *base)
{
	struct wlf_zwp_tablet_v2 *tablet = data;
	(void)base;
	wlf_signal_emit_mutable(&tablet->events.removed, tablet);
}

static void tablet_handle_bustype(void *data,
	struct zwp_tablet_v2 *base, uint32_t bustype)
{
	struct wlf_zwp_tablet_v2 *tablet = data;
	(void)base;
	tablet->bustype = (enum wlf_tablet_bustype)bustype;
}

static const struct zwp_tablet_v2_listener tablet_listener = {
	.name    = tablet_handle_name,
	.id      = tablet_handle_id,
	.path    = tablet_handle_path,
	.done    = tablet_handle_done,
	.removed = tablet_handle_removed,
	.bustype = tablet_handle_bustype,
};

/* -------------------------------------------------------------------------
 * Tool (zwp_tablet_tool_v2) listeners
 * ---------------------------------------------------------------------- */

static void tool_handle_type(void *data,
	struct zwp_tablet_tool_v2 *base, uint32_t tool_type)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->type = (enum wlf_tablet_tool_type)tool_type;
}

static void tool_handle_hardware_serial(void *data,
	struct zwp_tablet_tool_v2 *base,
	uint32_t hardware_serial_hi, uint32_t hardware_serial_lo)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->hardware_serial_hi = hardware_serial_hi;
	tool->hardware_serial_lo = hardware_serial_lo;
}

static void tool_handle_hardware_id_wacom(void *data,
	struct zwp_tablet_tool_v2 *base,
	uint32_t hardware_id_hi, uint32_t hardware_id_lo)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->hardware_id_hi = hardware_id_hi;
	tool->hardware_id_lo = hardware_id_lo;
}

static void tool_handle_capability(void *data,
	struct zwp_tablet_tool_v2 *base, uint32_t capability)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->capability_flags |= (1u << capability);
}

static void tool_handle_done(void *data, struct zwp_tablet_tool_v2 *base)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	wlf_signal_emit_mutable(&tool->events.done, tool);
}

static void tool_handle_removed(void *data, struct zwp_tablet_tool_v2 *base)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	wlf_signal_emit_mutable(&tool->events.removed, tool);
}

static void tool_handle_proximity_in(void *data,
	struct zwp_tablet_tool_v2 *base,
	uint32_t serial, struct zwp_tablet_v2 *tablet_base,
	struct wl_surface *surface)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.proximity_in = 1;
	tool->current.surface = surface;
	tool->current.tablet =
		(struct wlf_zwp_tablet_v2 *)zwp_tablet_v2_get_user_data(tablet_base);
	(void)serial;
	wlf_signal_emit_mutable(&tool->events.proximity_in, tool);
}

static void tool_handle_proximity_out(void *data, struct zwp_tablet_tool_v2 *base)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.proximity_in = 0;
	tool->current.surface = NULL;
	tool->current.tablet = NULL;
	wlf_signal_emit_mutable(&tool->events.proximity_out, tool);
}

static void tool_handle_down(void *data,
	struct zwp_tablet_tool_v2 *base, uint32_t serial)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	(void)serial;
	tool->current.down = 1;
	wlf_signal_emit_mutable(&tool->events.down, tool);
}

static void tool_handle_up(void *data, struct zwp_tablet_tool_v2 *base)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.down = 0;
	wlf_signal_emit_mutable(&tool->events.up, tool);
}

static void tool_handle_motion(void *data,
	struct zwp_tablet_tool_v2 *base,
	wl_fixed_t x, wl_fixed_t y)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.x = wl_fixed_to_double(x);
	tool->current.y = wl_fixed_to_double(y);
}

static void tool_handle_pressure(void *data,
	struct zwp_tablet_tool_v2 *base, uint32_t pressure)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.pressure = pressure;
}

static void tool_handle_distance(void *data,
	struct zwp_tablet_tool_v2 *base, uint32_t distance)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.distance = distance;
}

static void tool_handle_tilt(void *data,
	struct zwp_tablet_tool_v2 *base,
	wl_fixed_t tilt_x, wl_fixed_t tilt_y)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.tilt_x = wl_fixed_to_double(tilt_x);
	tool->current.tilt_y = wl_fixed_to_double(tilt_y);
}

static void tool_handle_rotation(void *data,
	struct zwp_tablet_tool_v2 *base, wl_fixed_t degrees)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.rotation = wl_fixed_to_double(degrees);
}

static void tool_handle_slider(void *data,
	struct zwp_tablet_tool_v2 *base, int32_t position)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.slider = position;
}

static void tool_handle_wheel(void *data,
	struct zwp_tablet_tool_v2 *base,
	wl_fixed_t degrees, int32_t clicks)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.wheel_degrees = wl_fixed_to_double(degrees);
	tool->current.wheel_clicks = clicks;
}

static void tool_handle_button(void *data,
	struct zwp_tablet_tool_v2 *base,
	uint32_t serial, uint32_t button, uint32_t state)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	tool->current.button_serial = serial;
	tool->current.button = button;
	tool->current.button_state = (enum wlf_tablet_button_state)state;
	wlf_signal_emit_mutable(&tool->events.button, tool);
}

static void tool_handle_frame(void *data,
	struct zwp_tablet_tool_v2 *base, uint32_t time)
{
	struct wlf_zwp_tablet_tool_v2 *tool = data;
	(void)base;
	(void)time;
	wlf_signal_emit_mutable(&tool->events.frame, tool);
}

static const struct zwp_tablet_tool_v2_listener tool_listener = {
	.type              = tool_handle_type,
	.hardware_serial   = tool_handle_hardware_serial,
	.hardware_id_wacom = tool_handle_hardware_id_wacom,
	.capability        = tool_handle_capability,
	.done              = tool_handle_done,
	.removed           = tool_handle_removed,
	.proximity_in      = tool_handle_proximity_in,
	.proximity_out     = tool_handle_proximity_out,
	.down              = tool_handle_down,
	.up                = tool_handle_up,
	.motion            = tool_handle_motion,
	.pressure          = tool_handle_pressure,
	.distance          = tool_handle_distance,
	.tilt              = tool_handle_tilt,
	.rotation          = tool_handle_rotation,
	.slider            = tool_handle_slider,
	.wheel             = tool_handle_wheel,
	.button            = tool_handle_button,
	.frame             = tool_handle_frame,
};

/* -------------------------------------------------------------------------
 * Pad (zwp_tablet_pad_v2) listeners
 * ---------------------------------------------------------------------- */

static void pad_handle_group(void *data,
	struct zwp_tablet_pad_v2 *base,
	struct zwp_tablet_pad_group_v2 *pad_group)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;

	struct zwp_tablet_pad_group_v2 **new_groups = realloc(pad->groups,
		(pad->n_groups + 2) * sizeof(*pad->groups));
	if (!new_groups) {
		wlf_log_errno(WLF_ERROR, "failed to grow pad->groups");
		return;
	}
	pad->groups = new_groups;
	pad->groups[pad->n_groups] = pad_group;
	pad->n_groups++;
	pad->groups[pad->n_groups] = NULL;
}

static void pad_handle_path(void *data,
	struct zwp_tablet_pad_v2 *base, const char *path)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;

	char **new_paths = realloc(pad->paths,
		(pad->n_paths + 2) * sizeof(char *));
	if (!new_paths) {
		wlf_log_errno(WLF_ERROR, "failed to grow pad->paths");
		return;
	}
	pad->paths = new_paths;
	pad->paths[pad->n_paths] = path ? strdup(path) : NULL;
	pad->n_paths++;
	pad->paths[pad->n_paths] = NULL;
}

static void pad_handle_buttons(void *data,
	struct zwp_tablet_pad_v2 *base, uint32_t buttons)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;
	pad->n_buttons = buttons;
}

static void pad_handle_done(void *data, struct zwp_tablet_pad_v2 *base)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;
	wlf_signal_emit_mutable(&pad->events.done, pad);
}

static void pad_handle_button(void *data,
	struct zwp_tablet_pad_v2 *base,
	uint32_t time, uint32_t button, uint32_t state)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;
	pad->button_time = time;
	pad->button = button;
	pad->button_state = (enum wlf_tablet_button_state)state;
	wlf_signal_emit_mutable(&pad->events.button, pad);
}

static void pad_handle_enter(void *data,
	struct zwp_tablet_pad_v2 *base,
	uint32_t serial, struct zwp_tablet_v2 *tablet,
	struct wl_surface *surface)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;
	(void)serial;
	(void)tablet;
	(void)surface;
	wlf_signal_emit_mutable(&pad->events.enter, pad);
}

static void pad_handle_leave(void *data,
	struct zwp_tablet_pad_v2 *base,
	uint32_t serial, struct wl_surface *surface)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;
	(void)serial;
	(void)surface;
	wlf_signal_emit_mutable(&pad->events.leave, pad);
}

static void pad_handle_removed(void *data, struct zwp_tablet_pad_v2 *base)
{
	struct wlf_zwp_tablet_pad_v2 *pad = data;
	(void)base;
	wlf_signal_emit_mutable(&pad->events.removed, pad);
}

static const struct zwp_tablet_pad_v2_listener pad_listener = {
	.group   = pad_handle_group,
	.path    = pad_handle_path,
	.buttons = pad_handle_buttons,
	.done    = pad_handle_done,
	.button  = pad_handle_button,
	.enter   = pad_handle_enter,
	.leave   = pad_handle_leave,
	.removed = pad_handle_removed,
};

/* -------------------------------------------------------------------------
 * Seat (zwp_tablet_seat_v2) listeners
 * ---------------------------------------------------------------------- */

static void seat_handle_tablet_added(void *data,
	struct zwp_tablet_seat_v2 *base, struct zwp_tablet_v2 *tablet_base)
{
	struct wlf_zwp_tablet_seat_v2 *seat = data;
	(void)base;

	struct wlf_zwp_tablet_v2 *tablet = calloc(1, sizeof(*tablet));
	if (!tablet) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_tablet_v2");
		zwp_tablet_v2_destroy(tablet_base);
		return;
	}

	tablet->base = tablet_base;
	wlf_signal_init(&tablet->events.done);
	wlf_signal_init(&tablet->events.removed);
	wlf_signal_init(&tablet->events.destroy);

	zwp_tablet_v2_set_user_data(tablet_base, tablet);
	zwp_tablet_v2_add_listener(tablet_base, &tablet_listener, tablet);

	wlf_signal_emit_mutable(&seat->events.tablet_added, tablet);
}

static void seat_handle_tool_added(void *data,
	struct zwp_tablet_seat_v2 *base, struct zwp_tablet_tool_v2 *tool_base)
{
	struct wlf_zwp_tablet_seat_v2 *seat = data;
	(void)base;

	struct wlf_zwp_tablet_tool_v2 *tool = calloc(1, sizeof(*tool));
	if (!tool) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_tablet_tool_v2");
		zwp_tablet_tool_v2_destroy(tool_base);
		return;
	}

	tool->base = tool_base;
	wlf_signal_init(&tool->events.done);
	wlf_signal_init(&tool->events.removed);
	wlf_signal_init(&tool->events.proximity_in);
	wlf_signal_init(&tool->events.proximity_out);
	wlf_signal_init(&tool->events.down);
	wlf_signal_init(&tool->events.up);
	wlf_signal_init(&tool->events.frame);
	wlf_signal_init(&tool->events.button);
	wlf_signal_init(&tool->events.destroy);

	zwp_tablet_tool_v2_add_listener(tool_base, &tool_listener, tool);

	wlf_signal_emit_mutable(&seat->events.tool_added, tool);
}

static void seat_handle_pad_added(void *data,
	struct zwp_tablet_seat_v2 *base, struct zwp_tablet_pad_v2 *pad_base)
{
	struct wlf_zwp_tablet_seat_v2 *seat = data;
	(void)base;

	struct wlf_zwp_tablet_pad_v2 *pad = calloc(1, sizeof(*pad));
	if (!pad) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_tablet_pad_v2");
		zwp_tablet_pad_v2_destroy(pad_base);
		return;
	}

	pad->base = pad_base;
	wlf_signal_init(&pad->events.done);
	wlf_signal_init(&pad->events.removed);
	wlf_signal_init(&pad->events.button);
	wlf_signal_init(&pad->events.enter);
	wlf_signal_init(&pad->events.leave);
	wlf_signal_init(&pad->events.destroy);

	zwp_tablet_pad_v2_add_listener(pad_base, &pad_listener, pad);

	wlf_signal_emit_mutable(&seat->events.pad_added, pad);
}

static const struct zwp_tablet_seat_v2_listener seat_listener = {
	.tablet_added = seat_handle_tablet_added,
	.tool_added   = seat_handle_tool_added,
	.pad_added    = seat_handle_pad_added,
};

/* -------------------------------------------------------------------------
 * Manager public API
 * ---------------------------------------------------------------------- */

struct wlf_zwp_tablet_manager_v2 *wlf_zwp_tablet_manager_v2_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver = (uint32_t)zwp_tablet_manager_v2_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwp_tablet_manager_v2 *manager = calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_tablet_manager_v2");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&zwp_tablet_manager_v2_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for zwp_tablet_manager_v2 (name: %u)",
			name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zwp_tablet_manager_v2 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_zwp_tablet_manager_v2_destroy(
	struct wlf_zwp_tablet_manager_v2 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	zwp_tablet_manager_v2_destroy(manager->base);
	free(manager);
}

struct wlf_zwp_tablet_seat_v2 *wlf_zwp_tablet_manager_v2_get_tablet_seat(
	struct wlf_zwp_tablet_manager_v2 *manager, struct wl_seat *seat)
{
	assert(manager);
	assert(manager->base);
	assert(seat);

	struct wlf_zwp_tablet_seat_v2 *tseat = calloc(1, sizeof(*tseat));
	if (!tseat) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_tablet_seat_v2");
		return NULL;
	}

	tseat->base = zwp_tablet_manager_v2_get_tablet_seat(manager->base, seat);
	if (!tseat->base) {
		wlf_log(WLF_ERROR,
			"zwp_tablet_manager_v2_get_tablet_seat() returned NULL");
		free(tseat);
		return NULL;
	}

	wlf_signal_init(&tseat->events.tablet_added);
	wlf_signal_init(&tseat->events.tool_added);
	wlf_signal_init(&tseat->events.pad_added);
	wlf_signal_init(&tseat->events.destroy);

	zwp_tablet_seat_v2_add_listener(tseat->base, &seat_listener, tseat);
	return tseat;
}

/* -------------------------------------------------------------------------
 * Seat public API
 * ---------------------------------------------------------------------- */

void wlf_zwp_tablet_seat_v2_destroy(struct wlf_zwp_tablet_seat_v2 *seat)
{
	if (!seat) {
		return;
	}

	wlf_signal_emit_mutable(&seat->events.destroy, seat);
	zwp_tablet_seat_v2_destroy(seat->base);
	free(seat);
}

/* -------------------------------------------------------------------------
 * Tablet public API
 * ---------------------------------------------------------------------- */

void wlf_zwp_tablet_v2_destroy(struct wlf_zwp_tablet_v2 *tablet)
{
	if (!tablet) {
		return;
	}

	wlf_signal_emit_mutable(&tablet->events.destroy, tablet);
	zwp_tablet_v2_destroy(tablet->base);
	free(tablet->name);
	for (size_t i = 0; i < tablet->n_paths; i++) {
		free(tablet->paths[i]);
	}
	free(tablet->paths);
	free(tablet);
}

/* -------------------------------------------------------------------------
 * Tool public API
 * ---------------------------------------------------------------------- */

void wlf_zwp_tablet_tool_v2_set_cursor(
	struct wlf_zwp_tablet_tool_v2 *tool,
	uint32_t serial, struct wl_surface *surface,
	int32_t hotspot_x, int32_t hotspot_y)
{
	assert(tool);
	assert(tool->base);
	zwp_tablet_tool_v2_set_cursor(tool->base, serial, surface,
		hotspot_x, hotspot_y);
}

void wlf_zwp_tablet_tool_v2_destroy(struct wlf_zwp_tablet_tool_v2 *tool)
{
	if (!tool) {
		return;
	}

	wlf_signal_emit_mutable(&tool->events.destroy, tool);
	zwp_tablet_tool_v2_destroy(tool->base);
	free(tool);
}

/* -------------------------------------------------------------------------
 * Pad public API
 * ---------------------------------------------------------------------- */

void wlf_zwp_tablet_pad_v2_set_feedback(
	struct wlf_zwp_tablet_pad_v2 *pad,
	uint32_t button, const char *description, uint32_t serial)
{
	assert(pad);
	assert(pad->base);
	zwp_tablet_pad_v2_set_feedback(pad->base, button, description, serial);
}

void wlf_zwp_tablet_pad_v2_destroy(struct wlf_zwp_tablet_pad_v2 *pad)
{
	if (!pad) {
		return;
	}

	wlf_signal_emit_mutable(&pad->events.destroy, pad);
	zwp_tablet_pad_v2_destroy(pad->base);
	for (size_t i = 0; i < pad->n_paths; i++) {
		free(pad->paths[i]);
	}
	free(pad->paths);
	free(pad->groups);
	free(pad);
}
