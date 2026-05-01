
/**
 * @file        wlf_array.h
 * @brief       Dynamic array utility for wlframe.
 * @details     This file provides a generic dynamic array implementation.
 *              It allows for dynamic allocation and management of contiguous memory
 *              for storing arbitrary data. The array automatically grows as needed
 *              and provides iteration macros for convenient traversal.
 * @author      YaoBing Xiao
 * @date        2026-05-01
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-01, initial version\n
 */

#ifndef UTILS_WLF_ARRAY_H
#define UTILS_WLF_ARRAY_H

#include <stddef.h>

/**
 * @brief Dynamic array structure.
 *
 * Manages a contiguous block of memory that can grow dynamically
 * as elements are added.
 */
struct wlf_array {
	size_t size;     /**< Current number of bytes used */
	size_t alloc;    /**< Allocated capacity in bytes */
	void *data;      /**< Pointer to array data */
};

/**
 * @brief Initializes a dynamic array.
 *
 * @param array Pointer to the array to initialize.
 */
void wlf_array_init(struct wlf_array *array);

/**
 * @brief Releases a dynamic array and frees its resources.
 *
 * @param array Pointer to the array to release.
 */
void wlf_array_release(struct wlf_array *array);

/**
 * @brief Adds space to the array for a new element.
 *
 * @param array Pointer to the array.
 * @param size Number of bytes to add.
 * @return Pointer to the newly allocated space, or NULL on failure.
 */
void *wlf_array_add(struct wlf_array *array, size_t size);

/**
 * @brief Copies the contents of a source array to a destination array.
 *
 * @param array Destination array pointer.
 * @param source Source array pointer.
 * @return 0 on success, non-zero on failure.
 */
int wlf_array_copy(struct wlf_array *array, struct wlf_array *source);

/**
 * @brief Iterate over all elements in an array.
 *
 * Usage: void *pos; wlf_array_for_each(pos, &array) { ... }
 *
 * @param pos Pointer variable for each element (should be cast to appropriate type).
 * @param array Pointer to the array to iterate.
 */
#define wlf_array_for_each(pos, array)					\
	for (pos = (array)->data;					\
	     (array)->size != 0 &&					\
	     (const char *) pos < ((const char *) (array)->data + (array)->size); \
	     (pos)++)

#endif // UTILS_WLF_ARRAY_H
