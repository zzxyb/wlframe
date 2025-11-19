/**
 * @file        wlf_video_session.h
 * @brief       Vulkan video session management for wlframe.
 * @details     This file defines utilities for managing Vulkan video sessions,
 *              including session parameters and resource management.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-23, initial version\n
 */

#ifndef VIDEO_WLF_VIDEO_SESSION_H
#define VIDEO_WLF_VIDEO_SESSION_H

#include "wlf/video/wlf_video_common.h"
#include <vulkan/vulkan.h>

/**
 * @struct wlf_video_session
 * @brief Video session management structure.
 */
struct wlf_video_session {
	VkVideoSessionKHR session;          /**< Vulkan video session */
	VkVideoSessionParametersKHR params; /**< Video session parameters */
	VkDevice device;                    /**< Vulkan device */
	VkDeviceMemory memory;              /**< Session memory */
	bool is_encode;                     /**< True if encode session, false if decode */
};

/**
 * @brief Create a video session.
 *
 * @param device Vulkan device
 * @param profile Video profile
 * @param max_coded_extent Maximum coded extent
 * @param picture_format Picture format
 * @param max_dpb_slots Maximum DPB slots
 * @param queue_family_index Queue family index
 * @param is_encode True for encode session, false for decode
 * @return Pointer to created session, or NULL on failure
 */
struct wlf_video_session *wlf_video_session_create(
	VkDevice device,
	const VkVideoProfileInfoKHR *profile,
	VkExtent2D max_coded_extent,
	VkFormat picture_format,
	uint32_t max_dpb_slots,
	uint32_t queue_family_index,
	bool is_encode);

/**
 * @brief Destroy a video session.
 *
 * @param session Pointer to session to destroy
 */
void wlf_video_session_destroy(struct wlf_video_session *session);

/**
 * @brief Update video session parameters.
 *
 * @param session Pointer to session
 * @param update_data Codec-specific update data
 * @param update_size Size of update data
 * @return true on success, false on failure
 */
bool wlf_video_session_update_parameters(struct wlf_video_session *session,
	const void *update_data, size_t update_size);

#endif /* VIDEO_WLF_VIDEO_SESSION_H */
