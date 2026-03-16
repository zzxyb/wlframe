#include "wlf/buffer/wlf_buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <stdint.h>

static void buffer_consider_destroy(struct wlf_buffer *buffer) {
	if (!buffer->dropped || buffer->n_locks > 0) {
		return;
	}

	assert(!buffer->accessing_data_ptr);

	buffer->impl->destroy(buffer);
}

static void readonly_data_buffer_destroy(struct wlf_buffer *wlf_buffer) {
	struct wlf_readonly_data_buffer *buffer =
		wlf_readonly_data_buffer_from_buffer(wlf_buffer);
	wlf_buffer_finish(wlf_buffer);
	free(buffer->saved_data);
	free(buffer);
}

static bool readonly_data_buffer_begin_data_ptr_access(struct wlf_buffer *wlf_buffer,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct wlf_readonly_data_buffer *buffer =
		wlf_readonly_data_buffer_from_buffer(wlf_buffer);
	if (buffer->data == NULL) {
		return false;
	}
	if (flags & WLF_BUFFER_DATA_PTR_ACCESS_WRITE) {
		return false;
	}
	*data = (void *)buffer->data;
	*format = buffer->format;
	*stride = buffer->stride;
	return true;
}

static void readonly_data_buffer_end_data_ptr_access(struct wlf_buffer *wlf_buffer) {
	WLF_UNUSED(wlf_buffer);
}

static const struct wlf_buffer_impl readonly_data_buffer_impl = {
	.destroy = readonly_data_buffer_destroy,
	.begin_data_ptr_access = readonly_data_buffer_begin_data_ptr_access,
	.end_data_ptr_access = readonly_data_buffer_end_data_ptr_access,
};

struct wlf_readonly_data_buffer *wlf_readonly_data_buffer_create(uint32_t format,
		size_t stride, uint32_t width, uint32_t height, const void *data) {
	struct wlf_readonly_data_buffer *buffer = malloc(sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}

	wlf_buffer_init(&buffer->base, &readonly_data_buffer_impl, width, height);

	buffer->data = data;
	buffer->format = format;
	buffer->stride = stride;
	buffer->saved_data = NULL;

	return buffer;
}

bool wlf_readonly_data_buffer_drop(struct wlf_readonly_data_buffer *buffer) {
		bool ok = true;

	if (buffer->base.n_locks > 0) {
		size_t size = buffer->stride * buffer->base.height;
		buffer->saved_data = malloc(size);
		if (buffer->saved_data == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate saved_data");
			ok = false;
			buffer->data = NULL;
		} else {
			memcpy(buffer->saved_data, buffer->data, size);
			buffer->data = buffer->saved_data;
		}
	}

	wlf_buffer_drop(&buffer->base);
	return ok;
}

bool wlf_buffer_is_readonly_data(const struct wlf_buffer *wlf_buffer) {
	return wlf_buffer->impl == &readonly_data_buffer_impl;
}

struct wlf_readonly_data_buffer *wlf_readonly_data_buffer_from_buffer(
		struct wlf_buffer *wlf_buffer) {
	if (!wlf_buffer_is_readonly_data(wlf_buffer)) {
		return NULL;
	}

	struct wlf_readonly_data_buffer *buffer = wlf_container_of(wlf_buffer, buffer, base);
	return buffer;
}

void wlf_buffer_init(struct wlf_buffer *buffer,
		const struct wlf_buffer_impl *impl, uint32_t width, uint32_t height) {
	assert(impl->destroy);
	if (impl->begin_data_ptr_access || impl->end_data_ptr_access) {
		assert(impl->begin_data_ptr_access && impl->end_data_ptr_access);
	}

	*buffer = (struct wlf_buffer){
		.impl = impl,
		.width = width,
		.height = height,
	};

	wlf_signal_init(&buffer->events.destroy);
	wlf_signal_init(&buffer->events.release);

	wlf_addon_set_init(&buffer->addons);
}

void wlf_buffer_finish(struct wlf_buffer *buffer) {
	wlf_signal_emit_mutable(&buffer->events.destroy, buffer);
	wlf_addon_set_finish(&buffer->addons);

	assert(wlf_linked_list_empty(&buffer->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&buffer->events.release.listener_list));
}

void wlf_buffer_drop(struct wlf_buffer *buffer) {
	if (buffer == NULL) {
		return;
	}

	assert(!buffer->dropped);
	buffer->dropped = true;
	buffer_consider_destroy(buffer);
}

struct wlf_buffer *wlf_buffer_lock(struct wlf_buffer *buffer) {
	buffer->n_locks++;

	return buffer;
}

void wlf_buffer_unlock(struct wlf_buffer *buffer) {
	assert(buffer->n_locks > 0);
	buffer->n_locks--;
	if (buffer->n_locks == 0) {
		wlf_signal_emit_mutable(&buffer->events.release, NULL);
	}

	buffer_consider_destroy(buffer);
}

bool wlf_buffer_is_opaque(struct wlf_buffer *buffer) {
	if (!buffer->impl->opaque_region) {
		return false;
	}

	return buffer->impl->opaque_region(buffer) != NULL;
}

bool wlf_buffer_begin_data_ptr_access(struct wlf_buffer *buffer, uint32_t flags,
		void **data, uint32_t *format, size_t *stride) {
	assert(!buffer->accessing_data_ptr);
	if (!buffer->impl->begin_data_ptr_access) {
		return false;
	}
	if (!buffer->impl->begin_data_ptr_access(buffer, flags, data, format, stride)) {
		return false;
	}
	buffer->accessing_data_ptr = true;
	return true;
}

void wlf_buffer_end_data_ptr_access(struct wlf_buffer *buffer) {
	assert(buffer->accessing_data_ptr);
	buffer->impl->end_data_ptr_access(buffer);
	buffer->accessing_data_ptr = false;
}
