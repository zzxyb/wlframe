#include "wlf/wayland/wlf_zwp_linux_dmabuf_v1.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_utils.h"
#include "wayland/protocols/linux-dmabuf-v1-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <wayland-client-protocol.h>

/* ── buffer params listeners ─────────────────────────────────────────────── */

static void params_created(void *data,
		struct zwp_linux_buffer_params_v1 *wp_params,
		struct wl_buffer *buffer) {
	WLF_UNUSED(wp_params);
	struct wlf_zwp_linux_buffer_params_v1 *params = data;
	wlf_signal_emit_mutable(&params->events.created, buffer);
}

static void params_failed(void *data,
		struct zwp_linux_buffer_params_v1 *wp_params) {
	WLF_UNUSED(wp_params);
	struct wlf_zwp_linux_buffer_params_v1 *params = data;
	wlf_log(WLF_ERROR, "zwp_linux_buffer_params_v1: import failed");
	wlf_signal_emit_mutable(&params->events.failed, NULL);
}

static const struct zwp_linux_buffer_params_v1_listener params_listener = {
	.created = params_created,
	.failed  = params_failed,
};

/* ── feedback helpers ────────────────────────────────────────────────────── */

static void feedback_unmap_table(struct wlf_zwp_linux_dmabuf_feedback_v1 *fb) {
	if (fb->format_table != NULL && fb->format_table_len > 0) {
		munmap(fb->format_table,
			fb->format_table_len * sizeof(struct wlf_dmabuf_format_entry));
		fb->format_table = NULL;
		fb->format_table_len = 0;
	}
	if (fb->_fmt_table_fd >= 0) {
		close(fb->_fmt_table_fd);
		fb->_fmt_table_fd = -1;
	}
}

static void feedback_free_tranches(struct wlf_zwp_linux_dmabuf_feedback_v1 *fb) {
	for (size_t i = 0; i < fb->n_tranches; i++) {
		free(fb->tranches[i].indices);
	}
	free(fb->tranches);
	fb->tranches     = NULL;
	fb->n_tranches   = 0;
	fb->_tranches_cap = 0;
}

static void feedback_clear_pending(struct wlf_zwp_linux_dmabuf_feedback_v1 *fb) {
	free(fb->_pending.indices);
	memset(&fb->_pending, 0, sizeof(fb->_pending));
}

/* ── feedback listeners ──────────────────────────────────────────────────── */

static void feedback_done(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *wp_feedback) {
	WLF_UNUSED(wp_feedback);
	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb = data;
	wlf_signal_emit_mutable(&fb->events.done, fb);
}

static void feedback_format_table(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *wp_feedback,
		int32_t fd, uint32_t size) {
	WLF_UNUSED(wp_feedback);
	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb = data;

	feedback_unmap_table(fb);
	feedback_free_tranches(fb);

	size_t n = size / sizeof(struct wlf_dmabuf_format_entry);
	void *ptr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) {
		wlf_log_errno(WLF_ERROR,
			"Failed to mmap zwp_linux_dmabuf_feedback_v1 format table");
		close(fd);
		return;
	}

	fb->format_table     = ptr;
	fb->format_table_len = n;
	fb->_fmt_table_fd    = fd;
}

static void feedback_main_device(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *wp_feedback,
		struct wl_array *device) {
	WLF_UNUSED(wp_feedback);
	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb = data;

	if (device->size >= sizeof(uint64_t)) {
		memcpy(&fb->main_device, device->data, sizeof(uint64_t));
	}
}

static void feedback_tranche_done(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *wp_feedback) {
	WLF_UNUSED(wp_feedback);
	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb = data;

	/* Grow tranches array if needed */
	if (fb->n_tranches >= fb->_tranches_cap) {
		size_t new_cap = fb->_tranches_cap == 0 ? 4 : fb->_tranches_cap * 2;
		struct wlf_zwp_linux_dmabuf_tranche *t =
			realloc(fb->tranches,
				new_cap * sizeof(struct wlf_zwp_linux_dmabuf_tranche));
		if (t == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to grow dmabuf tranche array");
			feedback_clear_pending(fb);
			return;
		}
		fb->tranches      = t;
		fb->_tranches_cap = new_cap;
	}

	fb->tranches[fb->n_tranches++] = fb->_pending;
	memset(&fb->_pending, 0, sizeof(fb->_pending));
}

static void feedback_tranche_target_device(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *wp_feedback,
		struct wl_array *device) {
	WLF_UNUSED(wp_feedback);
	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb = data;

	if (device->size >= sizeof(uint64_t)) {
		memcpy(&fb->_pending.target_device, device->data, sizeof(uint64_t));
	}
}

static void feedback_tranche_formats(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *wp_feedback,
		struct wl_array *indices) {
	WLF_UNUSED(wp_feedback);
	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb = data;

	size_t n = indices->size / sizeof(uint16_t);
	uint16_t *buf = malloc(indices->size);
	if (buf == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate tranche format index array");
		return;
	}

	memcpy(buf, indices->data, indices->size);
	free(fb->_pending.indices);
	fb->_pending.indices   = buf;
	fb->_pending.n_indices = n;
}

static void feedback_tranche_flags(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *wp_feedback,
		uint32_t flags) {
	WLF_UNUSED(wp_feedback);
	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb = data;
	fb->_pending.flags = flags;
}

static const struct zwp_linux_dmabuf_feedback_v1_listener feedback_listener = {
	.done                 = feedback_done,
	.format_table         = feedback_format_table,
	.main_device          = feedback_main_device,
	.tranche_done         = feedback_tranche_done,
	.tranche_target_device = feedback_tranche_target_device,
	.tranche_formats      = feedback_tranche_formats,
	.tranche_flags        = feedback_tranche_flags,
};

/* ── feedback alloc / init ───────────────────────────────────────────────── */

static struct wlf_zwp_linux_dmabuf_feedback_v1 *alloc_feedback(
		struct zwp_linux_dmabuf_feedback_v1 *base) {
	if (base == NULL) {
		return NULL;
	}

	struct wlf_zwp_linux_dmabuf_feedback_v1 *fb =
		calloc(1, sizeof(struct wlf_zwp_linux_dmabuf_feedback_v1));
	if (fb == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_zwp_linux_dmabuf_feedback_v1");
		zwp_linux_dmabuf_feedback_v1_destroy(base);
		return NULL;
	}

	fb->base         = base;
	fb->_fmt_table_fd = -1;
	wlf_signal_init(&fb->events.done);
	wlf_signal_init(&fb->events.destroy);

	zwp_linux_dmabuf_feedback_v1_add_listener(base, &feedback_listener, fb);
	return fb;
}

/* ── global manager ──────────────────────────────────────────────────────── */

struct wlf_zwp_linux_dmabuf_v1 *wlf_zwp_linux_dmabuf_v1_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_zwp_linux_dmabuf_v1 *dmabuf =
		malloc(sizeof(struct wlf_zwp_linux_dmabuf_v1));
	if (dmabuf == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_zwp_linux_dmabuf_v1");
		return NULL;
	}

	dmabuf->base = NULL;
	wlf_signal_init(&dmabuf->events.destroy);

	uint32_t bind_version = version;
	if (version > (uint32_t)zwp_linux_dmabuf_v1_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server zwp_linux_dmabuf_v1 version %u is higher than client "
			"version %u, using client version",
			version, (uint32_t)zwp_linux_dmabuf_v1_interface.version);
		bind_version = (uint32_t)zwp_linux_dmabuf_v1_interface.version;
	}

	dmabuf->base = wl_registry_bind(wl_registry, name,
		&zwp_linux_dmabuf_v1_interface, bind_version);
	if (dmabuf->base == NULL) {
		wlf_log(WLF_ERROR,
			"Failed to bind zwp_linux_dmabuf_v1 interface (name: %u)", name);
		free(dmabuf);
		return NULL;
	}

	dmabuf->version = bind_version;

	wlf_log(WLF_DEBUG,
		"Successfully bound zwp_linux_dmabuf_v1 (name: %u, version: %u)",
		name, bind_version);

	return dmabuf;
}

void wlf_zwp_linux_dmabuf_v1_destroy(struct wlf_zwp_linux_dmabuf_v1 *dmabuf) {
	if (dmabuf == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&dmabuf->events.destroy, dmabuf);

	if (dmabuf->base != NULL) {
		zwp_linux_dmabuf_v1_destroy(dmabuf->base);
		dmabuf->base = NULL;
	}

	free(dmabuf);
}

struct wlf_zwp_linux_buffer_params_v1 *
wlf_zwp_linux_dmabuf_v1_create_params(
		struct wlf_zwp_linux_dmabuf_v1 *dmabuf) {
	assert(dmabuf != NULL);
	assert(dmabuf->base != NULL);

	struct wlf_zwp_linux_buffer_params_v1 *params =
		malloc(sizeof(struct wlf_zwp_linux_buffer_params_v1));
	if (params == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_zwp_linux_buffer_params_v1");
		return NULL;
	}

	params->base = zwp_linux_dmabuf_v1_create_params(dmabuf->base);
	if (params->base == NULL) {
		wlf_log(WLF_ERROR,
			"zwp_linux_dmabuf_v1_create_params() returned NULL");
		free(params);
		return NULL;
	}

	wlf_signal_init(&params->events.created);
	wlf_signal_init(&params->events.failed);
	wlf_signal_init(&params->events.destroy);

	zwp_linux_buffer_params_v1_add_listener(params->base,
		&params_listener, params);

	return params;
}

struct wlf_zwp_linux_dmabuf_feedback_v1 *
wlf_zwp_linux_dmabuf_v1_get_default_feedback(
		struct wlf_zwp_linux_dmabuf_v1 *dmabuf) {
	assert(dmabuf != NULL);
	assert(dmabuf->base != NULL);

	if (dmabuf->version < 4) {
		wlf_log(WLF_ERROR,
			"zwp_linux_dmabuf_v1 version %u < 4: "
			"get_default_feedback not supported", dmabuf->version);
		return NULL;
	}

	struct zwp_linux_dmabuf_feedback_v1 *base =
		zwp_linux_dmabuf_v1_get_default_feedback(dmabuf->base);

	return alloc_feedback(base);
}

struct wlf_zwp_linux_dmabuf_feedback_v1 *
wlf_zwp_linux_dmabuf_v1_get_surface_feedback(
		struct wlf_zwp_linux_dmabuf_v1 *dmabuf, struct wl_surface *surface) {
	assert(dmabuf != NULL);
	assert(dmabuf->base != NULL);
	assert(surface != NULL);

	if (dmabuf->version < 4) {
		wlf_log(WLF_ERROR,
			"zwp_linux_dmabuf_v1 version %u < 4: "
			"get_surface_feedback not supported", dmabuf->version);
		return NULL;
	}

	struct zwp_linux_dmabuf_feedback_v1 *base =
		zwp_linux_dmabuf_v1_get_surface_feedback(dmabuf->base, surface);

	return alloc_feedback(base);
}

/* ── params requests ─────────────────────────────────────────────────────── */

void wlf_zwp_linux_buffer_params_v1_add(
		struct wlf_zwp_linux_buffer_params_v1 *params,
		int fd, uint32_t plane_idx, uint32_t offset, uint32_t stride,
		uint32_t modifier_hi, uint32_t modifier_lo) {
	assert(params != NULL);
	assert(params->base != NULL);

	zwp_linux_buffer_params_v1_add(params->base, fd, plane_idx,
		offset, stride, modifier_hi, modifier_lo);
}

void wlf_zwp_linux_buffer_params_v1_create(
		struct wlf_zwp_linux_buffer_params_v1 *params,
		int32_t width, int32_t height, uint32_t format, uint32_t flags) {
	assert(params != NULL);
	assert(params->base != NULL);

	zwp_linux_buffer_params_v1_create(params->base,
		width, height, format, flags);
}

struct wl_buffer *wlf_zwp_linux_buffer_params_v1_create_immed(
		struct wlf_zwp_linux_buffer_params_v1 *params,
		int32_t width, int32_t height, uint32_t format, uint32_t flags) {
	assert(params != NULL);
	assert(params->base != NULL);

	return zwp_linux_buffer_params_v1_create_immed(params->base,
		width, height, format, flags);
}

void wlf_zwp_linux_buffer_params_v1_destroy(
		struct wlf_zwp_linux_buffer_params_v1 *params) {
	if (params == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&params->events.destroy, params);

	if (params->base != NULL) {
		zwp_linux_buffer_params_v1_destroy(params->base);
		params->base = NULL;
	}

	free(params);
}

void wlf_zwp_linux_dmabuf_feedback_v1_destroy(
		struct wlf_zwp_linux_dmabuf_feedback_v1 *feedback) {
	if (feedback == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&feedback->events.destroy, feedback);

	feedback_free_tranches(feedback);
	feedback_clear_pending(feedback);
	feedback_unmap_table(feedback);

	if (feedback->base != NULL) {
		zwp_linux_dmabuf_feedback_v1_destroy(feedback->base);
		feedback->base = NULL;
	}

	free(feedback);
}
