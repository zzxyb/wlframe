/**
 * @file        wlf_sw_video_decoder.h
 * @brief       Software video decoder implementation for wlframe.
 * @details     This file defines the software-based video decoder that uses
 *              CPU-based decoding (e.g., FFmpeg libavcodec) as a fallback
 *              when hardware acceleration is not available.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-30
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-30, initial version\n
 */

#ifndef VA_WLF_SW_VIDEO_DECODER_H
#define VA_WLF_SW_VIDEO_DECODER_H

#include "wlf/va/wlf_video_decoder.h"

/**
 * @struct wlf_sw_video_decoder_config
 * @brief Configuration for software video decoder creation.
 */
struct wlf_sw_video_decoder_config {
	struct wlf_video_decoder_config base; /**< Base decoder configuration */

	uint32_t num_threads;                 /**< Number of decoder threads (0 = auto) */
	bool low_latency;                     /**< Enable low-latency mode */
};

/**
 * @struct wlf_sw_video_decoder
 * @brief Software video decoder instance.
 */
struct wlf_sw_video_decoder {
	struct wlf_video_decoder base;        /**< Base decoder (must be first) */

	/* Software decoder resources (e.g., FFmpeg AVCodecContext) */
	void *codec_context;                  /**< Codec context (e.g., AVCodecContext) */
	void *parser;                         /**< Parser context (e.g., AVCodecParserContext) */

	/* Frame buffers */
	void **frame_buffers;                 /**< Array of frame buffers */
	uint32_t frame_buffer_count;          /**< Number of frame buffers */

	/* Threading */
	uint32_t num_threads;                 /**< Number of decoder threads */

	/* Codec-specific data */
	void *codec_data;                     /**< Codec-specific data */
};

/**
 * @brief Create a software video decoder.
 *
 * @param config Software decoder configuration
 * @return Pointer to created decoder, or NULL on failure
 */
struct wlf_video_decoder *wlf_sw_video_decoder_create(
	const struct wlf_sw_video_decoder_config *config);

/**
 * @brief Check if a decoder is a software decoder.
 *
 * @param decoder Pointer to video decoder
 * @return true if decoder is software-based, false otherwise
 */
bool wlf_video_decoder_is_sw(struct wlf_video_decoder *decoder);

/**
 * @brief Convert a base decoder to software decoder.
 *
 * @param decoder Pointer to base decoder
 * @return Pointer to software decoder, or NULL if not a software decoder
 */
struct wlf_sw_video_decoder *wlf_sw_video_decoder_from_decoder(
	struct wlf_video_decoder *decoder);

/**
 * @brief Query software decoder capabilities.
 *
 * @param codec Video codec to query
 * @return true if codec is supported, false otherwise
 */
bool wlf_sw_video_decoder_query_capabilities(enum wlf_video_codec codec);

#endif /* VA_WLF_SW_VIDEO_DECODER_H */
