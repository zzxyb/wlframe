#include "wlf/stream/wlf_stream.h"

#include <assert.h>
#include <stdlib.h>

void wlf_stream_destroy(struct wlf_stream *stream) {
	if (stream == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&stream->events.destroy, stream);

	assert(wlf_linked_list_empty(&stream->events.destroy.listener_list));

	if (stream->impl && stream->impl->destroy) {
		stream->impl->destroy(stream);
	} else {
		free(stream);
	}
}
void wlf_stream_init(struct wlf_stream *stream,
		const struct wlf_stream_impl *impl) {
	assert(impl);
	assert(impl->destroy);

	stream->impl = impl;

	wlf_signal_init(&stream->events.destroy);
}

int wlf_stream_read(struct wlf_stream *stream, void *buffer, size_t size,
		size_t *bytes_read) {
	return stream->impl->read(stream, buffer, size, bytes_read);
}

int wlf_stream_write(struct wlf_stream *stream, const void *buffer, size_t size,
		size_t *bytes_written) {
	return stream->impl->write(stream, buffer, size, bytes_written);
}

int wlf_stream_seek(struct wlf_stream *stream, long offset, int whence) {
	return stream->impl->seek(stream, offset, whence);
}

int wlf_stream_tell(struct wlf_stream *stream, long *position) {
	return stream->impl->tell(stream, position);
}

int wlf_stream_get_size(struct wlf_stream *stream, size_t *size) {
	return stream->impl->get_size(stream, size);
}

int wlf_stream_flush(struct wlf_stream *stream) {
	return stream->impl->flush(stream);
}

const char *wlf_stream_error_string(enum wlf_stream_error error) {
	switch (error) {
	case WLF_STREAM_SUCCESS:
		return "Success";
	case WLF_STREAM_ERROR_NULL_POINTER:
		return "Null pointer";
	case WLF_STREAM_ERROR_INVALID_OPERATION:
		return "Invalid operation";
	case WLF_STREAM_ERROR_OUT_OF_MEMORY:
		return "Out of memory";
	case WLF_STREAM_ERROR_READ_FAILED:
		return "Read failed";
	case WLF_STREAM_ERROR_WRITE_FAILED:
		return "Write failed";
	case WLF_STREAM_ERROR_SEEK_FAILED:
		return "Seek failed";
	case WLF_STREAM_ERROR_INVALID_POSITION:
		return "Invalid position";
	case WLF_STREAM_ERROR_FILE_NOT_FOUND:
		return "File not found";
	case WLF_STREAM_ERROR_PERMISSION_DENIED:
		return "Permission denied";
	case WLF_STREAM_ERROR_NETWORK_INIT_FAILED:
		return "Network initialization failed";
	case WLF_STREAM_ERROR_NETWORK_CONNECTION_FAILED:
		return "Network connection failed";
	case WLF_STREAM_ERROR_NETWORK_SEND_FAILED:
		return "Network send failed";
	case WLF_STREAM_ERROR_NETWORK_RECV_FAILED:
		return "Network receive failed";
	case WLF_STREAM_ERROR_NETWORK_TIMEOUT:
		return "Network timeout";
	case WLF_STREAM_ERROR_NETWORK_HOST_NOT_FOUND:
		return "Host not found";
	case WLF_STREAM_ERROR_NETWORK_DISCONNECTED:
		return "Network disconnected";
	default:
		return "Unknown error";
	}
}

bool wlf_stream_is_readable(struct wlf_stream *stream) {
	return stream->impl && stream->impl->read;
}

bool wlf_stream_is_writable(struct wlf_stream *stream) {
	return stream->impl && stream->impl->write;
}
