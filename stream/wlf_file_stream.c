#include "wlf/stream/wlf_file_stream.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void stream_destroy(struct wlf_stream *stream) {
	struct wlf_file_stream *file_stream = wlf_file_stream_from_stream(stream);
	if (file_stream->file != NULL) {
		if (file_stream->owns_file) {
			fclose(file_stream->file);
		}
	}

	free(file_stream);
}

static int stream_read(struct wlf_stream *stream, void *buffer,
		size_t size, size_t *bytes_read) {
	struct wlf_file_stream *file_stream =
		wlf_file_stream_from_stream(stream);
	size_t result = fread(buffer, 1, size, file_stream->file);

	if (bytes_read != NULL) {
		*bytes_read = result;
	}

	if (result < size && ferror(file_stream->file)) {
		return WLF_STREAM_ERROR_READ_FAILED;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_write(struct wlf_stream *stream, const void *buffer,
		size_t size, size_t *bytes_written) {
	struct wlf_file_stream *file_stream =
		wlf_file_stream_from_stream(stream);
	size_t result = fwrite(buffer, 1, size, file_stream->file);

	if (bytes_written != NULL) {
		*bytes_written = result;
	}

	if (result < size) {
		return WLF_STREAM_ERROR_WRITE_FAILED;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_seek(struct wlf_stream *stream, long offset, int whence) {
	struct wlf_file_stream *file_stream =
		wlf_file_stream_from_stream(stream);

	if (fseek(file_stream->file, offset, whence) != 0) {
		return WLF_STREAM_ERROR_SEEK_FAILED;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_tell(struct wlf_stream *stream, long *position) {
	struct wlf_file_stream *file_stream =
		wlf_file_stream_from_stream(stream);
	long pos = ftell(file_stream->file);

	if (pos == -1L) {
		return WLF_STREAM_ERROR_SEEK_FAILED;
	}

	if (position != NULL) {
		*position = pos;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_get_size(struct wlf_stream *stream, size_t *size) {
	struct wlf_file_stream *file_stream =
		wlf_file_stream_from_stream(stream);
	long current_pos = ftell(file_stream->file);
	if (current_pos == -1L) {
		return WLF_STREAM_ERROR_SEEK_FAILED;
	}

	if (fseek(file_stream->file, 0, SEEK_END) != 0) {
		return WLF_STREAM_ERROR_SEEK_FAILED;
	}

	long file_size = ftell(file_stream->file);
	if (file_size == -1L) {
		(void)fseek(file_stream->file, current_pos, SEEK_SET);
		return WLF_STREAM_ERROR_SEEK_FAILED;
	}

	if (fseek(file_stream->file, current_pos, SEEK_SET) != 0) {
		return WLF_STREAM_ERROR_SEEK_FAILED;
	}

	if (size != NULL) {
		*size = (size_t)file_size;
	}

	return WLF_STREAM_SUCCESS;
}

static int stream_flush(struct wlf_stream *stream) {
	struct wlf_file_stream *file_stream =
		wlf_file_stream_from_stream(stream);

	if (fflush(file_stream->file) != 0) {
		return WLF_STREAM_ERROR_WRITE_FAILED;
	}

	return WLF_STREAM_SUCCESS;
}

static const struct wlf_stream_impl file_stream_impl = {
	.destroy = stream_destroy,
	.read = stream_read,
	.write = stream_write,
	.seek = stream_seek,
	.tell = stream_tell,
	.get_size = stream_get_size,
	.flush = stream_flush,
};

struct wlf_stream *wlf_file_stream_create_from_file(FILE *file,
		bool take_ownership) {
	struct wlf_file_stream *stream =
		malloc(sizeof(struct wlf_file_stream));
	if (stream == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_file_stream");

		return NULL;
	}

	wlf_stream_init(&stream->base, &file_stream_impl);
	stream->file = file;
	stream->owns_file = take_ownership;

	return &stream->base;
}

struct wlf_stream *wlf_file_stream_open(const char *filename,
		const char *mode) {
	FILE *file = fopen(filename, mode);
	if (file == NULL) {
		wlf_log_errno(WLF_ERROR, "Open %s failed!", filename);

		return NULL;
	}

	return wlf_file_stream_create_from_file(file, true);
}

bool wlf_stream_is_file(const struct wlf_stream *stream) {
	return stream->impl == &file_stream_impl;
}

struct wlf_file_stream *wlf_file_stream_from_stream(struct wlf_stream *stream) {
	assert(stream->impl == &file_stream_impl);

	struct wlf_file_stream *file_stream =
		wlf_container_of(stream, file_stream, base);

	return file_stream;
}
