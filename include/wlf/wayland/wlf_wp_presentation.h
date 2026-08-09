/**
 * @file        wlf_wp_presentation.h
 * @brief       Wayland wp_presentation protocol wrapper for wlframe.
 * @details     Implements the stable wp_presentation (presentation-time) protocol,
 *              which provides accurate per-frame presentation timestamps to clients.
 *
 *              The compositor advertises the presentation clock via a clock_id event
 *              on bind.  For each wl_surface.commit a client may request a
 *              wp_presentation_feedback object.  The compositor then delivers one of:
 *                - presented  – the frame was shown; timestamp and sequence are valid.
 *                - discarded  – the frame was superseded and never shown.
 *
 *              Once the feedback object delivers either event it is automatically
 *              destroyed on the protocol level.  The wlframe wrapper emits the
 *              corresponding signal and then frees itself, so callers must not
 *              access a feedback pointer after the signal fires.
 *
 *              Design reference: Chromium ui/ozone/platform/wayland presentation
 *              feedback handling (WaylandFrameManager).
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_WP_PRESENTATION_H
#define WAYLAND_WLF_WP_PRESENTATION_H

#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stdint.h>

struct wl_output;
struct wl_registry;
struct wl_surface;
struct wp_presentation;
struct wp_presentation_feedback;

/**
 * @brief Wrapper around a bound wp_presentation global.
 *
 * Obtain via wlf_wp_presentation_create().  Tracks the presentation clock ID
 * and emits a destroy signal before teardown.
 */
struct wlf_wp_presentation {
	struct wp_presentation *base; /**< Underlying protocol object */
	uint32_t version;	      /**< Bound protocol version */
	uint32_t clk_id;	      /**< POSIX clock ID (clock_id event) */
	bool has_clock_id;            /**< Whether clock_id has been received */

	struct {
		/** Emitted with this object when clk_id becomes available. */
		struct wlf_signal clock_id;
		struct wlf_signal
			destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Per-frame presentation feedback.
 *
 * Created by wlf_wp_presentation_request_feedback().  The object is
 * self-managed: after the presented or discarded signal fires the struct is
 * freed automatically.  Callers must not retain or dereference the pointer
 * after the signal handler returns.
 *
 * Fields below are only valid inside a presented signal handler.
 */
struct wlf_wp_presentation_feedback {
	struct wp_presentation_feedback *base;
	uint32_t version;

	/**
	 * Seconds part of the presentation timestamp.
	 * Reconstructed from tv_sec_hi and tv_sec_lo protocol fields.
	 */
	uint64_t tv_sec;

	/** Nanoseconds part of the presentation timestamp [0, 999999999]. */
	uint32_t tv_nsec;

	/** Compositor prediction: nanoseconds until the next refresh, or 0. */
	uint32_t refresh;

	/**
	 * Output vertical-retrace counter at presentation time.
	 * Reconstructed from seq_hi and seq_lo protocol fields.
	 */
	uint64_t seq;

	/** Bitmask of wp_presentation_feedback_kind flags. */
	uint32_t flags;

	struct {
		/**
		 * Emitted once for each output the presentation was
		 * synchronised to, before the presented event.
		 * Signal data: struct wl_output *
		 */
		struct wlf_signal sync_output;

		/**
		 * Emitted when the frame was successfully displayed.
		 * Signal data: struct wlf_wp_presentation_feedback *
		 * The struct is freed immediately after this signal returns.
		 */
		struct wlf_signal presented;

		/**
		 * Emitted when the frame was discarded (never shown).
		 * Signal data: struct wlf_wp_presentation_feedback *
		 * The struct is freed immediately after this signal returns.
		 */
		struct wlf_signal discarded;

		/** Emitted immediately before the wrapper is freed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind the wp_presentation global from the Wayland registry.
 *
 * @param wl_registry  Registry to bind from.
 * @param name         Global name received in the registry announce event.
 * @param version      Advertised interface version.
 * @return Newly allocated wrapper, or NULL on failure.
 */
struct wlf_wp_presentation *wlf_wp_presentation_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the wp_presentation wrapper and the underlying protocol object.
 *
 * Emits events.destroy before freeing.  Accepts NULL.
 *
 * @param presentation  Object to destroy.
 */
void wlf_wp_presentation_destroy(struct wlf_wp_presentation *presentation);

/**
 * @brief Request presentation feedback for the surface's next commit.
 *
 * The request applies to the surface's next commit. The returned feedback
 * object is self-managed and will free itself after emitting either the
 * presented or discarded signal.
 *
 * @param presentation  Bound wp_presentation object.
 * @param surface       Surface whose next commit is being tracked.
 * @return Feedback wrapper, or NULL on failure.
 */
struct wlf_wp_presentation_feedback *wlf_wp_presentation_request_feedback(
	struct wlf_wp_presentation *presentation, struct wl_surface *surface);

/**
 * @brief Stop tracking feedback and free the local wrapper.
 *
 * No protocol destroy request exists for feedback objects. This function
 * destroys the local proxy, causing any later server events to be ignored.
 */
void wlf_wp_presentation_feedback_destroy(
	struct wlf_wp_presentation_feedback *feedback);

#endif // WAYLAND_WLF_WP_PRESENTATION_H
