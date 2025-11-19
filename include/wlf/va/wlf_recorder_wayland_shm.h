/**
 * @file        wlf_recorder_wayland_shm.h
 * @brief       Wayland SHM backend interface for video recorder.
 * @details     Public interface for Wayland shared memory buffer capture.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#ifndef WLF_RECORDER_WAYLAND_SHM_H
#define WLF_RECORDER_WAYLAND_SHM_H

#include <stdbool.h>
#include <stdint.h>
#include "wlf/va/wlf_recorder_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wlf_video_recorder;

/**
 * @brief Creates a new Wayland SHM backend instance.
 *
 * @param recorder The recorder instance (can be NULL during creation).
 * @param frame_callback Callback function for captured frames.
 * @param user_data User data passed to callback.
 * @return New backend instance, or NULL on failure.
 */
struct wlf_recorder_backend *wlf_recorder_wayland_shm_backend_create(
	struct wlf_video_recorder *recorder,
	wlf_recorder_frame_callback frame_callback,
	void *user_data);

/**
 * @brief Submits a Wayland SHM buffer for recording.
 *
 * This function copies the SHM data and converts it to a format suitable
 * for the video encoder.
 *
 * @param backend Backend instance.
 * @param shm_data Pointer to shared memory data.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 * @param stride Buffer stride (bytes per row).
 * @param format wl_shm_format format (e.g., WL_SHM_FORMAT_ARGB8888).
 * @param timestamp_us Frame timestamp in microseconds.
 * @return true on success, false on failure.
 */
bool wlf_recorder_wayland_shm_backend_submit_buffer(
	struct wlf_recorder_backend *backend,
	const void *shm_data,
	uint32_t width,
	uint32_t height,
	uint32_t stride,
	uint32_t format,
	uint64_t timestamp_us);

/**
 * @brief Submits a Wayland SHM buffer using file descriptor.
 *
 * This function maps the SHM buffer and submits it for recording.
 * The buffer is automatically unmapped after submission.
 *
 * @param backend Backend instance.
 * @param shm_fd Shared memory file descriptor.
 * @param offset Offset in the shared memory.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 * @param stride Buffer stride (bytes per row).
 * @param format wl_shm_format format.
 * @param timestamp_us Frame timestamp in microseconds.
 * @return true on success, false on failure.
 */
bool wlf_recorder_wayland_shm_backend_submit_buffer_fd(
	struct wlf_recorder_backend *backend,
	int shm_fd,
	uint32_t offset,
	uint32_t width,
	uint32_t height,
	uint32_t stride,
	uint32_t format,
	uint64_t timestamp_us);

#ifdef __cplusplus
}
#endif

#endif /* WLF_RECORDER_WAYLAND_SHM_H */
