/**
 * @file        wlf_vk_video_decoder.h
 * @brief       Vulkan video decoder implementation for wlframe.
 * @details     This file defines the Vulkan-based video decoder that uses
 *              Vulkan Video extensions for hardware-accelerated decoding.
 *              It can reuse VkDevice and VkPhysicalDevice from wlf_vk_renderer.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-30
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-30, initial version\n
 */

#ifndef VA_WLF_VK_VIDEO_DECODER_H
#define VA_WLF_VK_VIDEO_DECODER_H

#include "wlf/va/wlf_video_decoder.h"
#include <vulkan/vulkan.h>

struct wlf_vk_device;
struct wlf_vk_renderer;

/**
 * @struct wlf_vk_video_decoder_config
 * @brief Configuration for Vulkan video decoder creation.
 */
struct wlf_vk_video_decoder_config {
	struct wlf_video_decoder_config base; /**< Base decoder configuration */

	/* Option 1: Use existing Vulkan renderer */
	struct wlf_vk_renderer *renderer;     /**< Optional: reuse renderer's Vulkan device */

	/* Option 2: Use existing Vulkan device */
	struct wlf_vk_device *vk_device;      /**< Optional: reuse existing VkDevice wrapper */

	/* Option 3: Use custom Vulkan objects */
	VkDevice device;                      /**< Optional: custom Vulkan device */
	VkPhysicalDevice physical_device;     /**< Optional: custom Vulkan physical device */
	VkQueue decode_queue;                 /**< Optional: custom video decode queue */
	uint32_t queue_family_index;          /**< Optional: custom queue family index */
};

/**
 * @struct wlf_vk_video_decoder
 * @brief Vulkan video decoder instance.
 */
struct wlf_vk_video_decoder {
	struct wlf_video_decoder base;        /**< Base decoder (must be first) */

	/* Vulkan resources - can be shared from renderer */
	struct wlf_vk_device *vk_device;      /**< Vulkan device wrapper (may be shared) */
	bool owns_vk_device;                  /**< Whether we own the vk_device */

	/* Vulkan objects */
	VkDevice device;                      /**< Vulkan device */
	VkPhysicalDevice physical_device;     /**< Vulkan physical device */
	VkQueue decode_queue;                 /**< Video decode queue */
	uint32_t queue_family_index;          /**< Queue family index for video decode */

	/* Video session */
	VkVideoSessionKHR video_session;      /**< Vulkan video session */
	VkVideoSessionParametersKHR session_params; /**< Video session parameters */

	/* Decode resources */
	struct wlf_video_buffer *bitstream_buffer; /**< Bitstream buffer */
	struct wlf_video_image **dpb_images;       /**< Decoded Picture Buffer images */
	uint32_t dpb_count;                        /**< Number of DPB slots */

	/* Codec-specific data */
	void *codec_data;                     /**< Codec-specific data (H.264/H.265/AV1/VP9) */
};

/**
 * @brief Create a Vulkan video decoder.
 *
 * @param config Vulkan decoder configuration
 * @return Pointer to created decoder, or NULL on failure
 */
struct wlf_video_decoder *wlf_vk_video_decoder_create(
	const struct wlf_vk_video_decoder_config *config);

/**
 * @brief Create a Vulkan video decoder from an existing renderer.
 *
 * This is a convenience function that creates a decoder reusing the
 * Vulkan device from an existing renderer.
 *
 * @param renderer Pointer to Vulkan renderer
 * @param config Base decoder configuration
 * @return Pointer to created decoder, or NULL on failure
 */
struct wlf_video_decoder *wlf_vk_video_decoder_create_from_renderer(
	struct wlf_vk_renderer *renderer,
	const struct wlf_video_decoder_config *config);

/**
 * @brief Create a Vulkan video decoder from an existing Vulkan device.
 *
 * @param vk_device Pointer to Vulkan device wrapper
 * @param config Base decoder configuration
 * @return Pointer to created decoder, or NULL on failure
 */
struct wlf_video_decoder *wlf_vk_video_decoder_create_from_device(
	struct wlf_vk_device *vk_device,
	const struct wlf_video_decoder_config *config);

/**
 * @brief Check if a decoder is a Vulkan decoder.
 *
 * @param decoder Pointer to video decoder
 * @return true if decoder is Vulkan-based, false otherwise
 */
bool wlf_video_decoder_is_vk(struct wlf_video_decoder *decoder);

/**
 * @brief Convert a base decoder to Vulkan decoder.
 *
 * @param decoder Pointer to base decoder
 * @return Pointer to Vulkan decoder, or NULL if not a Vulkan decoder
 */
struct wlf_vk_video_decoder *wlf_vk_video_decoder_from_decoder(
	struct wlf_video_decoder *decoder);

/**
 * @brief Query video decode capabilities for Vulkan.
 *
 * @param physical_device Vulkan physical device
 * @param codec Video codec to query
 * @param capabilities Output capabilities structure
 * @return true if codec is supported, false otherwise
 */
bool wlf_vk_video_decoder_query_capabilities(VkPhysicalDevice physical_device,
	enum wlf_video_codec codec, VkVideoCapabilitiesKHR *capabilities);

#endif /* VA_WLF_VK_VIDEO_DECODER_H */
