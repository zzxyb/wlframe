/**
 * @file        wlf_video_common.h
 * @brief       Common definitions for Vulkan video codec support in wlframe.
 * @details     This file provides common data structures, enumerations, and types
 *              for Vulkan video encoding and decoding operations. It includes
 *              codec types, chroma formats, profile definitions, and buffer management.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-23, initial version\n
 */

#ifndef VIDEO_WLF_VIDEO_COMMON_H
#define VIDEO_WLF_VIDEO_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

/**
 * @enum wlf_video_codec
 * @brief Supported video codec types.
 */
enum wlf_video_codec {
	WLF_VIDEO_CODEC_H264 = 1,  /**< H.264/AVC codec */
	WLF_VIDEO_CODEC_H265 = 2,  /**< H.265/HEVC codec */
	WLF_VIDEO_CODEC_AV1 = 3,   /**< AV1 codec */
	WLF_VIDEO_CODEC_VP9 = 4,   /**< VP9 codec */
};

/**
 * @enum wlf_video_chroma_format
 * @brief Video chroma subsampling formats.
 */
enum wlf_video_chroma_format {
	WLF_VIDEO_CHROMA_MONOCHROME = 0,  /**< 4:0:0 monochrome */
	WLF_VIDEO_CHROMA_420 = 1,         /**< 4:2:0 subsampling */
	WLF_VIDEO_CHROMA_422 = 2,         /**< 4:2:2 subsampling */
	WLF_VIDEO_CHROMA_444 = 3,         /**< 4:4:4 no subsampling */
};

/**
 * @enum wlf_video_frame_type
 * @brief Video frame types.
 */
enum wlf_video_frame_type {
	WLF_VIDEO_FRAME_TYPE_I = 1,  /**< Intra frame (keyframe) */
	WLF_VIDEO_FRAME_TYPE_P = 2,  /**< Predicted frame */
	WLF_VIDEO_FRAME_TYPE_B = 3,  /**< Bidirectional frame */
	WLF_VIDEO_FRAME_TYPE_IDR = 4, /**< IDR frame (H.264/H.265) */
};

/**
 * @struct wlf_video_format
 * @brief Video format description.
 */
struct wlf_video_format {
	enum wlf_video_codec codec;              /**< Video codec type */
	enum wlf_video_chroma_format chroma;     /**< Chroma format */
	uint32_t width;                          /**< Video width in pixels */
	uint32_t height;                         /**< Video height in pixels */
	uint32_t bit_depth_luma;                 /**< Bit depth of luma component */
	uint32_t bit_depth_chroma;               /**< Bit depth of chroma components */
	uint32_t framerate_numerator;            /**< Framerate numerator */
	uint32_t framerate_denominator;          /**< Framerate denominator */
};

/**
 * @struct wlf_video_buffer
 * @brief Video buffer for encoded/decoded data.
 */
struct wlf_video_buffer {
	VkBuffer buffer;                    /**< Vulkan buffer handle */
	VkDeviceMemory memory;              /**< Device memory for buffer */
	VkDeviceSize size;                  /**< Buffer size in bytes */
	VkDeviceSize offset;                /**< Offset into buffer */
	void *mapped_data;                  /**< Mapped pointer (if mapped) */
	uint32_t ref_count;                 /**< Reference count */
};

/**
 * @struct wlf_video_image
 * @brief Video image for decoded frames.
 */
struct wlf_video_image {
	VkImage image;                      /**< Vulkan image handle */
	VkDeviceMemory memory;              /**< Device memory for image */
	VkImageView image_view;             /**< Image view */
	VkFormat format;                    /**< Image format */
	uint32_t width;                     /**< Image width */
	uint32_t height;                    /**< Image height */
	uint32_t ref_count;                 /**< Reference count */
};

/**
 * @brief Initialize a video buffer.
 *
 * @param buffer Pointer to the buffer to initialize
 * @param device Vulkan device
 * @param size Buffer size in bytes
 * @param usage Buffer usage flags
 * @return true on success, false on failure
 */
bool wlf_video_buffer_init(struct wlf_video_buffer *buffer,
	VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage);

/**
 * @brief Destroy a video buffer.
 *
 * @param buffer Pointer to the buffer to destroy
 * @param device Vulkan device
 */
void wlf_video_buffer_destroy(struct wlf_video_buffer *buffer, VkDevice device);

/**
 * @brief Map a video buffer for CPU access.
 *
 * @param buffer Pointer to the buffer to map
 * @param device Vulkan device
 * @return Mapped pointer on success, NULL on failure
 */
void *wlf_video_buffer_map(struct wlf_video_buffer *buffer, VkDevice device);

/**
 * @brief Unmap a video buffer.
 *
 * @param buffer Pointer to the buffer to unmap
 * @param device Vulkan device
 */
void wlf_video_buffer_unmap(struct wlf_video_buffer *buffer, VkDevice device);

/**
 * @brief Get codec name string.
 *
 * @param codec Codec type
 * @return String name of codec
 */
const char *wlf_video_codec_to_string(enum wlf_video_codec codec);

/**
 * @brief Get chroma format string.
 *
 * @param chroma Chroma format
 * @return String name of chroma format
 */
const char *wlf_video_chroma_to_string(enum wlf_video_chroma_format chroma);

#endif /* VIDEO_WLF_VIDEO_COMMON_H */
