#include "wlf/wayland/wlf_wl_shm.h"
#include "wlf/wayland/wlf_wl_shm_pool.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void shm_handle_format(void *data, struct wl_shm *base, uint32_t format) {
	(void)base;
	(void)format;
	struct wlf_wl_shm *shm = data;
	wlf_signal_emit_mutable(&shm->events.format, shm);
}

static const struct wl_shm_listener wl_shm_listener = {
	.format = shm_handle_format,
};

struct wlf_wl_shm *wlf_wl_shm_create(struct wl_registry *wl_registry,
		uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_shm *shm = calloc(1, sizeof(*shm));
	if (shm == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_shm");
		return NULL;
	}

	uint32_t bind_version = version;
	if (version > (uint32_t)wl_shm_interface.version) {
		bind_version = (uint32_t)wl_shm_interface.version;
	}

	shm->wl_shm = wl_registry_bind(wl_registry, name,
		&wl_shm_interface, bind_version);
	if (shm->wl_shm == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_shm");
		free(shm);
		return NULL;
	}

	wlf_signal_init(&shm->events.destroy);
	wlf_signal_init(&shm->events.format);

	wl_shm_add_listener(shm->wl_shm, &wl_shm_listener, shm);

	return shm;
}

void wlf_wl_shm_destroy(struct wlf_wl_shm *shm) {
	if (shm == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&shm->events.destroy, shm);
	wl_shm_destroy(shm->wl_shm);
	free(shm);
}

struct wlf_wl_shm_pool *wlf_wl_shm_create_pool(struct wlf_wl_shm *shm,
		int fd, int32_t size) {
	assert(shm != NULL);

	struct wlf_wl_shm_pool *pool = calloc(1, sizeof(*pool));
	if (pool == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_shm_pool");
		return NULL;
	}

	pool->wl_shm_pool = wl_shm_create_pool(shm->wl_shm, fd, size);
	if (pool->wl_shm_pool == NULL) {
		wlf_log(WLF_ERROR, "wl_shm_create_pool failed");
		free(pool);
		return NULL;
	}

	wlf_signal_init(&pool->events.destroy);

	return pool;
}
