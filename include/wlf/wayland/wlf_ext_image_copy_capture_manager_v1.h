/**
 * @file        wlf_ext_image_copy_capture_manager_v1.h
 * @brief       ext-image-copy-capture-v1 client wrappers.
 * @details     Provides wlframe wrappers for image-copy capture managers,
 *              sessions, frames, and pointer-cursor sessions.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_H
#define WLF_EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_H

#include <stdbool.h>
#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct ext_image_capture_source_v1;
struct ext_image_copy_capture_manager_v1;
struct wl_buffer;
struct wl_pointer;
struct wl_registry;
struct wlf_ext_image_capture_source_v1;

/**
 * @brief Options used when creating an image-copy capture session.
 *
 * Values are protocol-defined bit flags and may be combined by callers.
 */
enum wlf_ext_image_copy_capture_options_v1 {
	WLF_EXT_IMAGE_COPY_CAPTURE_OPTIONS_V1_PAINT_CURSORS = 1, /**< Include cursors. */
};

/**
 * @brief Failure reasons reported by a capture frame.
 *
 * The value is valid when the frame's failed signal is emitted.
 */
enum wlf_ext_image_copy_capture_frame_v1_failure_reason {
	WLF_EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_UNKNOWN = 0, /**< Unknown failure. */
	WLF_EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_BUFFER_CONSTRAINTS = 1, /**< Buffer constraints failed. */
	WLF_EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED = 2, /**< Capture was stopped. */
};

/**
 * @brief Captured frame wrapper and metadata.
 *
 * The metadata fields are populated by the compositor before the ready or
 * failed signal is emitted.
 */
struct wlf_ext_image_copy_capture_frame_v1 {
	struct ext_image_copy_capture_frame_v1 *base; /**< Protocol object. */

	uint32_t transform; /**< Buffer transform reported by the compositor. */
	uint32_t tv_sec_hi; /**< High 32 bits of the presentation timestamp. */
	uint32_t tv_sec_lo; /**< Low 32 bits of the presentation timestamp. */
	uint32_t tv_nsec; /**< Nanoseconds part of the presentation timestamp. */
	uint32_t failed_reason; /**< Protocol failure reason, when failed. */

	struct {
		struct wlf_signal ready;   /**< Payload: wlf_ext_image_copy_capture_frame_v1. */
		struct wlf_signal failed;  /**< Payload: wlf_ext_image_copy_capture_frame_v1. */
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Image-copy capture session wrapper.
 *
 * The buffer dimensions and wl_shm format are updated by the compositor and
 * are used when allocating frame buffers.
 */
struct wlf_ext_image_copy_capture_session_v1 {
	struct ext_image_copy_capture_session_v1 *base; /**< Protocol object. */

	uint32_t buffer_width; /**< Width required for capture buffers. */
	uint32_t buffer_height; /**< Height required for capture buffers. */
	uint32_t shm_format; /**< wl_shm format required for capture buffers. */

	struct {
		struct wlf_signal done;    /**< Payload: wlf_ext_image_copy_capture_session_v1. */
		struct wlf_signal stopped; /**< Payload: wlf_ext_image_copy_capture_session_v1. */
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Pointer-cursor capture session wrapper.
 *
 * Cursor position and hotspot fields are updated before the corresponding
 * signal is emitted.
 */
struct wlf_ext_image_copy_capture_cursor_session_v1 {
	struct ext_image_copy_capture_cursor_session_v1 *base; /**< Protocol object. */

	bool entered; /**< Whether the cursor is currently over the source. */
	int32_t x; /**< Cursor x coordinate in source pixels. */
	int32_t y; /**< Cursor y coordinate in source pixels. */
	int32_t hotspot_x; /**< Cursor hotspot x coordinate. */
	int32_t hotspot_y; /**< Cursor hotspot y coordinate. */

	struct {
		struct wlf_signal enter;    /**< Payload: wlf_ext_image_copy_capture_cursor_session_v1. */
		struct wlf_signal leave;    /**< Payload: wlf_ext_image_copy_capture_cursor_session_v1. */
		struct wlf_signal position; /**< Payload: wlf_ext_image_copy_capture_cursor_session_v1. */
		struct wlf_signal hotspot;  /**< Payload: wlf_ext_image_copy_capture_cursor_session_v1. */
		struct wlf_signal destroy;  /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Image-copy capture manager wrapper.
 *
 * The manager creates ordinary and pointer-cursor capture sessions.
 */
struct wlf_ext_image_copy_capture_manager_v1 {
	struct ext_image_copy_capture_manager_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Creates an image-copy capture manager wrapper.
 * @param registry Wayland registry used to bind the manager.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_ext_image_copy_capture_manager_v1 *
wlf_ext_image_copy_capture_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys an image-copy capture manager.
 * @param manager Manager to destroy.
 */
void wlf_ext_image_copy_capture_manager_v1_destroy(
	struct wlf_ext_image_copy_capture_manager_v1 *manager);

/**
 * @brief Creates a capture session for @p source.
 * @param manager Image-copy capture manager.
 * @param source Image-capture source.
 * @param options Session creation options.
 * @return Newly allocated capture session, or NULL on failure.
 */
struct wlf_ext_image_copy_capture_session_v1 *
wlf_ext_image_copy_capture_manager_v1_create_session(
	struct wlf_ext_image_copy_capture_manager_v1 *manager,
	struct wlf_ext_image_capture_source_v1 *source,
	enum wlf_ext_image_copy_capture_options_v1 options);

/**
 * @brief Creates a cursor session for @p source and @p pointer.
 * @param manager Image-copy capture manager.
 * @param source Image-capture source.
 * @param pointer Pointer whose cursor should be captured.
 * @return Newly allocated cursor session, or NULL on failure.
 */
struct wlf_ext_image_copy_capture_cursor_session_v1 *
wlf_ext_image_copy_capture_manager_v1_create_pointer_cursor_session(
	struct wlf_ext_image_copy_capture_manager_v1 *manager,
	struct wlf_ext_image_capture_source_v1 *source,
	struct wl_pointer *pointer);

/**
 * @brief Creates a frame request in a capture session.
 * @param session Capture session receiving the request.
 * @return Newly allocated frame, or NULL on failure.
 */
struct wlf_ext_image_copy_capture_frame_v1 *
wlf_ext_image_copy_capture_session_v1_create_frame(
	struct wlf_ext_image_copy_capture_session_v1 *session);

/**
 * @brief Destroys a capture session.
 * @param session Capture session to destroy.
 */
void wlf_ext_image_copy_capture_session_v1_destroy(
	struct wlf_ext_image_copy_capture_session_v1 *session);

/**
 * @brief Attaches a destination buffer to a frame request.
 * @param frame Frame receiving the buffer.
 * @param buffer Destination Wayland buffer.
 */
void wlf_ext_image_copy_capture_frame_v1_attach_buffer(
	struct wlf_ext_image_copy_capture_frame_v1 *frame,
	struct wl_buffer *buffer);

/**
 * @brief Marks a damaged rectangle in the destination buffer.
 * @param frame Frame receiving the damage rectangle.
 * @param x Rectangle x coordinate.
 * @param y Rectangle y coordinate.
 * @param width Rectangle width.
 * @param height Rectangle height.
 */
void wlf_ext_image_copy_capture_frame_v1_damage_buffer(
	struct wlf_ext_image_copy_capture_frame_v1 *frame,
	int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Requests capture of the configured frame.
 * @param frame Frame to capture.
 */
void wlf_ext_image_copy_capture_frame_v1_capture(
	struct wlf_ext_image_copy_capture_frame_v1 *frame);

/**
 * @brief Destroys a capture frame.
 * @param frame Frame to destroy.
 */
void wlf_ext_image_copy_capture_frame_v1_destroy(
	struct wlf_ext_image_copy_capture_frame_v1 *frame);

/**
 * @brief Returns the ordinary capture session associated with a cursor session.
 * @param cursor_session Cursor session to inspect.
 * @return Associated capture session, or NULL when unavailable.
 */
struct wlf_ext_image_copy_capture_session_v1 *
wlf_ext_image_copy_capture_cursor_session_v1_get_capture_session(
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cursor_session);

/**
 * @brief Destroys a pointer-cursor capture session.
 * @param cursor_session Cursor session to destroy.
 */
void wlf_ext_image_copy_capture_cursor_session_v1_destroy(
	struct wlf_ext_image_copy_capture_cursor_session_v1 *cursor_session);

#endif /* WLF_EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_H */
