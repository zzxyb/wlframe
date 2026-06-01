/**
 * @file        wlf_palette.h
 * @brief       Shared widget palette for wlframe.
 * @details     This file defines lightweight semantic palette data that can be
 *              reused by widgets built directly on top of `wlf_scene_tree`.
 *              The goal is to keep each widget structure simple while still
 *              providing a consistent place to control theme colors.
 * @author      YaoBing Xiao
 * @date        2026-06-04
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, Codex, 2026-06-04, initial version\n
 */

#ifndef WIDGETS_WLF_PALETTE_H
#define WIDGETS_WLF_PALETTE_H

#include "wlf/pass/wlf_pass.h"
#include "wlf/pass/wlf_text_pass.h"
#include "wlf/types/wlf_color.h"

#include <stdbool.h>

/**
 * @brief Global theme mode.
 */
enum wlf_palette_mode {
	WLF_PALETTE_MODE_LIGHT = 0, /**< Light appearance */
	WLF_PALETTE_MODE_DARK,      /**< Dark appearance */
};

/**
 * @brief Common widget interaction states used for color selection.
 */
enum wlf_widget_state {
	WLF_WIDGET_STATE_NORMAL = 0,   /**< Default state */
	WLF_WIDGET_STATE_HOVERED,      /**< Pointer hover state */
	WLF_WIDGET_STATE_PRESSED,      /**< Pointer pressed state */
	WLF_WIDGET_STATE_DISABLED,     /**< Disabled state */
	WLF_WIDGET_STATE_SELECTED,     /**< Selected state */
	WLF_WIDGET_STATE_CHECKED,      /**< Checked/on state */
	WLF_WIDGET_STATE_COUNT         /**< Number of state entries */
};

/**
 * @brief Primary widget axis direction.
 */
enum wlf_widget_orientation {
	WLF_WIDGET_HORIZONTAL = 0, /**< Left-to-right / horizontal */
	WLF_WIDGET_VERTICAL        /**< Top-to-bottom / vertical */
};

/**
 * @brief How an icon should fit into its target box.
 */
enum wlf_widget_icon_fit_mode {
	WLF_WIDGET_ICON_CONTAIN = 0, /**< Preserve aspect ratio and fit inside */
	WLF_WIDGET_ICON_COVER,       /**< Preserve aspect ratio and cover the box */
	WLF_WIDGET_ICON_FILL         /**< Stretch to fill the box */
};

/**
 * @brief Shared semantic colors for widgets.
 */
struct wlf_palette {
	enum wlf_palette_mode mode;             /**< Light or dark theme */

	struct wlf_color background_color;      /**< Primary window/background color */
	struct wlf_color surface_color;         /**< Surface or card color */
	struct wlf_color surface_alt_color;     /**< Alternate surface color */
	struct wlf_color text_color;            /**< Primary text color */
	struct wlf_color text_muted_color;      /**< Secondary text color */
	struct wlf_color border_color;          /**< Neutral border color */
	struct wlf_color accent_color;          /**< Primary accent color */
	struct wlf_color accent_text_color;     /**< Foreground color on accent surfaces */
	struct wlf_color success_color;         /**< Success state color */
	struct wlf_color warning_color;         /**< Warning state color */
	struct wlf_color danger_color;          /**< Error/destructive state color */
	struct wlf_color disabled_fill_color;   /**< Disabled fill color */
	struct wlf_color disabled_text_color;   /**< Disabled text color */
	struct wlf_color focus_ring_color;      /**< Focus/highlight color */
};

#endif // WIDGETS_WLF_PALETTE_H
