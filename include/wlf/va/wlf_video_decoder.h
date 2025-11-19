/**
 * @file        wlf_video_decoder.h
 * @brief       Base video decoder interface for wlframe.
 * @details     This file defines the base video decoder API abstraction.
 *              Concrete implementations (Vulkan, VA-API, software) provide
 *              specific hardware acceleration methods. Supports H.264, H.265,
 *              AV1, and VP9 codecs.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-23, initial version\n
 */

#ifndef VIDEO_WLF_VIDEO_DECODER_H
#define VIDEO_WLF_VIDEO_DECODER_H

#include "wlf/video/wlf_video_common.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_video_decoder;

/**
 * @struct wlf_video_decoder_config
 * @brief Configuration for video decoder creation.
 */
struct wlf_video_decoder_config {
	enum wlf_video_codec codec;              /**< Video codec to decode */
	uint32_t max_width;                      /**< Maximum video width */
	uint32_t max_height;                     /**< Maximum video height */
	uint32_t max_dpb_slots;                  /**< Maximum DPB (Decoded Picture Buffer) slots */
	uint32_t max_active_references;          /**< Maximum active reference frames */
	enum wlf_video_chroma_format chroma;     /**< Expected chroma format */
	uint32_t bit_depth;                      /**< Bit depth (8, 10, or 12) */
	bool enable_film_grain;                  /**< Enable film grain synthesis (AV1) */
};

/**
 * @struct wlf_video_decoder_impl
 * @brief Video decoder implementation interface.
 */
struct wlf_video_decoder_impl {
	bool (*decode_frame)(struct wlf_video_decoder *decoder,
		const uint8_t *bitstream_data, size_t bitstream_size,
		struct wlf_video_image *output_image);
	void (*flush)(struct wlf_video_decoder *decoder);
	void (*destroy)(struct wlf_video_decoder *decoder);
};

/**
 * @struct wlf_video_decoder
 * @brief Base video decoder instance.
 */
struct wlf_video_decoder {
	const struct wlf_video_decoder_impl *impl;

	struct {
		struct wlf_signal frame_decoded;  /**< Signal emitted when frame is decoded */
		struct wlf_signal destroy;        /**< Signal emitted on destruction */
	} events;

	struct wlf_video_decoder_config config; /**< Decoder configuration */
	struct wlf_video_format format;         /**< Current video format */

	void *data;                           /**< User data */
};

/**
 * @brief Create a video decoder.
 *
 * @param config Decoder configuration
 * @return Pointer to created decoder, or NULL on failure
 */
struct wlf_video_decoder *wlf_video_decoder_create(
	const struct wlf_video_decoder_config *config);

/**
 * @brief Destroy a video decoder.
 *
 * @param decoder Pointer to decoder to destroy
 */
void wlf_video_decoder_destroy(struct wlf_video_decoder *decoder);

/**
 * @brief Decode a video frame.
 *
 * @param decoder Pointer to decoder
 * @param bitstream_data Pointer to compressed bitstream data
 * @param bitstream_size Size of bitstream data in bytes
 * @param output_image Pointer to output image structure
 * @return true on success, false on failure
 */
bool wlf_video_decoder_decode_frame(struct wlf_video_decoder *decoder,
	const uint8_t *bitstream_data, size_t bitstream_size,
	struct wlf_video_image *output_image);

/**
 * @brief Flush decoder and output all pending frames.
 *
 * @param decoder Pointer to decoder
 */
void wlf_video_decoder_flush(struct wlf_video_decoder *decoder);

#endif /* VIDEO_WLF_VIDEO_DECODER_H */
