#include "wlf/render/wlf_vk_render.h"
#include "wlf/version.h"

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>

// 简单的 Vulkan 错误字符串转换函数
static const char* vulkan_strerror(VkResult result) {
	switch (result) {
		case VK_SUCCESS: return "VK_SUCCESS";
		case VK_NOT_READY: return "VK_NOT_READY";
		case VK_TIMEOUT: return "VK_TIMEOUT";
		case VK_EVENT_SET: return "VK_EVENT_SET";
		case VK_EVENT_RESET: return "VK_EVENT_RESET";
		case VK_INCOMPLETE: return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		default: return "Unknown Vulkan error";
	}
}

static bool check_vk_extension(const VkExtensionProperties *avail,
		uint32_t avail_len, const char *name) {
	for (size_t i = 0; i < avail_len; i++) {
		if (strcmp(avail[i].extensionName, name) == 0) {
			return true;
		}
	}
	return false;
}

static VKAPI_ATTR VkBool32 vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *debug_data,
		void *data) {
	// we ignore some of the non-helpful warnings
	static const char *const ignored[] = {
		// notifies us that shader output is not consumed since
		// we use the shared vertex buffer with uv output
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
		wlf_log(WLF_ERROR, "wlframe requires vulkan 1.1 which is not available");
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

	struct wlf_vk_instance *instance = calloc(1, sizeof(struct wlf_vk_instance));
	if (!instance) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for wlf_vk_instance");
		return NULL;
	}

	size_t extensions_len = 0;
	const char *extensions[1] = {0};

	bool debug_utils_found = false;
	if (debug && check_vk_extension(avail_ext_props, avail_extc,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
		debug_utils_found = true;
		extensions[extensions_len++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	assert(extensions_len <= sizeof(extensions) / sizeof(extensions[0]));

	VkApplicationInfo application_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pEngineName = "wlframe",
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
		.pfnUserCallback = &vk_debug_callback,
		.pUserData = instance,
	};

	if (debug_utils_found) {
		// already adding the debug utils messenger extension to
		// instance creation gives us additional information during
		// instance creation and destruction, can be useful for debugging
		// layers/extensions not being found.
		instance_info.pNext = &debug_info;
	}

	res = vkCreateInstance(&instance_info, NULL, &instance->instance);
	if (res != VK_SUCCESS) {
		wlf_vk_error("Could not create instance", res);
		goto error;
	}

	if (debug_utils_found) {
		instance->ext.createDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				instance->instance, "vkCreateDebugUtilsMessengerEXT");
		instance->ext.destroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				instance->instance, "vkDestroyDebugUtilsMessengerEXT");

		if (instance->ext.createDebugUtilsMessengerEXT) {
			instance->ext.createDebugUtilsMessengerEXT(instance->instance,
				&debug_info, NULL, &instance->messenger);
		} else {
			wlf_log(WLF_ERROR, "vkCreateDebugUtilsMessengerEXT not found");
		}
	}

	return instance;

error:
	wlf_vk_instance_destroy(instance);
	return NULL;
}

void wlf_vk_instance_destroy(struct wlf_vk_instance *instance) {
	if (instance == NULL) {
		return;
	}

	if (instance->messenger && instance->ext.destroyDebugUtilsMessengerEXT) {
		instance->ext.destroyDebugUtilsMessengerEXT(instance->instance,
			instance->messenger, NULL);
	}

	if (instance->instance) {
		vkDestroyInstance(instance->instance, NULL);
	}

	free(instance);
}

struct wlf_render *wlf_vk_renderer_create(void) {
	wlf_log(WLF_INFO, "Run with VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation "
		"to enable the validation layer");

	struct wlf_vk_instance *instance = wlf_vk_instance_create(true);
	if (!instance) {
		wlf_log(WLF_ERROR, "creating vulkan instance for renderer failed");
		return NULL;
	}

	// TODO: 实现 Vulkan 渲染器创建逻辑
	(void)instance; // 避免未使用变量警告
	return NULL;
}
