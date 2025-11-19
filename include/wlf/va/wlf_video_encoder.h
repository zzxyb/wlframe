/**
 * @file        wlf_video_encoder.h
 * @brief       Base video encoder interface for wlframe.
 * @details     This file defines the base video encoder API abstraction.
 *              Concrete implementations (Vulkan, VA-API, software) provide
 *              specific hardware acceleration methods. Supports H.264, H.265,
 *              and AV1 codecs.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-23, initial version\n
 */

#ifndef VIDEO_WLF_VIDEO_ENCODER_H
#define VIDEO_WLF_VIDEO_ENCODER_H

#include "wlf/video/wlf_video_common.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_video_encoder;

/**
 * @enum wlf_video_rate_control_mode
 * @brief Video rate control modes.
 */
enum wlf_video_rate_control_mode {
	WLF_VIDEO_RATE_CONTROL_DISABLED = 0,  /**< No rate control */
	WLF_VIDEO_RATE_CONTROL_CBR = 1,       /**< Constant bitrate */
	WLF_VIDEO_RATE_CONTROL_VBR = 2,       /**< Variable bitrate */
	WLF_VIDEO_RATE_CONTROL_CQP = 3,       /**< Constant quantization parameter */
};

/**
 * @struct wlf_video_encoder_config
 * @brief Configuration for video encoder creation.
 */
struct wlf_video_encoder_config {
	enum wlf_video_codec codec;              /**< Video codec to encode */
	uint32_t width;                          /**< Video width */
	uint32_t height;                         /**< Video height */
	uint32_t framerate_num;                  /**< Framerate numerator */
	uint32_t framerate_den;                  /**< Framerate denominator */
	enum wlf_video_chroma_format chroma;     /**< Chroma format */
	uint32_t bit_depth;                      /**< Bit depth (8, 10, or 12) */

	/* Rate control */
	enum wlf_video_rate_control_mode rate_control_mode; /**< Rate control mode */
	uint32_t target_bitrate;                 /**< Target bitrate in bits/sec */
	uint32_t max_bitrate;                    /**< Maximum bitrate in bits/sec */
	uint32_t qp_i;                           /**< QP for I frames (CQP mode) */
	uint32_t qp_p;                           /**< QP for P frames (CQP mode) */
	uint32_t qp_b;                           /**< QP for B frames (CQP mode) */

	/* GOP structure */
	uint32_t gop_size;                       /**< GOP size (I-frame interval) */
	uint32_t num_b_frames;                   /**< Number of B frames between references */
	bool use_open_gop;                       /**< Use open GOP structure */

	/* Codec-specific */
	uint32_t profile;                        /**< Codec profile */
	uint32_t level;                          /**< Codec level */
	uint32_t tier;                           /**< Codec tier (HEVC/AV1) */
};

/**
 * @struct wlf_video_encoded_frame
 * @brief Encoded video frame output.
 */
struct wlf_video_encoded_frame {
	uint8_t *data;                      /**< Encoded data */
	size_t size;                        /**< Size of encoded data */
	enum wlf_video_frame_type type;     /**< Frame type */
	uint64_t pts;                       /**< Presentation timestamp */
	uint64_t dts;                       /**< Decode timestamp */
	bool is_keyframe;                   /**< Is this a keyframe? */
};

/**
 * @struct wlf_video_encoder_impl
 * @brief Video encoder implementation interface.
 */
struct wlf_video_encoder_impl {
	bool (*encode_frame)(struct wlf_video_encoder *encoder,
		const struct wlf_video_image *input_image,
		struct wlf_video_encoded_frame *output_frame);
	void (*flush)(struct wlf_video_encoder *encoder);
	void (*destroy)(struct wlf_video_encoder *encoder);
};

/**
 * @struct wlf_video_encoder
 * @brief Base video encoder instance.
 */
struct wlf_video_encoder {
	const struct wlf_video_encoder_impl *impl;

	struct {
		struct wlf_signal frame_encoded;  /**< Signal emitted when frame is encoded */
		struct wlf_signal destroy;        /**< Signal emitted on destruction */
	} events;

	struct wlf_video_encoder_config config; /**< Encoder configuration */
	struct wlf_video_format format;         /**< Video format */

	uint64_t frame_count;                 /**< Total frames encoded */
	uint64_t current_pts;                 /**< Current PTS */

	void *data;                           /**< User data */
};

/**
 * @brief Create a video encoder.
 *
 * @param config Encoder configuration
 * @return Pointer to created encoder, or NULL on failure
 */
struct wlf_video_encoder *wlf_video_encoder_create(
	const struct wlf_video_encoder_config *config);

/**
 * @brief Destroy a video encoder.
 *
 * @param encoder Pointer to encoder to destroy
 */
void wlf_video_encoder_destroy(struct wlf_video_encoder *encoder);

/**
 * @brief Encode a video frame.
 *
 * @param encoder Pointer to encoder
 * @param input_image Pointer to input image to encode
 * @param output_frame Pointer to output encoded frame structure
 * @return true on success, false on failure
 */
bool wlf_video_encoder_encode_frame(struct wlf_video_encoder *encoder,
	const struct wlf_video_image *input_image,
	struct wlf_video_encoded_frame *output_frame);

/**
 * @brief Flush encoder and output all pending frames.
 *
 * @param encoder Pointer to encoder
 */
void wlf_video_encoder_flush(struct wlf_video_encoder *encoder);

#endif /* VIDEO_WLF_VIDEO_ENCODER_H */
