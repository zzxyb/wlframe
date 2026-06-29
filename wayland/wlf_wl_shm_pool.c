#include "wlf/wayland/wlf_wl_shm_pool.h"
#include "wlf/wayland/wlf_wl_buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

void wlf_wl_shm_pool_destroy(struct wlf_wl_shm_pool *pool) {
	if (pool == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&pool->events.destroy, pool);
	wl_shm_pool_destroy(pool->wl_shm_pool);
	free(pool);
}

void wlf_wl_shm_pool_resize(struct wlf_wl_shm_pool *pool, int32_t size) {
	assert(pool != NULL);
	wl_shm_pool_resize(pool->wl_shm_pool, size);
}

struct wlf_wl_buffer *wlf_wl_shm_pool_create_buffer(struct wlf_wl_shm_pool *pool,
		int32_t offset, int32_t width, int32_t height,
		int32_t stride, uint32_t format) {
	assert(pool != NULL);

	struct wlf_wl_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_buffer");
		return NULL;
	}

	struct wl_buffer *wl_buffer = wl_shm_pool_create_buffer(pool->wl_shm_pool,
		offset, width, height, stride, format);
	if (wl_buffer == NULL) {
		wlf_log(WLF_ERROR, "wl_shm_pool_create_buffer failed");
		free(buffer);
		return NULL;
	}

	free(buffer);
	return wlf_wl_buffer_wrap(wl_buffer);
}
