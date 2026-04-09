/**
 * @file        wlf_stream.h
 * @brief       Stream abstraction interface for wlframe.
 * @details     This file defines the base stream interface used by wlframe.
 *              It provides common stream operations (read/write/seek/tell/flush),
 *              a virtual method table for backend implementations, and lifecycle
 *              helpers for stream initialization and destruction.
 * @author      YaoBing Xiao
 * @date        2026-04-08
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-08, initial version\n
 */

#ifndef STREAM_WLF_STREAM_H
#define STREAM_WLF_STREAM_H

#include "wlf/utils/wlf_signal.h"

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Stream operation status codes.
 */
enum wlf_stream_error {
    WLF_STREAM_SUCCESS = 0,                /**< Operation succeeded. */
    WLF_STREAM_ERROR_NULL_POINTER,         /**< Required pointer argument is NULL. */
    WLF_STREAM_ERROR_INVALID_OPERATION,    /**< Operation is not supported for this stream. */
    WLF_STREAM_ERROR_OUT_OF_MEMORY,        /**< Memory allocation failed. */
    WLF_STREAM_ERROR_READ_FAILED,          /**< Read operation failed. */
    WLF_STREAM_ERROR_WRITE_FAILED,         /**< Write operation failed. */
    WLF_STREAM_ERROR_SEEK_FAILED,          /**< Seek operation failed. */
    WLF_STREAM_ERROR_INVALID_POSITION,     /**< Target position is invalid. */
    WLF_STREAM_ERROR_FILE_NOT_FOUND,       /**< File was not found. */
    WLF_STREAM_ERROR_PERMISSION_DENIED,    /**< Insufficient permissions for requested action. */
    WLF_STREAM_ERROR_NETWORK_INIT_FAILED,  /**< Network subsystem initialization failed. */
    WLF_STREAM_ERROR_NETWORK_CONNECTION_FAILED, /**< Network connection setup failed. */
    WLF_STREAM_ERROR_NETWORK_SEND_FAILED,  /**< Network send operation failed. */
    WLF_STREAM_ERROR_NETWORK_RECV_FAILED,  /**< Network receive operation failed. */
    WLF_STREAM_ERROR_NETWORK_TIMEOUT,      /**< Network operation timed out. */
    WLF_STREAM_ERROR_NETWORK_HOST_NOT_FOUND, /**< Host lookup failed. */
    WLF_STREAM_ERROR_NETWORK_DISCONNECTED, /**< Network peer is disconnected. */
};

struct wlf_stream;

/**
 * @brief Virtual methods for stream operations.
 *
 * Backend-specific stream implementations must provide these callbacks.
 */
struct wlf_stream_impl {
    /**
     * @brief Releases all resources owned by this stream object.
     * @param stream Stream instance.
     */
    void (*destroy)(struct wlf_stream *stream);
    /**
     * @brief Reads bytes from stream.
     */
	int (*read)(struct wlf_stream *stream, void *buffer, size_t size, size_t *bytes_read);
    /**
     * @brief Writes bytes to stream.
     */
	int (*write)(struct wlf_stream *stream, const void *buffer, size_t size, size_t *bytes_written);
    /**
     * @brief Moves stream cursor.
     */
	int (*seek)(struct wlf_stream *stream, long offset, int whence);
    /**
     * @brief Gets current stream cursor position.
     */
	int (*tell)(struct wlf_stream *stream, long *position);
    /**
     * @brief Gets stream total size in bytes.
     */
	int (*get_size)(struct wlf_stream *stream, size_t *size);
    /**
     * @brief Flushes pending buffered writes.
     */
	int (*flush)(struct wlf_stream *stream);
};

/**
 * @brief Base stream object.
 *
 * Concrete stream types must place this structure as their first member.
 */
struct wlf_stream {
    const struct wlf_stream_impl *impl; /**< Virtual method table. */

	struct {
		struct wlf_signal destroy;  /**< Signal emitted when the stream is destroyed. */
	} events;

    void *data; /**< Backend-specific user data (opaque pointer). */
};

/**
 * @brief Destroys a stream.
 * @param stream Stream to destroy.
 */
void wlf_stream_destroy(struct wlf_stream *stream);

/**
 * @brief Initializes a stream object.
 * @param stream Stream to initialize.
 * @param impl Stream implementation methods.
 */
void wlf_stream_init(struct wlf_stream *stream,
	const struct wlf_stream_impl *impl);

/**
 * @brief Reads data from stream.
 */
int wlf_stream_read(struct wlf_stream *stream, void *buffer, size_t size,
	size_t *bytes_read);

/**
 * @brief Writes data to stream.
 */
int wlf_stream_write(struct wlf_stream *stream, const void *buffer,
	size_t size, size_t *bytes_written);

/**
 * @brief Seeks stream cursor.
 */
int wlf_stream_seek(struct wlf_stream *stream, long offset, int whence);

/**
 * @brief Gets current stream cursor position.
 */
int wlf_stream_tell(struct wlf_stream *stream, long *position);

/**
 * @brief Gets stream size in bytes.
 */
int wlf_stream_get_size(struct wlf_stream *stream, size_t *size);

/**
 * @brief Flushes stream writes.
 */
int wlf_stream_flush(struct wlf_stream *stream);

/**
 * @brief Converts stream error code to readable string.
 */
const char *wlf_stream_error_string(enum wlf_stream_error error);

/**
 * @brief Returns whether stream supports reading.
 */
bool wlf_stream_is_readable(struct wlf_stream *stream);

/**
 * @brief Returns whether stream supports writing.
 */
bool wlf_stream_is_writable(struct wlf_stream *stream);

#endif // STREAM_WLF_STREAM_H
