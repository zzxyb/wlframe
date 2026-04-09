/**
 * @file        wlf_file_stream.h
 * @brief       File-backed stream implementation for wlframe.
 * @details     This file declares the file stream type and helper APIs for
 *              creating file-backed streams, type checking, and conversion
 *              from the generic stream base object.
 * @author      YaoBing Xiao
 * @date        2026-04-08
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-08, initial version\n
 */

#ifndef STREAM_WLF_FILE_STREAM_H
#define STREAM_WLF_FILE_STREAM_H

#include "wlf/stream/wlf_stream.h"

#include <stdio.h>
#include <stdbool.h>

/**
 * @brief File stream object.
 */
struct wlf_file_stream {
	struct wlf_stream base; /**< Base stream object. */
	FILE *file;             /**< Underlying C stdio file handle. */
	bool owns_file;         /**< Whether this stream closes @ref file on destroy. */
};

/**
 * @brief Creates a stream from an existing FILE handle.
 * @param file Existing file handle.
 * @param take_ownership Whether the stream should close @p file on destroy.
 * @return Stream object, or NULL on failure.
 */
struct wlf_stream *wlf_file_stream_create_from_file(FILE *file,
	bool take_ownership);

/**
 * @brief Opens file and creates a file stream.
 * @param filename Path to file.
 * @param mode fopen mode string.
 * @return Stream object, or NULL on failure.
 */
struct wlf_stream *wlf_file_stream_open(const char *filename,
	const char *mode);

/**
 * @brief Checks whether stream is a file stream.
 * @param stream Stream object.
 * @return true if file stream, otherwise false.
 */
bool wlf_stream_is_file(const struct wlf_stream *stream);

/**
 * @brief Casts base stream to file stream.
 * @param stream Base stream object.
 * @return File stream object.
 */
struct wlf_file_stream *wlf_file_stream_from_stream(struct wlf_stream *stream);

#endif // STREAM_WLF_FILE_STREAM_H
