#include "wlf/buffer/wlf_buffer.h"

#include <assert.h>

void wlf_buffer_init(struct wlf_buffer *buffer,
	const struct wlf_buffer_impl *impl, int width, int height) {
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
}

void wlf_buffer_finish(struct wlf_buffer *buffer) {
	if (buffer == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&buffer->events.destroy, NULL);
	buffer->impl->destroy(buffer);
}

void wlf_buffer_lock(struct wlf_buffer *buffer) {
	buffer->n_locks++;
}

void wlf_buffer_unlock(struct wlf_buffer *buffer) {
	assert(buffer->n_locks > 0);
	buffer->n_locks--;
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
