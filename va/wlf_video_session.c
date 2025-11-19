#include "wlf/video/wlf_video_session.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>

struct wlf_video_session *wlf_video_session_create(
	VkDevice device,
	const VkVideoProfileInfoKHR *profile,
	VkExtent2D max_coded_extent,
	VkFormat picture_format,
	uint32_t max_dpb_slots,
	uint32_t queue_family_index,
	bool is_encode) {

	if (!device || !profile) {
		wlf_log(WLF_ERROR, "Invalid parameters for session creation");
		return NULL;
	}

	struct wlf_video_session *session = calloc(1, sizeof(struct wlf_video_session));
	if (!session) {
		wlf_log(WLF_ERROR, "Failed to allocate video session");
		return NULL;
	}

	session->device = device;
	session->is_encode = is_encode;

	/* Create video session */
	VkVideoSessionCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR,
		.queueFamilyIndex = queue_family_index,
		.pVideoProfile = profile,
		.pictureFormat = picture_format,
		.maxCodedExtent = max_coded_extent,
		.referencePictureFormat = picture_format,
		.maxDpbSlots = max_dpb_slots,
		.maxActiveReferencePictures = max_dpb_slots,
	};

	VkResult result = vkCreateVideoSessionKHR(device, &create_info, NULL, &session->session);
	if (result != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create video session: %d", result);
		free(session);
		return NULL;
	}

	/* Get memory requirements and allocate */
	uint32_t mem_req_count;
	vkGetVideoSessionMemoryRequirementsKHR(device, session->session, &mem_req_count, NULL);

	if (mem_req_count > 0) {
		VkVideoSessionMemoryRequirementsKHR *mem_reqs =
			calloc(mem_req_count, sizeof(VkVideoSessionMemoryRequirementsKHR));

		if (mem_reqs) {
			for (uint32_t i = 0; i < mem_req_count; i++) {
				mem_reqs[i].sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR;
			}

			vkGetVideoSessionMemoryRequirementsKHR(device, session->session,
				&mem_req_count, mem_reqs);

			/* Allocate and bind memory - simplified, would need proper implementation */
			free(mem_reqs);
		}
	}

	wlf_log(WLF_DEBUG, "Created video session (%s)",
		is_encode ? "encode" : "decode");

	return session;
}

void wlf_video_session_destroy(struct wlf_video_session *session) {
	if (!session) {
		return;
	}

	if (session->params != VK_NULL_HANDLE) {
		vkDestroyVideoSessionParametersKHR(session->device, session->params, NULL);
	}

	if (session->session != VK_NULL_HANDLE) {
		vkDestroyVideoSessionKHR(session->device, session->session, NULL);
	}

	if (session->memory != VK_NULL_HANDLE) {
		vkFreeMemory(session->device, session->memory, NULL);
	}

	free(session);
}

bool wlf_video_session_update_parameters(struct wlf_video_session *session,
	const void *update_data, size_t update_size) {

	if (!session || !update_data) {
		return false;
	}

	/* Create or update session parameters */
	if (session->params == VK_NULL_HANDLE) {
		VkVideoSessionParametersCreateInfoKHR create_info = {
			.sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR,
			.videoSession = session->session,
			/* Codec-specific chains would be added here */
		};

		VkResult result = vkCreateVideoSessionParametersKHR(session->device,
			&create_info, NULL, &session->params);

		if (result != VK_SUCCESS) {
			wlf_log(WLF_ERROR, "Failed to create video session parameters: %d", result);
			return false;
		}
	} else {
		/* Update existing parameters */
		VkVideoSessionParametersUpdateInfoKHR update_info = {
			.sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_UPDATE_INFO_KHR,
			.updateSequenceCount = 1,
		};

		VkResult result = vkUpdateVideoSessionParametersKHR(session->device,
			session->params, &update_info);

		if (result != VK_SUCCESS) {
			wlf_log(WLF_ERROR, "Failed to update video session parameters: %d", result);
			return false;
		}
	}

	return true;
}
