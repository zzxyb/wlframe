/**
 * @file        wlf_format_set.h
 * @brief       render format and modifier set utilities.
 * @details     This file defines render format descriptors and format-set helpers used
 *              to store, query, and combine DRM formats with their supported modifiers.
 * @author      YaoBing Xiao
 * @date        2026-04-25
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-25, initial version\n
 */

#ifndef TYPES_WLF_FORMAT_SET_H
#define TYPES_WLF_FORMAT_SET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief A single DRM format with a set of supported modifiers.
 */
struct wlf_render_format {
	/** DRM format code from wlf_pixel_format. */
	uint32_t format;
	/** Number of valid entries in modifiers. */
	size_t len;
	/** Capacity of modifiers array; internal use only. */
	size_t capacity;
	/** Array of supported DRM modifiers. */
	uint64_t *modifiers;
};

/**
 * @brief A set of DRM formats and their modifiers.
 */
struct wlf_render_format_set {
	/** Number of valid entries in formats. */
	size_t len;
	/** Capacity of formats array; internal use only. */
	size_t capacity;
	/** Array of render formats. */
	struct wlf_render_format *formats;
};

/**
 * @brief Initializes a render format.
 *
 * @param fmt Render format to initialize.
 * @param format DRM format code.
 */
void wlf_render_format_init(struct wlf_render_format *fmt, uint32_t format);

/**
 * @brief Releases resources owned by a render format.
 *
 * @param format Render format to finish.
 */
void wlf_render_format_finish(struct wlf_render_format *format);

/**
 * @brief Checks whether a render format contains a modifier.
 *
 * @param fmt Render format to query.
 * @param modifier DRM modifier to check.
 * @return true if the modifier exists, otherwise false.
 */
bool wlf_render_format_has(const struct wlf_render_format *fmt, uint64_t modifier);

/**
 * @brief Adds a modifier to a render format.
 *
 * @param fmt Render format to modify.
 * @param modifier DRM modifier to add.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_add(struct wlf_render_format *fmt, uint64_t modifier);

/**
 * @brief Copies a render format.
 *
 * @param dst Destination render format.
 * @param src Source render format.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_copy(struct wlf_render_format *dst, const struct wlf_render_format *src);

/**
 * @brief Computes intersection of two render formats.
 *
 * @param dst Destination for intersection result.
 * @param a First input render format.
 * @param b Second input render format.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_intersect(struct wlf_render_format *dst,
	const struct wlf_render_format *a, const struct wlf_render_format *b);

/**
 * @brief Releases resources owned by a render format set.
 *
 * @param set Render format set to finish.
 */
void wlf_render_format_set_finish(struct wlf_render_format_set *set);

/**
 * @brief Gets a render format by DRM format code.
 *
 * @param set Render format set to query.
 * @param format DRM format code.
 * @return Matching render format, or NULL if not found.
 */
const struct wlf_render_format *wlf_render_format_set_get(
	const struct wlf_render_format_set *set, uint32_t format);

/**
 * @brief Removes a modifier from a format in the set.
 *
 * @param set Render format set to modify.
 * @param format DRM format code.
 * @param modifier DRM modifier to remove.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_set_remove(struct wlf_render_format_set *set, uint32_t format,
	uint64_t modifier);

/**
 * @brief Checks whether a format in the set has a modifier.
 *
 * @param set Render format set to query.
 * @param format DRM format code.
 * @param modifier DRM modifier to check.
 * @return true if present, otherwise false.
 */
bool wlf_render_format_set_has(const struct wlf_render_format_set *set,
	uint32_t format, uint64_t modifier);

/**
 * @brief Adds a format/modifier pair to the set.
 *
 * @param set Render format set to modify.
 * @param format DRM format code.
 * @param modifier DRM modifier to add.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_set_add(struct wlf_render_format_set *set, uint32_t format,
	uint64_t modifier);

/**
 * @brief Computes intersection of two render format sets.
 *
 * @param dst Destination for intersection result.
 * @param a First input format set.
 * @param b Second input format set.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_set_intersect(struct wlf_render_format_set *dst,
	const struct wlf_render_format_set *a, const struct wlf_render_format_set *b);

/**
 * @brief Computes union of two render format sets.
 *
 * @param dst Destination for union result.
 * @param a First input format set.
 * @param b Second input format set.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_set_union(struct wlf_render_format_set *dst,
	const struct wlf_render_format_set *a, const struct wlf_render_format_set *b);

/**
 * @brief Copies a render format set.
 *
 * @param dst Destination render format set.
 * @param src Source render format set.
 * @return true on success, otherwise false.
 */
bool wlf_render_format_set_copy(struct wlf_render_format_set *dst, const struct wlf_render_format_set *src);

#endif // TYPES_WLF_FORMAT_SET_H
