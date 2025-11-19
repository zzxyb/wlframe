/**
 * @file        wlf_video_recorder.h
 * @brief       Video recording interface with multiple backend support.
 * @details     Provides a unified interface for recording video from various
 *              sources (dmabuf, pipewire) to file formats like MP4, WebM, etc.
 *              Inspired by Weston's screen recorder implementation.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 * @version     v1.0
 */

#ifndef WLF_VIDEO_RECORDER_H
#define WLF_VIDEO_RECORDER_H

#include <stdbool.h>
#include <stdint.h>
#include "wlf/va/wlf_video_encoder.h"
#include "wlf/dmabuf/wlf_dmabuf.h"
#include "wlf/utils/wlf_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wlf_video_recorder;
struct wlf_recorder_backend;

/**
 * @enum wlf_recorder_container_format
 * @brief Container format for output file.
 */
enum wlf_recorder_container_format {
	WLF_RECORDER_FORMAT_MP4,          /**< MP4 container */
	WLF_RECORDER_FORMAT_WEBM,         /**< WebM container */
	WLF_RECORDER_FORMAT_MKV,          /**< Matroska container */
	WLF_RECORDER_FORMAT_AVI,          /**< AVI container (legacy) */
};

/**
 * @enum wlf_recorder_state
 * @brief Recording state.
 */
enum wlf_recorder_state {
	WLF_RECORDER_STATE_IDLE,          /**< Recorder is idle */
	WLF_RECORDER_STATE_RECORDING,     /**< Currently recording */
	WLF_RECORDER_STATE_PAUSED,        /**< Recording paused */
	WLF_RECORDER_STATE_ERROR,         /**< Error occurred */
};

/**
 * @struct wlf_recorder_config
 * @brief Configuration for video recorder creation.
 */
struct wlf_recorder_config {
	/* Video encoding configuration */
	struct wlf_video_encoder_config encoder_config;  /**< Encoder settings */

	/* Output configuration */
	const char *output_filename;                  /**< Output file path */
	enum wlf_recorder_container_format format;    /**< Container format */

	/* Audio configuration (optional) */
	bool enable_audio;                            /**< Enable audio recording */
	uint32_t audio_sample_rate;                   /**< Audio sample rate (Hz) */
	uint32_t audio_channels;                      /**< Number of audio channels */

	/* PipeWire-specific configuration */
	const char *pipewire_node_name;               /**< PipeWire node name to capture */
	uint32_t pipewire_node_id;                    /**< PipeWire node ID (0 = auto) */

	/* Buffer management */
	uint32_t max_buffer_frames;                   /**< Maximum frames to buffer */
	bool drop_frames_on_overflow;                 /**< Drop frames if buffer full */
};

/**
 * @struct wlf_recorder_statistics
 * @brief Recording statistics and performance metrics.
 */
struct wlf_recorder_statistics {
	uint64_t total_frames_captured;    /**< Total frames captured */
	uint64_t total_frames_encoded;     /**< Total frames encoded */
	uint64_t total_frames_dropped;     /**< Total frames dropped */
	uint64_t total_bytes_written;      /**< Total bytes written to file */
	double average_fps;                /**< Average FPS */
	double average_encode_time_ms;     /**< Average encode time per frame */
	uint64_t recording_duration_us;    /**< Total recording duration (microseconds) */
};

/**
 * @brief Creates a new video recorder instance.
 *
 * @param backend Backend instance for frame capture.
 * @param config Recorder configuration.
 * @return New recorder instance, or NULL on failure.
 */
struct wlf_video_recorder *wlf_video_recorder_create(
	struct wlf_recorder_backend *backend,
	const struct wlf_recorder_config *config);

/**
 * @brief Destroys a video recorder and frees resources.
 *
 * @param recorder Recorder instance to destroy.
 */
void wlf_video_recorder_destroy(struct wlf_video_recorder *recorder);

/**
 * @brief Starts recording.
 *
 * @param recorder Recorder instance.
 * @return true on success, false on failure.
 */
bool wlf_video_recorder_start(struct wlf_video_recorder *recorder);

/**
 * @brief Stops recording and finalizes output file.
 *
 * @param recorder Recorder instance.
 * @return true on success, false on failure.
 */
bool wlf_video_recorder_stop(struct wlf_video_recorder *recorder);

/**
 * @brief Pauses recording (keeps resources active).
 *
 * @param recorder Recorder instance.
 * @return true on success, false on failure.
 */
bool wlf_video_recorder_pause(struct wlf_video_recorder *recorder);

/**
 * @brief Resumes recording from paused state.
 *
 * @param recorder Recorder instance.
 * @return true on success, false on failure.
 */
bool wlf_video_recorder_resume(struct wlf_video_recorder *recorder);

/**
 * @brief Submits a DMA-BUF frame for recording (dmabuf backend).
 *
 * @param recorder Recorder instance.
 * @param attribs DMA-BUF attributes of the frame.
 * @param timestamp_us Frame timestamp in microseconds.
 * @return true on success, false on failure.
 */
bool wlf_video_recorder_submit_dmabuf(struct wlf_video_recorder *recorder,
	const struct wlf_dmabuf_attributes *attribs,
	uint64_t timestamp_us);

/**
 * @brief Gets current recording state.
 *
 * @param recorder Recorder instance.
 * @return Current recording state.
 */
enum wlf_recorder_state wlf_video_recorder_get_state(
	const struct wlf_video_recorder *recorder);

/**
 * @brief Gets recording statistics.
 *
 * @param recorder Recorder instance.
 * @param stats Output statistics structure.
 * @return true on success, false on failure.
 */
bool wlf_video_recorder_get_statistics(
	const struct wlf_video_recorder *recorder,
	struct wlf_recorder_statistics *stats);

/**
 * @brief Sets recording quality (dynamically adjusts encoder parameters).
 *
 * @param recorder Recorder instance.
 * @param quality Quality level (0-100, higher is better).
 * @return true on success, false on failure.
 */
bool wlf_video_recorder_set_quality(struct wlf_video_recorder *recorder,
	uint32_t quality);

#ifdef __cplusplus
}
#endif

#endif /* WLF_VIDEO_RECORDER_H */
