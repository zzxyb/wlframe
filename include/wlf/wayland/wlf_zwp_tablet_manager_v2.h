/**
 * @file        wlf_zwp_tablet_manager_v2.h
 * @brief       Wayland zwp_tablet_manager_v2 protocol wrapper for wlframe.
 * @details     Implements the stable tablet-v2 protocol, which provides
 *              access to graphics tablets, styli, and drawing pads.
 *
 *              Interfaces wrapped:
 *              - wlf_zwp_tablet_manager_v2  — global manager; call
 *                get_tablet_seat() for each wl_seat.
 *              - wlf_zwp_tablet_seat_v2     — per-seat tablet state;
 *                emits signals when devices are added.
 *              - wlf_zwp_tablet_v2          — a tablet device; carries name,
 *                USB id, bus type and path list.
 *              - wlf_zwp_tablet_tool_v2     — a tool (pen, eraser, etc.);
 *                current state is stored on the struct before each signal.
 *              - wlf_zwp_tablet_pad_v2      — a pad (button/ring/strip
 *                panel); pad group sub-objects are exposed as raw
 *                zwp_tablet_pad_{group,ring,strip,dial}_v2 pointers.
 *
 *              Tool event batching:
 *                Events between two frame events form one logical frame.
 *                Listeners should respond to events.frame and read the
 *                accumulated state from the tool struct fields.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_ZWP_TABLET_MANAGER_V2_H
#define WAYLAND_WLF_ZWP_TABLET_MANAGER_V2_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_seat;
struct wl_surface;
struct zwp_tablet_manager_v2;
struct zwp_tablet_seat_v2;
struct zwp_tablet_v2;
struct zwp_tablet_tool_v2;
struct zwp_tablet_pad_v2;
struct zwp_tablet_pad_group_v2;
struct zwp_tablet_pad_ring_v2;
struct zwp_tablet_pad_strip_v2;
struct zwp_tablet_pad_dial_v2;

/* Forward declarations */
struct wlf_zwp_tablet_seat_v2;
struct wlf_zwp_tablet_v2;
struct wlf_zwp_tablet_tool_v2;
struct wlf_zwp_tablet_pad_v2;

/* -------------------------------------------------------------------------
 * Tool type and capability enums
 * ---------------------------------------------------------------------- */

/** Tool type (mirrors zwp_tablet_tool_v2.type). */
enum wlf_tablet_tool_type {
	WLF_TABLET_TOOL_TYPE_PEN       = 0x140, /**< Pen                     */
	WLF_TABLET_TOOL_TYPE_ERASER    = 0x141, /**< Eraser                  */
	WLF_TABLET_TOOL_TYPE_BRUSH     = 0x142, /**< Brush                   */
	WLF_TABLET_TOOL_TYPE_PENCIL    = 0x143, /**< Pencil                  */
	WLF_TABLET_TOOL_TYPE_AIRBRUSH  = 0x144, /**< Airbrush                */
	WLF_TABLET_TOOL_TYPE_FINGER    = 0x145, /**< Finger (touch ring etc) */
	WLF_TABLET_TOOL_TYPE_MOUSE     = 0x146, /**< Mouse                   */
	WLF_TABLET_TOOL_TYPE_LENS      = 0x147, /**< Lens cursor             */
};

/** Tool capability flags (mirrors zwp_tablet_tool_v2.capability). */
enum wlf_tablet_tool_capability {
	WLF_TABLET_TOOL_CAP_TILT     = 1, /**< Tilt axes                */
	WLF_TABLET_TOOL_CAP_PRESSURE = 2, /**< Pressure axis            */
	WLF_TABLET_TOOL_CAP_DISTANCE = 3, /**< Distance axis            */
	WLF_TABLET_TOOL_CAP_ROTATION = 4, /**< Rotation axis            */
	WLF_TABLET_TOOL_CAP_SLIDER   = 5, /**< Airbrush slider          */
	WLF_TABLET_TOOL_CAP_WHEEL    = 6, /**< Mouse wheel              */
};

/** Button state (mirrors zwp_tablet_tool_v2.button_state). */
enum wlf_tablet_button_state {
	WLF_TABLET_BUTTON_RELEASED = 0,
	WLF_TABLET_BUTTON_PRESSED  = 1,
};

/** Tablet bus type (mirrors zwp_tablet_v2.bustype, since version 2). */
enum wlf_tablet_bustype {
	WLF_TABLET_BUSTYPE_USB       = 3,
	WLF_TABLET_BUSTYPE_BLUETOOTH = 5,
};

/* -------------------------------------------------------------------------
 * wlf_zwp_tablet_manager_v2
 * ---------------------------------------------------------------------- */

/**
 * @brief Wrapper around a bound zwp_tablet_manager_v2 global.
 */
struct wlf_zwp_tablet_manager_v2 {
	struct zwp_tablet_manager_v2 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * wlf_zwp_tablet_seat_v2
 * ---------------------------------------------------------------------- */

/**
 * @brief Per-seat tablet manager object.
 *
 * Created by wlf_zwp_tablet_manager_v2_get_tablet_seat().
 * Emits signals when tablets, tools or pads are added.
 */
struct wlf_zwp_tablet_seat_v2 {
	struct zwp_tablet_seat_v2 *base;

	struct {
		/** Data: wlf_zwp_tablet_v2 *   — new tablet device     */
		struct wlf_signal tablet_added;
		/** Data: wlf_zwp_tablet_tool_v2 * — new tool           */
		struct wlf_signal tool_added;
		/** Data: wlf_zwp_tablet_pad_v2 *  — new pad device     */
		struct wlf_signal pad_added;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * wlf_zwp_tablet_v2
 * ---------------------------------------------------------------------- */

/**
 * @brief Represents a single graphics tablet device.
 *
 * String fields (name, paths) are heap-allocated; freed on destroy.
 * The @c paths array is NULL-terminated.
 */
struct wlf_zwp_tablet_v2 {
	struct zwp_tablet_v2 *base;

	char *name;          /**< Tablet name string, or NULL          */
	uint32_t vid;        /**< Vendor ID (USB)                      */
	uint32_t pid;        /**< Product ID (USB)                     */
	enum wlf_tablet_bustype bustype; /**< Bus type (since v2)      */

	char **paths;        /**< NULL-terminated array of device paths */
	size_t n_paths;

	struct {
		/** Emitted after all initial descriptive events. Data: self */
		struct wlf_signal done;
		/** Emitted when the tablet is removed.  Caller must destroy. */
		struct wlf_signal removed;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * wlf_zwp_tablet_tool_v2
 * ---------------------------------------------------------------------- */

/**
 * @brief Accumulated per-frame tool state.
 *
 * Updated before events.frame fires.
 */
struct wlf_zwp_tablet_tool_v2_state {
	double x;             /**< Surface-local X position (wl_fixed_t converted) */
	double y;             /**< Surface-local Y position                         */
	uint32_t pressure;    /**< Pressure [0, 65535]                              */
	uint32_t distance;    /**< Distance from surface [0, 65535]                 */
	double tilt_x;        /**< Tilt angle in degrees around X                   */
	double tilt_y;        /**< Tilt angle in degrees around Y                   */
	double rotation;      /**< Tool rotation in degrees                         */
	int32_t slider;       /**< Airbrush slider position [-65535, 65535]         */
	double wheel_degrees; /**< Mouse wheel discrete rotation in degrees          */
	int32_t wheel_clicks; /**< Mouse wheel click count                          */

	/** True while the tool is in contact with the surface. */
	int down;
	/** True while the tool is in proximity of the surface. */
	int proximity_in;
	/** Surface currently in proximity (valid when proximity_in). */
	struct wl_surface *surface;
	/** Tablet for current proximity (valid when proximity_in). */
	struct wlf_zwp_tablet_v2 *tablet;

	/** Last button event. */
	uint32_t button;
	enum wlf_tablet_button_state button_state;
	uint32_t button_serial;
};

/**
 * @brief Represents a single graphics tablet tool (pen, eraser, etc.).
 *
 * Current input state is accumulated in the @c current field between
 * two @c events.frame emissions.  Listeners should respond to
 * @c events.frame and read @c current at that point.
 */
struct wlf_zwp_tablet_tool_v2 {
	struct zwp_tablet_tool_v2 *base;

	enum wlf_tablet_tool_type type;
	uint32_t hardware_serial_hi;
	uint32_t hardware_serial_lo;
	uint32_t hardware_id_hi;   /**< Wacom hardware ID high word */
	uint32_t hardware_id_lo;   /**< Wacom hardware ID low word  */

	uint32_t capability_flags; /**< Bitmask of wlf_tablet_tool_capability */

	struct wlf_zwp_tablet_tool_v2_state current;

	struct {
		/** Emitted after all capability events. Data: self */
		struct wlf_signal done;
		struct wlf_signal removed;
		struct wlf_signal proximity_in;
		struct wlf_signal proximity_out;
		struct wlf_signal down;
		struct wlf_signal up;
		/** Emitted per logical input frame. Data: self */
		struct wlf_signal frame;
		struct wlf_signal button;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * wlf_zwp_tablet_pad_v2
 * ---------------------------------------------------------------------- */

/**
 * @brief Represents a graphics tablet pad (buttons, rings, strips, dials).
 *
 * Pad group sub-objects (zwp_tablet_pad_group_v2, ring, strip, dial) are
 * exposed as the raw protocol objects in the @c groups array.  Callers
 * may add their own listeners to these objects directly.
 */
struct wlf_zwp_tablet_pad_v2 {
	struct zwp_tablet_pad_v2 *base;

	uint32_t n_buttons;
	char **paths;          /**< NULL-terminated array of device paths */
	size_t n_paths;

	struct zwp_tablet_pad_group_v2 **groups;
	size_t n_groups;

	/** Last button event. */
	uint32_t button;
	uint32_t button_time;
	enum wlf_tablet_button_state button_state;

	struct {
		/** Emitted after all pad description events. Data: self */
		struct wlf_signal done;
		struct wlf_signal removed;
		/** Data: self (read button/button_state fields) */
		struct wlf_signal button;
		struct wlf_signal enter;
		struct wlf_signal leave;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * Manager API
 * ---------------------------------------------------------------------- */

/**
 * @brief Bind to the zwp_tablet_manager_v2 global from the registry.
 */
struct wlf_zwp_tablet_manager_v2 *wlf_zwp_tablet_manager_v2_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_zwp_tablet_manager_v2_destroy(
	struct wlf_zwp_tablet_manager_v2 *manager);

/**
 * @brief Create a per-seat tablet manager.
 *
 * @param manager  Bound tablet manager.
 * @param seat     The wl_seat to get tablet devices for.
 * @return A new wlf_zwp_tablet_seat_v2, or NULL on failure.
 */
struct wlf_zwp_tablet_seat_v2 *wlf_zwp_tablet_manager_v2_get_tablet_seat(
	struct wlf_zwp_tablet_manager_v2 *manager, struct wl_seat *seat);

/* -------------------------------------------------------------------------
 * Seat API
 * ---------------------------------------------------------------------- */

/**
 * @brief Destroy the tablet seat and free its resources.
 */
void wlf_zwp_tablet_seat_v2_destroy(struct wlf_zwp_tablet_seat_v2 *seat);

/* -------------------------------------------------------------------------
 * Tablet API
 * ---------------------------------------------------------------------- */

/**
 * @brief Destroy the tablet object.
 *
 * Must be called from the removed event handler.
 */
void wlf_zwp_tablet_v2_destroy(struct wlf_zwp_tablet_v2 *tablet);

/* -------------------------------------------------------------------------
 * Tool API
 * ---------------------------------------------------------------------- */

/**
 * @brief Set the cursor surface for the tool.
 *
 * @param tool     Tool object.
 * @param serial   Enter event serial.
 * @param surface  Cursor surface, or NULL to hide.
 * @param hotspot_x Cursor hotspot X.
 * @param hotspot_y Cursor hotspot Y.
 */
void wlf_zwp_tablet_tool_v2_set_cursor(
	struct wlf_zwp_tablet_tool_v2 *tool,
	uint32_t serial, struct wl_surface *surface,
	int32_t hotspot_x, int32_t hotspot_y);

/**
 * @brief Destroy the tool object.
 *
 * Must be called from the removed event handler.
 */
void wlf_zwp_tablet_tool_v2_destroy(struct wlf_zwp_tablet_tool_v2 *tool);

/* -------------------------------------------------------------------------
 * Pad API
 * ---------------------------------------------------------------------- */

/**
 * @brief Set button feedback string for a pad button.
 *
 * @param pad      Pad object.
 * @param button   Button index.
 * @param description  Description string.
 * @param serial   Mode switch serial.
 */
void wlf_zwp_tablet_pad_v2_set_feedback(
	struct wlf_zwp_tablet_pad_v2 *pad,
	uint32_t button, const char *description, uint32_t serial);

/**
 * @brief Destroy the pad object.
 *
 * Must be called from the removed event handler.
 */
void wlf_zwp_tablet_pad_v2_destroy(struct wlf_zwp_tablet_pad_v2 *pad);

#endif /* WAYLAND_WLF_ZWP_TABLET_MANAGER_V2_H */
