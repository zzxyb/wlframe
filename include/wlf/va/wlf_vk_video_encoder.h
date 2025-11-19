/**
 * @file        wlf_vk_video_encoder.h
 * @brief       Vulkan video encoder implementation for wlframe.
 * @details     This file defines the Vulkan-based video encoder that uses
 *              Vulkan Video extensions for hardware-accelerated encoding.
 *              It can reuse VkDevice and VkPhysicalDevice from wlf_vk_renderer.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-30
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-30, initial version\n
 */

#ifndef VA_WLF_VK_VIDEO_ENCODER_H
#define VA_WLF_VK_VIDEO_ENCODER_H

#include "wlf/va/wlf_video_encoder.h"
#include <vulkan/vulkan.h>

struct wlf_vk_device;
struct wlf_vk_renderer;

/**
 * @struct wlf_vk_video_encoder_config
 * @brief Configuration for Vulkan video encoder creation.
 */
struct wlf_vk_video_encoder_config {
	struct wlf_video_encoder_config base; /**< Base encoder configuration */

	/* Option 1: Use existing Vulkan renderer */
	struct wlf_vk_renderer *renderer;     /**< Optional: reuse renderer's Vulkan device */

	/* Option 2: Use existing Vulkan device */
	struct wlf_vk_device *vk_device;      /**< Optional: reuse existing VkDevice wrapper */

	/* Option 3: Use custom Vulkan objects */
	VkDevice device;                      /**< Optional: custom Vulkan device */
	VkPhysicalDevice physical_device;     /**< Optional: custom Vulkan physical device */
	VkQueue encode_queue;                 /**< Optional: custom video encode queue */
	uint32_t queue_family_index;          /**< Optional: custom queue family index */
};

/**
 * @struct wlf_vk_video_encoder
 * @brief Vulkan video encoder instance.
 */
struct wlf_vk_video_encoder {
	struct wlf_video_encoder base;        /**< Base encoder (must be first) */

	/* Vulkan resources - can be shared from renderer */
	struct wlf_vk_device *vk_device;      /**< Vulkan device wrapper (may be shared) */
	bool owns_vk_device;                  /**< Whether we own the vk_device */

	/* Vulkan objects */
	VkDevice device;                      /**< Vulkan device */
	VkPhysicalDevice physical_device;     /**< Vulkan physical device */
	VkQueue encode_queue;                 /**< Video encode queue */
	uint32_t queue_family_index;          /**< Queue family index for video encode */

	/* Video session */
	VkVideoSessionKHR video_session;      /**< Vulkan video session */
	VkVideoSessionParametersKHR session_params; /**< Video session parameters */

	/* Encode resources */
	struct wlf_video_buffer *output_buffer; /**< Output bitstream buffer */
	struct wlf_video_image **dpb_images;    /**< Reference picture buffer */
	uint32_t dpb_count;                     /**< Number of DPB slots */

	/* Codec-specific data */
	void *codec_data;                     /**< Codec-specific data (H.264/H.265/AV1) */
};

/**
 * @brief Create a Vulkan video encoder.
 *
 * @param config Vulkan encoder configuration
 * @return Pointer to created encoder, or NULL on failure
 */
struct wlf_video_encoder *wlf_vk_video_encoder_create(
	const struct wlf_vk_video_encoder_config *config);

/**
 * @brief Create a Vulkan video encoder from an existing renderer.
 *
 * This is a convenience function that creates an encoder reusing the
 * Vulkan device from an existing renderer.
 *
 * @param renderer Pointer to Vulkan renderer
 * @param config Base encoder configuration
 * @return Pointer to created encoder, or NULL on failure
 */
struct wlf_video_encoder *wlf_vk_video_encoder_create_from_renderer(
	struct wlf_vk_renderer *renderer,
	const struct wlf_video_encoder_config *config);

/**
 * @brief Create a Vulkan video encoder from an existing Vulkan device.
 *
 * @param vk_device Pointer to Vulkan device wrapper
 * @param config Base encoder configuration
 * @return Pointer to created encoder, or NULL on failure
 */
struct wlf_video_encoder *wlf_vk_video_encoder_create_from_device(
	struct wlf_vk_device *vk_device,
	const struct wlf_video_encoder_config *config);

/**
 * @brief Check if an encoder is a Vulkan encoder.
 *
 * @param encoder Pointer to video encoder
 * @return true if encoder is Vulkan-based, false otherwise
 */
bool wlf_video_encoder_is_vk(struct wlf_video_encoder *encoder);

/**
 * @brief Convert a base encoder to Vulkan encoder.
 *
 * @param encoder Pointer to base encoder
 * @return Pointer to Vulkan encoder, or NULL if not a Vulkan encoder
 */
struct wlf_vk_video_encoder *wlf_vk_video_encoder_from_encoder(
	struct wlf_video_encoder *encoder);

/**
 * @brief Query video encode capabilities for Vulkan.
 *
 * @param physical_device Vulkan physical device
 * @param codec Video codec to query
 * @param capabilities Output capabilities structure
 * @return true if codec is supported, false otherwise
 */
bool wlf_vk_video_encoder_query_capabilities(VkPhysicalDevice physical_device,
	enum wlf_video_codec codec, VkVideoCapabilitiesKHR *capabilities);

#endif /* VA_WLF_VK_VIDEO_ENCODER_H */
