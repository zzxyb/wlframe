#ifndef WLF_ALLOCATOR_H
#define WLF_ALLOCATOR_H

#include <wayland-server-core.h>

struct wlf_allocator;
struct wlf_backend;
struct wlf_drm_format;
struct wlf_renderer;

/**
 * @brief A structure representing the interface for an allocator
 */
struct wlf_allocator_interface {
	struct wlf_buffer *(*create_buffer)(struct wlf_allocator *alloc,
						int width, int height, 
						const struct wlf_drm_format *format); /**< Function to create a buffer */
	void (*destroy)(struct wlf_allocator *alloc); /**< Function to destroy the allocator */
};

/**
 * @brief Initializes an allocator
 * @param alloc Pointer to the allocator to initialize
 * @param impl Pointer to the allocator interface implementation
 * @param buffer_caps Capabilities of the buffers created with this allocator
 */
void wlf_allocator_init(struct wlf_allocator *alloc,
	const struct wlf_allocator_interface *impl, uint32_t buffer_caps);

/**
 * @brief A structure representing an allocator for pixel buffers
 *
 * An allocator is responsible for allocating memory for pixel buffers.
 * Each allocator may return buffers with different capabilities (shared
 * memory, DMA-BUF, memory mapping, etc.), placement (main memory, VRAM on a
 * GPU, etc.), and properties (possible usage, access performance, etc). 
 * See struct wlf_buffer for more details.
 *
 * An allocator can be passed to a struct wlf_swapchain for multiple buffering.
 */
struct wlf_allocator {
	const struct wlf_allocator_interface *impl; /**< Pointer to the allocator interface implementation */
	uint32_t buffer_caps;                        /**< Capabilities of the buffers created with this allocator */

	struct {
		struct wl_signal destroy;                 /**< Signal emitted when the allocator is destroyed */
	} events;
};

/**
 * @brief Creates an appropriate allocator given a backend and a renderer
 * @param backend Pointer to the backend to use
 * @param renderer Pointer to the renderer to use
 * @return Pointer to the newly created wlf_allocator structure
 */
struct wlf_allocator *wlf_allocator_autocreate(struct wlf_backend *backend,
	struct wlf_renderer *renderer);

/**
 * @brief Destroys an allocator and frees associated resources
 * @param alloc Pointer to the allocator to destroy
 */
void wlf_allocator_destroy(struct wlf_allocator *alloc);

/**
 * @brief Allocates a new buffer
 * @param alloc Pointer to the allocator to use
 * @param width Width of the buffer to allocate
 * @param height Height of the buffer to allocate
 * @param format Pointer to the DRM format to use for the buffer
 * @return Pointer to the newly allocated wlf_buffer structure
 *
 * When the caller is done with the buffer, they must unreference it by calling
 * wlf_buffer_drop().
 *
 * The `format` passed in indicates the format to use and the list of
 * acceptable modifiers. The order in which modifiers are listed is not
 * significant.
 *
 * When running with legacy drivers which don't support explicit modifiers, the
 * allocator must recognize two modifiers: INVALID (for implicit tiling and/or
 * compression) and LINEAR.
 *
 * The allocator must return a buffer using one of the modifiers listed. In
 * particular, allocators must not return a buffer with an implicit modifier
 * unless the user has allowed it by passing INVALID in the modifier list.
 */
struct wlf_buffer *wlf_allocator_create_buffer(struct wlf_allocator *alloc,
	int width, int height, const struct wlf_drm_format *format);

#endif
