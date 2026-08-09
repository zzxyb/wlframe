/**
 * @file        wlf_zwp_text_input_manager_v3.h
 * @brief       zwp-text-input-v3 client wrappers.
 * @details     Wraps text-input state, pending requests, and protocol events
 *              for keyboard-composition and input-method integration.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_ZWP_TEXT_INPUT_MANAGER_V3_H
#define WLF_ZWP_TEXT_INPUT_MANAGER_V3_H

#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct wl_registry;
struct wl_seat;
struct wl_surface;
struct zwp_text_input_manager_v3;
struct zwp_text_input_v3;

/**
 * @brief Text-input-v3 object and its committed/pending state.
 *
 * The committed fields are updated by compositor done events. Requests made
 * by the caller accumulate in @p pending until they are committed.
 */
struct wlf_zwp_text_input_v3 {
	struct zwp_text_input_v3 *base; /**< Protocol object. */

	/**
	 * @brief Current state, updated when the compositor sends done.
	 */
	struct wl_surface *surface; /**< Current enter/leave surface. */
	char *preedit_text; /**< Current preedit string. */
	int32_t preedit_cursor_begin; /**< Start of the preedit cursor range. */
	int32_t preedit_cursor_end; /**< End of the preedit cursor range. */
	char *commit_text; /**< Current committed text. */
	uint32_t delete_before_length; /**< Bytes to delete before the cursor. */
	uint32_t delete_after_length; /**< Bytes to delete after the cursor. */
	uint32_t done_serial; /**< Serial of the last done event. */

	/**
	 * @brief Pending state accumulated between done events.
	 */
	struct {
		char *preedit_text; /**< Pending preedit string. */
		int32_t preedit_cursor_begin; /**< Pending preedit cursor start. */
		int32_t preedit_cursor_end; /**< Pending preedit cursor end. */
		char *commit_text; /**< Pending committed text. */
		uint32_t delete_before_length; /**< Pending deletion before cursor. */
		uint32_t delete_after_length; /**< Pending deletion after cursor. */
	} pending;

	struct {
		struct wlf_signal enter;
		struct wlf_signal leave;
		struct wlf_signal preedit_string;
		struct wlf_signal commit_string;
		struct wlf_signal delete_surrounding_text;
		struct wlf_signal done;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Text-input-v3 manager wrapper.
 *
 * The manager creates one text-input object per Wayland seat.
 */
struct wlf_zwp_text_input_manager_v3 {
	struct zwp_text_input_manager_v3 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Creates a text-input manager wrapper.
 * @param registry Wayland registry used to bind the manager.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_zwp_text_input_manager_v3 *wlf_zwp_text_input_manager_v3_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys a text-input manager.
 * @param manager Manager to destroy.
 */
void wlf_zwp_text_input_manager_v3_destroy(
	struct wlf_zwp_text_input_manager_v3 *manager);

/**
 * @brief Obtains the text-input object for a seat.
 * @param manager Text-input manager.
 * @param seat Seat whose text-input object should be created.
 * @return Text-input object, or NULL on failure.
 */
struct wlf_zwp_text_input_v3 *wlf_zwp_text_input_manager_v3_get_text_input(
	struct wlf_zwp_text_input_manager_v3 *manager, struct wl_seat *seat);

/**
 * @brief Enables text input for the current focus surface.
 * @param ti Text-input object to enable.
 */
void wlf_zwp_text_input_v3_enable(struct wlf_zwp_text_input_v3 *ti);

/**
 * @brief Disables text input.
 * @param ti Text-input object to disable.
 */
void wlf_zwp_text_input_v3_disable(struct wlf_zwp_text_input_v3 *ti);

/**
 * @brief Sets surrounding UTF-8 text and cursor/anchor positions.
 * @param ti Text-input object to update.
 * @param text Surrounding UTF-8 text.
 * @param cursor Cursor byte offset in @p text.
 * @param anchor Selection anchor byte offset in @p text.
 */
void wlf_zwp_text_input_v3_set_surrounding_text(
	struct wlf_zwp_text_input_v3 *ti, const char *text,
	int32_t cursor, int32_t anchor);

/**
 * @brief Sets the reason for the next text change.
 * @param ti Text-input object to update.
 * @param cause Protocol text-change cause value.
 */
void wlf_zwp_text_input_v3_set_text_change_cause(
	struct wlf_zwp_text_input_v3 *ti, uint32_t cause);

/**
 * @brief Sets content hints and purpose for the input method.
 * @param ti Text-input object to update.
 * @param hint Protocol content-hint bitmask.
 * @param purpose Protocol content-purpose value.
 */
void wlf_zwp_text_input_v3_set_content_type(
	struct wlf_zwp_text_input_v3 *ti, uint32_t hint, uint32_t purpose);

/**
 * @brief Sets the cursor rectangle in surface-local coordinates.
 * @param ti Text-input object to update.
 * @param x Rectangle x coordinate.
 * @param y Rectangle y coordinate.
 * @param width Rectangle width.
 * @param height Rectangle height.
 */
void wlf_zwp_text_input_v3_set_cursor_rectangle(
	struct wlf_zwp_text_input_v3 *ti,
	int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Commits pending text-input state to the compositor.
 * @param ti Text-input object whose pending state is committed.
 */
void wlf_zwp_text_input_v3_commit(struct wlf_zwp_text_input_v3 *ti);

/**
 * @brief Destroys a text-input object.
 * @param ti Text-input object to destroy.
 */
void wlf_zwp_text_input_v3_destroy(struct wlf_zwp_text_input_v3 *ti);

#endif /* WLF_ZWP_TEXT_INPUT_MANAGER_V3_H */
