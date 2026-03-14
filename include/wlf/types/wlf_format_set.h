/**
 * @file        wlf_format_set.h
 * @brief       Pixel format and modifier set utility for wlframe.
 * @details     This file provides a cross-platform set container for
 *              format+modifier pairs.
 *              The API is platform-neutral and suitable for cross-platform
 *              usage without OS-specific dependencies.
 *              The format field uses wlframe fourcc values defined in
 *              wlf_pixel_format.h.
 * @author      YaoBing Xiao
 * @date        2026-03-14
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-14, initial version\n
 */

#ifndef TYPES_WLF_FORMAT_SET_H
#define TYPES_WLF_FORMAT_SET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Special modifier used to indicate implicit/unknown layout support.
 *
 * This matches the numeric value used by DRM_FORMAT_MOD_INVALID so callers can
 * interoperate with external graphics code paths when needed while keeping the
 * API cross-platform.
 */
#define WLF_FORMAT_MOD_INVALID UINT64_C(0x00ffffffffffffff)

/**
 * @brief Modifier used to indicate a linear memory layout.
 */
#define WLF_FORMAT_MOD_LINEAR UINT64_C(0)

/**
 * @brief A single pixel format with a set of modifiers.
 *
 * A format entry stores one format code and all supported memory layout
 * modifiers for that format.
 */
struct wlf_format {
	uint32_t format;    /**< Pixel format code (fourcc), see enum wlf_pixel_format. */
	size_t len;         /**< Number of valid entries in modifiers. */
	size_t capacity;    /**< Allocated capacity for modifiers. Internal use only. */
	uint64_t *modifiers; /**< Modifiers array of length len. */
};

/**
 * @brief Release resources held by a format entry.
 *
 * @param format Format entry to clear.
 */
void wlf_format_finish(struct wlf_format *format);

/**
 * @brief A set of pixel formats and modifiers.
 *
 * This container describes supported format+modifier combinations.
 */
struct wlf_format_set {
	size_t len;                 /**< Number of valid entries in formats. */
	size_t capacity;            /**< Allocated capacity for formats. Internal use only. */
	struct wlf_format *formats; /**< Array of format entries. */
};

/**
 * @brief Free all entries and reset the set to empty.
 *
 * @param set Set to clear.
 */
void wlf_format_set_finish(struct wlf_format_set *set);

/**
 * @brief Find the format entry by format code.
 *
 * @param set Set to search in.
 * @param format Format code to find.
 * @return Pointer to the matching entry, or NULL when not found.
 */
const struct wlf_format *wlf_format_set_get(
	const struct wlf_format_set *set, uint32_t format);

/**
 * @brief Remove one format+modifier pair from the set.
 *
 * @param set Set to modify.
 * @param format Format code.
 * @param modifier Modifier value to remove.
 * @return true when removed, false if not found.
 */
bool wlf_format_set_remove(struct wlf_format_set *set, uint32_t format,
	uint64_t modifier);

/**
 * @brief Check whether a format+modifier pair exists in the set.
 *
 * @param set Set to query.
 * @param format Format code.
 * @param modifier Modifier value.
 * @return true when present, otherwise false.
 */
bool wlf_format_set_has(const struct wlf_format_set *set,
	uint32_t format, uint64_t modifier);

/**
 * @brief Insert a format+modifier pair into the set.
 *
 * @param set Set to modify.
 * @param format Format code.
 * @param modifier Modifier value to add.
 * @return true on success (including when already present), false on OOM.
 */
bool wlf_format_set_add(struct wlf_format_set *set, uint32_t format,
	uint64_t modifier);

/**
 * @brief Compute intersection of two sets and store in dst.
 *
 * dst can be empty or pre-initialized. Existing content in dst will be
 * replaced.
 *
 * @param dst Destination set.
 * @param a Left source set.
 * @param b Right source set.
 * @return true when intersection is non-empty, false on OOM or when empty.
 */
bool wlf_format_set_intersect(struct wlf_format_set *dst,
	const struct wlf_format_set *a, const struct wlf_format_set *b);

/**
 * @brief Compute union of two sets and store in dst.
 *
 * dst can be empty or pre-initialized. Existing content in dst will be
 * replaced.
 *
 * @param dst Destination set.
 * @param a Left source set.
 * @param b Right source set.
 * @return true on success, false on OOM.
 */
bool wlf_format_set_union(struct wlf_format_set *dst,
	const struct wlf_format_set *a, const struct wlf_format_set *b);

#endif // TYPES_WLF_FORMAT_SET_H
