/**
 * @file        wlf_recorder_backend.h
 * @brief       Internal backend interface for video recorder.
 * @details     Defines the backend abstraction for different capture sources.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#ifndef WLF_RECORDER_BACKEND_H
#define WLF_RECORDER_BACKEND_H

#include <stdbool.h>
#include <stdint.h>
#include "wlf/dmabuf/wlf_dmabuf.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wlf_recorder_backend;
struct wlf_video_recorder;

/**
 * @struct wlf_recorder_frame
 * @brief Captured frame data.
 */
struct wlf_recorder_frame {
	struct wlf_dmabuf_attributes dmabuf;  /**< DMA-BUF attributes */
	uint64_t timestamp_us;                /**< Timestamp in microseconds */
	uint32_t width;                       /**< Frame width */
	uint32_t height;                      /**< Frame height */
	uint32_t format;                      /**< DRM fourcc format */
	void *user_data;                      /**< User data */
};

/**
 * @typedef wlf_recorder_frame_callback
 * @brief Callback function called when a new frame is captured.
 *
 * @param recorder The recorder instance.
 * @param frame The captured frame.
 * @param user_data User data provided during backend creation.
 */
typedef void (*wlf_recorder_frame_callback)(
	struct wlf_video_recorder *recorder,
	const struct wlf_recorder_frame *frame,
	void *user_data);

/**
 * @struct wlf_recorder_backend_impl
 * @brief Backend implementation interface.
 */
struct wlf_recorder_backend_impl {
	const char *name;  /**< Backend name for identification */

	/**
	 * @brief Starts capture.
	 * @return true on success, false on failure.
	 */
	bool (*start)(struct wlf_recorder_backend *backend);

	/**
	 * @brief Stops capture.
	 */
	void (*stop)(struct wlf_recorder_backend *backend);

	/**
	 * @brief Destroys backend and frees resources.
	 */
	void (*destroy)(struct wlf_recorder_backend *backend);
};

/**
 * @struct wlf_recorder_backend
 * @brief Base backend instance.
 */
struct wlf_recorder_backend {
	const struct wlf_recorder_backend_impl *impl;
	struct wlf_video_recorder *recorder;
	wlf_recorder_frame_callback frame_callback;
	void *user_data;
};

#ifdef __cplusplus
}
#endif

#endif /* WLF_RECORDER_BACKEND_H */
