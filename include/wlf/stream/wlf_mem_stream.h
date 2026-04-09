/**
 * @file        wlf_mem_stream.h
 * @brief       Memory-backed stream implementation for wlframe.
 * @details     This file declares the in-memory stream type and APIs for
 *              creating read-only or writable memory streams, accessing
 *              internal buffers, and converting from generic stream objects.
 * @author      YaoBing Xiao
 * @date        2026-04-08
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-08, initial version\n
 */

#ifndef STREAM_WLF_MEM_STREAM_H
#define STREAM_WLF_MEM_STREAM_H

#include "wlf/stream/wlf_stream.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Memory stream object.
 */
struct wlf_mem_stream {
	struct wlf_stream base; /**< Base stream object. */
	uint8_t *buffer;        /**< Backing memory buffer. */
	size_t size;            /**< Valid data length in buffer. */
	size_t capacity;        /**< Allocated buffer capacity. */
	size_t position;        /**< Current stream cursor position. */
	bool is_writable;       /**< Whether writes are allowed. */
	bool owns_buffer;       /**< Whether stream owns buffer lifetime. */
	bool is_expandable;     /**< Whether buffer can grow automatically. */
};

/**
 * @brief Creates a stream from user-provided memory.
 * @param buffer Backing memory.
 * @param size Buffer size in bytes.
 * @param is_writable Whether stream permits writes.
 * @param take_ownership Whether stream owns buffer lifetime.
 * @return Stream object, or NULL on failure.
 */
struct wlf_stream *wlf_mem_stream_create(void *buffer, size_t size,
	bool is_writable, bool take_ownership);

/**
 * @brief Creates an empty writable expandable memory stream.
 * @param initial_capacity Initial allocation size.
 * @return Stream object, or NULL on failure.
 */
struct wlf_stream *wlf_mem_stream_create_empty(size_t initial_capacity);

/**
 * @brief Creates read-only stream by copying input data.
 * @param data Source data.
 * @param size Data size in bytes.
 * @return Stream object, or NULL on failure.
 */
struct wlf_stream *wlf_mem_stream_create_from_data(const void *data,
	size_t size);

/**
 * @brief Gets underlying memory buffer and data size.
 * @param stream Memory stream object.
 * @param buffer Output buffer pointer.
 * @param size Output size pointer.
 * @return Stream error code.
 */
int wlf_mem_stream_get_buffer(struct wlf_mem_stream *stream, void **buffer,
	size_t *size);

/**
 * @brief Detaches and returns underlying memory buffer.
 * @param stream Memory stream object.
 * @param buffer Output detached buffer pointer.
 * @param size Output detached data size.
 * @return Stream error code.
 */
int wlf_mem_stream_detach_buffer(struct wlf_mem_stream *stream, void **buffer,
	size_t *size);

/**
 * @brief Checks whether stream is a memory stream.
 * @param stream Stream object.
 * @return true if memory stream, otherwise false.
 */
bool wlf_stream_is_mem(const struct wlf_stream *stream);

/**
 * @brief Casts base stream to memory stream.
 * @param stream Base stream object.
 * @return Memory stream object.
 */
struct wlf_mem_stream *wlf_mem_stream_from_stream(struct wlf_stream *stream);

#endif // STREAM_WLF_MEM_STREAM_H
