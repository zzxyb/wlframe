#include "wlf/stream/wlf_mem_stream.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void stream_destroy(struct wlf_stream *stream) {
	struct wlf_mem_stream *mem_stream = wlf_mem_stream_from_stream(stream);
	if (mem_stream->owns_buffer && mem_stream->buffer != NULL) {
		free(mem_stream->buffer);
	}

	mem_stream->buffer = NULL;
	mem_stream->size = 0;
	mem_stream->capacity = 0;
	free(stream);
}

static int wlf_mem_stream_ensure_capacity(struct wlf_mem_stream *stream,
	size_t required_size) {
	if (!stream->is_expandable || required_size <= stream->capacity) {
		return WLF_STREAM_SUCCESS;
	}

	size_t new_capacity = stream->capacity;
	if (new_capacity == 0) {
		new_capacity = 1024;
	}

	while (new_capacity < required_size) {
		new_capacity *= 2;
	}

	uint8_t *new_buffer = realloc(stream->buffer, new_capacity);
	if (new_buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to realloc new_buffer");

		return WLF_STREAM_ERROR_OUT_OF_MEMORY;
	}

	stream->buffer = new_buffer;
	stream->capacity = new_capacity;

	return WLF_STREAM_SUCCESS;
}

static int stream_read(struct wlf_stream *stream, void *buffer,
		size_t size, size_t *bytes_read) {
	struct wlf_mem_stream *mem_stream = wlf_mem_stream_from_stream(stream);

	size_t available = (mem_stream->position < mem_stream->size) ?
		(mem_stream->size - mem_stream->position) : 0;
	size_t to_read = (size < available) ? size : available;

	if (to_read > 0) {
		memcpy(buffer, mem_stream->buffer + mem_stream->position, to_read);
		mem_stream->position += to_read;
	}

	if (bytes_read != NULL) {
		*bytes_read = to_read;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_write(struct wlf_stream *stream, const void *buffer,
		size_t size, size_t *bytes_written) {
	struct wlf_mem_stream *mem_stream = wlf_mem_stream_from_stream(stream);
	if (!mem_stream->is_writable) {
		return WLF_STREAM_ERROR_INVALID_OPERATION;
	}

	size_t required_size = mem_stream->position + size;
	int result = wlf_mem_stream_ensure_capacity(mem_stream, required_size);
	if (result != WLF_STREAM_SUCCESS) {
		return result;
	}

	memcpy(mem_stream->buffer + mem_stream->position, buffer, size);
	mem_stream->position += size;
	if (mem_stream->size < mem_stream->position) {
		mem_stream->size = mem_stream->position;
	}

	if (bytes_written != NULL) {
		*bytes_written = size;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_seek(struct wlf_stream *stream, long offset, int whence) {
	struct wlf_mem_stream *mem_stream = wlf_mem_stream_from_stream(stream);
	long new_position;

	switch (whence) {
	case SEEK_SET:
		new_position = offset;
		break;
	case SEEK_CUR:
		new_position = (long)mem_stream->position + offset;
		break;
	case SEEK_END:
		new_position = (long)mem_stream->size + offset;
		break;
	default:
		return WLF_STREAM_ERROR_INVALID_OPERATION;
	}

	if (new_position < 0) {
		return WLF_STREAM_ERROR_INVALID_POSITION;
	}

	mem_stream->position = (size_t)new_position;
	return WLF_STREAM_SUCCESS;
}

static int stream_tell(struct wlf_stream *stream, long *position) {
	struct wlf_mem_stream *mem_stream = wlf_mem_stream_from_stream(stream);
	if (position != NULL) {
		*position = (long)mem_stream->position;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_get_size(struct wlf_stream *stream, size_t *size) {
	struct wlf_mem_stream *mem_stream = wlf_mem_stream_from_stream(stream);
	if (size != NULL) {
		*size = mem_stream->size;
	}

	return WLF_STREAM_SUCCESS;
}

static const struct wlf_stream_impl mem_stream_impl_readonly = {
	.destroy = stream_destroy,
	.read = stream_read,
	.write = NULL,
	.seek = stream_seek,
	.tell = stream_tell,
	.get_size = stream_get_size,
	.flush = NULL,
};

static const struct wlf_stream_impl mem_stream_impl_rw = {
	.destroy = stream_destroy,
	.read = stream_read,
	.write = stream_write,
	.seek = stream_seek,
	.tell = stream_tell,
	.get_size = stream_get_size,
	.flush = NULL,
};

struct wlf_stream *wlf_mem_stream_create(void *buffer, size_t size,
	bool is_writable, bool take_ownership) {
	if (buffer == NULL || size == 0) {
		return NULL;
	}

	struct wlf_mem_stream *stream =
		malloc(sizeof(struct wlf_mem_stream));
	if (stream == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_mem_stream");

		return NULL;
	}

	wlf_stream_init(&stream->base,
		is_writable ? &mem_stream_impl_rw : &mem_stream_impl_readonly);
	stream->buffer = (uint8_t *)buffer;
	stream->size = size;
	stream->capacity = size;
	stream->position = 0;
	stream->is_writable = is_writable;
	stream->owns_buffer = take_ownership;
	stream->is_expandable = false;

	return &stream->base;
}

struct wlf_stream *wlf_mem_stream_create_empty(size_t initial_capacity) {
	if (initial_capacity == 0) {
		initial_capacity = 1024;
	}

	uint8_t *buffer = malloc(initial_capacity);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate buffer");

		return NULL;
	}

	struct wlf_mem_stream *stream =
		malloc(sizeof(struct wlf_mem_stream));
	if (stream == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_mem_stream");
		free(buffer);

		return NULL;
	}

	wlf_stream_init(&stream->base, &mem_stream_impl_rw);
	stream->buffer = buffer;
	stream->size = 0;
	stream->capacity = initial_capacity;
	stream->position = 0;
	stream->is_writable = true;
	stream->owns_buffer = true;
	stream->is_expandable = true;

	return &stream->base;
}

struct wlf_stream *wlf_mem_stream_create_from_data(const void *data,
	size_t size) {
	if (!data || size == 0) {
		return NULL;
	}

	uint8_t *buffer = malloc(size);
	if (!buffer) {
		return NULL;
	}
	memcpy(buffer, data, size);

	struct wlf_stream *stream =
		wlf_mem_stream_create(buffer, size, false, true);
	if (stream == NULL) {
		free(buffer);
		return NULL;
	}

	return stream;
}

int wlf_mem_stream_get_buffer(struct wlf_mem_stream *stream, void **buffer,
		size_t *size) {
	if (buffer != NULL) {
		*buffer = stream->buffer;
	}
	if (size != NULL) {
		*size = stream->size;
	}

	return WLF_STREAM_SUCCESS;
}

int wlf_mem_stream_detach_buffer(struct wlf_mem_stream *stream, void **buffer,
		size_t *size) {
	if (!stream->owns_buffer) {
		return WLF_STREAM_ERROR_INVALID_OPERATION;
	}

	if (buffer != NULL) {
		*buffer = stream->buffer;
	}

	if (size != NULL) {
		*size = stream->size;
	}

	stream->buffer = NULL;
	stream->size = 0;
	stream->capacity = 0;
	stream->owns_buffer = false;

	return WLF_STREAM_SUCCESS;
}

bool wlf_stream_is_mem(const struct wlf_stream *stream) {
	return stream->impl == &mem_stream_impl_readonly ||
		stream->impl == &mem_stream_impl_rw;
}

struct wlf_mem_stream *wlf_mem_stream_from_stream(struct wlf_stream *stream) {
	assert(wlf_stream_is_mem(stream));

	struct wlf_mem_stream *mem_stream =
		wlf_container_of(stream, mem_stream, base);

	return mem_stream;
}
