/**
 * @file        wlf_allocator.h
 * @brief       Allocator interface in wlframe.
 * @details     This file defines the generic allocator abstraction used to
 *              create buffers for rendering and composition backends.
 *              Implementations provide lifecycle and buffer allocation logic.
 * @author      YaoBing Xiao
 * @date        2026-04-13
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-13, initial version\n
 */

#ifndef ALLOCATOR_WLF_ALLOCATOR_H
#define ALLOCATOR_WLF_ALLOCATOR_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/buffer/wlf_buffer.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"

struct wlf_allocator;

/**
 * @brief Virtual method table for allocator implementations.
 */
struct wlf_allocator_impl {
	/**
	 * @brief Destroys allocator-specific resources.
	 *
	 * @param alloc Allocator instance.
	 */
	void (*destroy)(struct wlf_allocator *alloc);

	/**
	 * @brief Creates a buffer managed by this allocator.
	 *
	 * @param allocator Allocator instance.
	 * @param width Buffer width in pixels.
	 * @param height Buffer height in pixels.
	 * @return Newly created buffer, or NULL on failure.
	 */
	struct wlf_buffer *(*create_buffer)(struct wlf_allocator *allocator,
		uint32_t width, uint32_t height);
};

/**
 * @brief Base allocator object.
 */
struct wlf_allocator {
	const struct wlf_allocator_impl *impl; /**< Implementation vtable */

	struct {
		struct wlf_signal destroy; /**< Emitted before allocator is destroyed */
	} events;
};

/**
 * @brief Initializes an allocator object.
 *
 * @param allocator Allocator object to initialize.
 * @param impl Implementation vtable.
 */
void wlf_allocator_init(struct wlf_allocator *allocator,
	const struct wlf_allocator_impl *impl);

/**
 * @brief Automatically creates a suitable allocator for backend and renderer.
 *
 * @param backend Backend used by this allocator.
 * @param renderer Renderer used by this allocator.
 * @return Created allocator instance, or NULL when unavailable.
 */
struct wlf_allocator *wlf_allocator_autocreate(struct wlf_backend *backend,
	struct wlf_renderer *renderer);

/**
 * @brief Destroys an allocator object.
 *
 * Calls implementation-specific destroy callback and emits destroy signal.
 *
 * @param allocator Allocator object to destroy.
 */
void wlf_allocator_destroy(struct wlf_allocator *allocator);

/**
 * @brief Creates a buffer through allocator implementation.
 *
 * @param allocator Allocator used to create buffer.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 * @return Newly created buffer, or NULL on failure.
 */
struct wlf_buffer *wlf_allocator_create_buffer(struct wlf_allocator *allocator,
	uint32_t width, uint32_t height);

#endif // ALLOCATOR_WLF_ALLOCATOR_H
