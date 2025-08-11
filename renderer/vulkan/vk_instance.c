#include "wlf/renderer/vulkan/vk_instance.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/renderer/vulkan/vk_renderer.h"
#include "wlf/version.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

static VKAPI_ATTR VkBool32 debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *debug_data,
		void *data) {
	static const char *const ignored[] = {
		"UNASSIGNED-CoreValidation-Shader-OutputNotConsumed",
	};

	if (debug_data->pMessageIdName) {
		for (unsigned i = 0; i < sizeof(ignored) / sizeof(ignored[0]); ++i) {
			if (strcmp(debug_data->pMessageIdName, ignored[i]) == 0) {
				return false;
			}
		}
	}

	enum wlf_log_importance importance;
	switch (severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		importance = WLF_ERROR;
		break;
	default:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		importance = WLF_INFO;
		break;
	}

	wlf_log(importance, "%s (%s)", debug_data->pMessage,
		debug_data->pMessageIdName);
	if (debug_data->queueLabelCount > 0) {
		const char *name = debug_data->pQueueLabels[0].pLabelName;
		if (name) {
			wlf_log(importance, "    last label '%s'", name);
		}
	}

	for (unsigned i = 0; i < debug_data->objectCount; ++i) {
		if (debug_data->pObjects[i].pObjectName) {
			wlf_log(importance, "    involving '%s'", debug_data->pMessage);
		}
	}

	return false;
}

struct wlf_vk_instance *wlf_vk_instance_create(bool debug) {
	PFN_vkEnumerateInstanceVersion pfEnumInstanceVersion =
		(PFN_vkEnumerateInstanceVersion)
		vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
	if (!pfEnumInstanceVersion) {
		wlf_log(WLF_ERROR, "wlframe requires vulkan 1.1 which is not available");
		return NULL;
	}

	uint32_t ini_version;
	if (pfEnumInstanceVersion(&ini_version) != VK_SUCCESS ||
			ini_version < VK_API_VERSION_1_1) {
		wlf_log(WLF_ERROR, "wlroots requires vulkan 1.1 which is not available");
		return NULL;
	}

	uint32_t avail_extc = 0;
	VkResult res;
	res = vkEnumerateInstanceExtensionProperties(NULL, &avail_extc, NULL);
	if ((res != VK_SUCCESS) || (avail_extc == 0)) {
		wlf_vk_error("Could not enumerate instance extensions (1)", res);
		return NULL;
	}

	VkExtensionProperties avail_ext_props[avail_extc + 1];
	res = vkEnumerateInstanceExtensionProperties(NULL, &avail_extc,
		avail_ext_props);
	if (res != VK_SUCCESS) {
		wlf_vk_error("Could not enumerate instance extensions (2)", res);
		return NULL;
	}

	for (size_t j = 0; j < avail_extc; ++j) {
		wlf_log(WLF_DEBUG, "Vulkan instance extension %s v%"PRIu32,
			avail_ext_props[j].extensionName, avail_ext_props[j].specVersion);
	}

	struct wlf_vk_instance *ini = calloc(1, sizeof(*ini));
	if (!ini) {
		wlf_log_errno(WLF_ERROR, "allocation wlr_vk_instance failed");
		return NULL;
	}

	size_t extensions_len = 0;
	const char *extensions[1] = {0};

	bool debug_utils_found = false;
	if (debug && check_extension(avail_ext_props, avail_extc,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
		debug_utils_found = true;
		extensions[extensions_len++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	assert(extensions_len <= sizeof(extensions) / sizeof(extensions[0]));

	VkApplicationInfo application_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pEngineName = "wlroots",
		.engineVersion = WLF_VERSION_NUM,
		.apiVersion = VK_API_VERSION_1_1,
	};

	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &application_info,
		.enabledExtensionCount = extensions_len,
		.ppEnabledExtensionNames = extensions,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
	};

	VkDebugUtilsMessageSeverityFlagsEXT severity =
		// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	VkDebugUtilsMessageTypeFlagsEXT types =
		// VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	VkDebugUtilsMessengerCreateInfoEXT debug_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = severity,
		.messageType = types,
		.pfnUserCallback = &debug_callback,
		.pUserData = ini,
	};

	if (debug_utils_found) {
		instance_info.pNext = &debug_info;
	}

	res = vkCreateInstance(&instance_info, NULL, &ini->base);
	if (res != VK_SUCCESS) {
		wlf_vk_error("Could not create instance", res);
		goto error;
	}

	if (debug_utils_found) {
		ini->api.createDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				ini->base, "vkCreateDebugUtilsMessengerEXT");
		ini->api.destroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				ini->base, "vkDestroyDebugUtilsMessengerEXT");

		if (ini->api.createDebugUtilsMessengerEXT) {
			ini->api.createDebugUtilsMessengerEXT(ini->base,
				&debug_info, NULL, &ini->messenger);
		} else {
			wlf_log(WLF_ERROR, "vkCreateDebugUtilsMessengerEXT not found");
		}
	}

	return ini;

error:
	wlf_vk_instance_destroy(ini);
	return NULL;
}

void wlf_vk_instance_destroy(struct wlf_vk_instance *instance) {
	if (!instance) {
		return;
	}

	if (instance->messenger && instance->api.destroyDebugUtilsMessengerEXT) {
		instance->api.destroyDebugUtilsMessengerEXT(instance->base,
			instance->messenger, NULL);
	}

	if (instance->base) {
		vkDestroyInstance(instance->base, NULL);
	}

	free(instance);
}
