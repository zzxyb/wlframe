/**
 * @file        wlf_sw_video_encoder.h
 * @brief       Software video encoder implementation for wlframe.
 * @details     This file defines the software-based video encoder that uses
 *              CPU-based encoding (e.g., x264, x265, libaom) as a fallback
 *              when hardware acceleration is not available.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-30
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-30, initial version\n
 */

#ifndef VA_WLF_SW_VIDEO_ENCODER_H
#define VA_WLF_SW_VIDEO_ENCODER_H

#include "wlf/va/wlf_video_encoder.h"

/**
 * @enum wlf_sw_encoder_preset
 * @brief Software encoder speed/quality presets.
 */
enum wlf_sw_encoder_preset {
	WLF_SW_PRESET_ULTRAFAST = 0,  /**< Fastest encoding, lowest quality */
	WLF_SW_PRESET_SUPERFAST,      /**< Very fast encoding */
	WLF_SW_PRESET_VERYFAST,       /**< Fast encoding */
	WLF_SW_PRESET_FASTER,         /**< Faster encoding */
	WLF_SW_PRESET_FAST,           /**< Fast encoding */
	WLF_SW_PRESET_MEDIUM,         /**< Balanced (default) */
	WLF_SW_PRESET_SLOW,           /**< Slower, better quality */
	WLF_SW_PRESET_SLOWER,         /**< Much slower, high quality */
	WLF_SW_PRESET_VERYSLOW,       /**< Very slow, very high quality */
	WLF_SW_PRESET_PLACEBO,        /**< Slowest, highest quality */
};

/**
 * @enum wlf_sw_encoder_tune
 * @brief Software encoder tuning options.
 */
enum wlf_sw_encoder_tune {
	WLF_SW_TUNE_NONE = 0,         /**< No specific tuning */
	WLF_SW_TUNE_FILM,             /**< Tune for film content */
	WLF_SW_TUNE_ANIMATION,        /**< Tune for animation */
	WLF_SW_TUNE_GRAIN,            /**< Tune for grainy content */
	WLF_SW_TUNE_STILLIMAGE,       /**< Tune for still images */
	WLF_SW_TUNE_FASTDECODE,       /**< Tune for fast decoding */
	WLF_SW_TUNE_ZEROLATENCY,      /**< Tune for zero latency streaming */
};

/**
 * @struct wlf_sw_video_encoder_config
 * @brief Configuration for software video encoder creation.
 */
struct wlf_sw_video_encoder_config {
	struct wlf_video_encoder_config base; /**< Base encoder configuration */

	enum wlf_sw_encoder_preset preset;    /**< Speed/quality preset */
	enum wlf_sw_encoder_tune tune;        /**< Tuning option */
	uint32_t num_threads;                 /**< Number of encoder threads (0 = auto) */
	bool low_latency;                     /**< Enable low-latency mode */
	int crf;                              /**< Constant Rate Factor (for CRF mode, -1 = use bitrate) */
};

/**
 * @struct wlf_sw_video_encoder
 * @brief Software video encoder instance.
 */
struct wlf_sw_video_encoder {
	struct wlf_video_encoder base;        /**< Base encoder (must be first) */

	/* Software encoder resources (e.g., x264_t, x265_encoder) */
	void *encoder_context;                /**< Encoder context (codec-specific) */
	void *picture_buffer;                 /**< Picture buffer for input */

	/* Configuration */
	enum wlf_sw_encoder_preset preset;    /**< Speed/quality preset */
	enum wlf_sw_encoder_tune tune;        /**< Tuning option */
	uint32_t num_threads;                 /**< Number of encoder threads */

	/* Output buffer */
	uint8_t *output_buffer;               /**< Output bitstream buffer */
	size_t output_buffer_size;            /**< Size of output buffer */

	/* Codec-specific data */
	void *codec_data;                     /**< Codec-specific data */
};

/**
 * @brief Create a software video encoder.
 *
 * @param config Software encoder configuration
 * @return Pointer to created encoder, or NULL on failure
 */
struct wlf_video_encoder *wlf_sw_video_encoder_create(
	const struct wlf_sw_video_encoder_config *config);

/**
 * @brief Check if an encoder is a software encoder.
 *
 * @param encoder Pointer to video encoder
 * @return true if encoder is software-based, false otherwise
 */
bool wlf_video_encoder_is_sw(struct wlf_video_encoder *encoder);

/**
 * @brief Convert a base encoder to software encoder.
 *
 * @param encoder Pointer to base encoder
 * @return Pointer to software encoder, or NULL if not a software encoder
 */
struct wlf_sw_video_encoder *wlf_sw_video_encoder_from_encoder(
	struct wlf_video_encoder *encoder);

/**
 * @brief Query software encoder capabilities.
 *
 * @param codec Video codec to query
 * @return true if codec is supported, false otherwise
 */
bool wlf_sw_video_encoder_query_capabilities(enum wlf_video_codec codec);

#endif /* VA_WLF_SW_VIDEO_ENCODER_H */
