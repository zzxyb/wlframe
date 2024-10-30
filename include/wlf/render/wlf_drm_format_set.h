#ifndef WLF_DRM_FORMAT_SET_H
#define WLF_DRM_FORMAT_SET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief A structure representing a single DRM format with a set of modifiers attached
 */
struct wlf_drm_format {
	uint32_t format;    /**< The actual DRM format, from `drm_fourcc.h` */
	size_t len;        /**< The number of modifiers */
	size_t capacity;   /**< The capacity of the array; do not use */
	uint64_t *modifiers; /**< Pointer to the actual modifiers */
};

/**
 * @brief Frees all resources allocated to a DRM format
 * @param format Pointer to the DRM format to finish
 */
void wlf_drm_format_finish(struct wlf_drm_format *format);

/**
 * @brief A structure representing a set of DRM formats and modifiers
 *
 * This structure is used to describe the supported format + modifier combinations. 
 * For instance, backends will report the set they can display, and renderers will 
 * report the set they can render to. For a more general overview of formats and 
 * modifiers, see:
 * https://www.kernel.org/doc/html/next/userspace-api/dma-buf-alloc-exchange.html#formats-and-modifiers
 *
 * For compatibility with legacy drivers which don't support explicit modifiers, 
 * the special modifier DRM_FORMAT_MOD_INVALID is used to indicate that implicit 
 * modifiers are supported. Legacy drivers can also support the DRM_FORMAT_MOD_LINEAR 
 * modifier, which forces the buffer to have a linear layout.
 *
 * Users must not assume that implicit modifiers are supported unless INVALID 
 * is listed in the modifier list.
 */
struct wlf_drm_format_set {
	size_t len;       /**< The number of formats */
	size_t capacity;  /**< The capacity of the array; private to wlfoots */
	struct wlf_drm_format *formats; /**< Pointer to an array of `struct wlf_drm_format *` of length `len` */
};

/**
 * @brief Frees all of the DRM formats in the set, making the set empty
 * @param set Pointer to the DRM format set to finish
 */
void wlf_drm_format_set_finish(struct wlf_drm_format_set *set);

/**
 * @brief Returns a pointer to a member of the DRM format set of a specified format
 * @param set Pointer to the DRM format set to search
 * @param format The format to look for
 * @return Pointer to the matching DRM format, or NULL if none exists
 */
const struct wlf_drm_format *wlf_drm_format_set_get(
	const struct wlf_drm_format_set *set, uint32_t format);

/**
 * @brief Checks if a specific format and modifier exist in the DRM format set
 * @param set Pointer to the DRM format set to check
 * @param format The format to check for
 * @param modifier The modifier to check for
 * @return true if the format and modifier exist, false otherwise
 */
bool wlf_drm_format_set_has(const struct wlf_drm_format_set *set,
	uint32_t format, uint64_t modifier);

/**
 * @brief Adds a format and modifier to the DRM format set
 * @param set Pointer to the DRM format set to add to
 * @param format The format to add
 * @param modifier The modifier to add
 * @return true if the addition was successful, false otherwise
 */
bool wlf_drm_format_set_add(struct wlf_drm_format_set *set, uint32_t format,
	uint64_t modifier);

/**
 * @brief Intersects two DRM format sets, storing the result in a destination set
 * @param dst Pointer to the destination DRM format set
 * @param a Pointer to the first source DRM format set
 * @param b Pointer to the second source DRM format set
 * @return false on failure or when the intersection is empty, true otherwise
 */
bool wlf_drm_format_set_intersect(struct wlf_drm_format_set *dst,
	const struct wlf_drm_format_set *a, const struct wlf_drm_format_set *b);

/**
 * @brief Unions two DRM format sets, storing the result in a destination set
 * @param dst Pointer to the destination DRM format set
 * @param a Pointer to the first source DRM format set
 * @param b Pointer to the second source DRM format set
 * @return false on failure, true otherwise
 */
bool wlf_drm_format_set_union(struct wlf_drm_format_set *dst,
	const struct wlf_drm_format_set *a, const struct wlf_drm_format_set *b);

#endif
