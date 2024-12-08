#ifndef WLF_BUFFER_H
#define WLF_BUFFER_H

#include "wlf/utils/wlf_addon.h"
#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_signal.h"

#define WLF_DMABUF_MAX_PLANES 4

struct wlf_buffer;

enum wlf_buffer_cap {
	WLF_BUFFER_CAP_DATA_PTR = 1 << 0,
	WLF_BUFFER_CAP_DMABUF = 1 << 1,
	WLF_BUFFER_CAP_SHM = 1 << 2,
};

struct wlf_shm_attributes {
	int fd;
	uint32_t format;
	int width, height;
	int stride;
	size_t offset;
};

struct wlf_dmabuf_attributes {
	int32_t width, height;
	uint32_t format;
	uint64_t modifier;

	int n_planes;
	uint32_t offset[WLF_DMABUF_MAX_PLANES];
	uint32_t stride[WLF_DMABUF_MAX_PLANES];
	int fd[WLF_DMABUF_MAX_PLANES];
};

/**
 * @brief Closes all file descriptors in the DMA-BUF attributes
 * @param attribs Pointer to the DMA-BUF attributes to finish
 */
void wlf_dmabuf_attributes_finish(struct wlf_dmabuf_attributes *attribs);

/**
 * @brief Clones the DMA-BUF attributes
 * @param dst Pointer to the destination DMA-BUF attributes
 * @param src Pointer to the source DMA-BUF attributes to copy from
 * @return true if the cloning was successful, false otherwise
 */
bool wlf_dmabuf_attributes_copy(struct wlf_dmabuf_attributes *dst,
	const struct wlf_dmabuf_attributes *src);

struct wlf_buffer_impl {
	void (*destroy)(struct wlf_buffer *buffer);
	bool (*get_dmabuf)(struct wlf_buffer *buffer,
		struct wlf_dmabuf_attributes *attribs);
	bool (*get_shm)(struct wlf_buffer *buffer,
		struct wlf_shm_attributes *attribs);
	bool (*begin_data_ptr_access)(struct wlf_buffer *buffer, uint32_t flags,
		void **data, uint32_t *format, size_t *stride);
	void (*end_data_ptr_access)(struct wlf_buffer *buffer);
};

struct wlf_buffer {
	const struct wlf_buffer_impl *impl;

	int width, height;

	bool dropped;
	size_t n_locks;
	bool accessing_data_ptr;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal release;
	} events;

	struct wlf_addon_set addons;
};

void wlf_buffer_init(struct wlf_buffer *buffer,
	const struct wlf_buffer_impl *impl, int width, int height);

/**
 * Unreference the buffer. This function should be called by producers when
 * they are done with the buffer.
 */
void wlf_buffer_drop(struct wlf_buffer *buffer);

/**
 * Lock the buffer. This function should be called by consumers to make
 * sure the buffer can be safely read from. Once the consumer is done with the
 * buffer, they should call wlf_buffer_unlock().
 */
struct wlf_buffer *wlf_buffer_lock(struct wlf_buffer *buffer);

/**
 * Unlock the buffer. This function should be called by consumers once they are
 * done with the buffer.
 */
void wlf_buffer_unlock(struct wlf_buffer *buffer);

/**
 * Reads the DMA-BUF attributes of the buffer. If this buffer isn't a DMA-BUF,
 * returns false.
 *
 * The returned DMA-BUF attributes are valid for the lifetime of the
 * struct wlf_buffer. The caller isn't responsible for cleaning up the DMA-BUF
 * attributes.
 */
bool wlf_buffer_get_dmabuf(struct wlf_buffer *buffer,
	struct wlf_dmabuf_attributes *attribs);

/**
 * Read shared memory attributes of the buffer. If this buffer isn't shared
 * memory, returns false.
 *
 * The returned shared memory attributes are valid for the lifetime of the
 * struct wlf_buffer. The caller isn't responsible for cleaning up the shared
 * memory attributes.
 */
bool wlf_buffer_get_shm(struct wlf_buffer *buffer,
	struct wlf_shm_attributes *attribs);

/**
 * Buffer data pointer access flags.
 */
enum wlf_buffer_data_ptr_access_flag {
	/**
	 * The buffer contents can be read back.
	 */
	WLR_BUFFER_DATA_PTR_ACCESS_READ = 1 << 0,
	/**
	 * The buffer contents can be written to.
	 */
	WLR_BUFFER_DATA_PTR_ACCESS_WRITE = 1 << 1,
};

/**
 * Get a pointer to a region of memory referring to the buffer's underlying
 * storage. The format and stride can be used to interpret the memory region
 * contents.
 *
 * The returned pointer should be pointing to a valid memory region for the
 * operations specified in the flags. The returned pointer is only valid up to
 * the next wlf_buffer_end_data_ptr_access() call.
 */
bool wlf_buffer_begin_data_ptr_access(struct wlf_buffer *buffer, uint32_t flags,
	void **data, uint32_t *format, size_t *stride);

/**
 * Indicate that a pointer to a buffer's underlying memory will no longer be
 * used.
 *
 * This function must be called after wlf_buffer_begin_data_ptr_access().
 */
void wlf_buffer_end_data_ptr_access(struct wlf_buffer *buffer);

#endif
