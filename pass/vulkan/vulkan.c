#include "wlf/pass/vulkan/vulkan.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/renderer/vulkan/renderer.h"

bool wlf_vk_create_shader_module(VkDevice dev, const uint32_t *code,
		size_t code_size, const char *name, VkShaderModule *out) {
	VkShaderModuleCreateInfo sinfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code_size,
		.pCode = code,
	};

	VkResult res = vkCreateShaderModule(dev, &sinfo, NULL, out);
	if (res != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create %s shader module: %s (%d)",
			name, wlf_vulkan_strerror(res), res);
		return false;
	}

	return true;
}

float wlf_color_to_linear(float non_linear) {
	return pow(non_linear, 2.2);
}

float wlf_color_to_linear_premult(float non_linear, float alpha) {
	return (alpha == 0) ? 0 : wlf_color_to_linear(non_linear / alpha) * alpha;
}

// void wlf_encode_proj_matrix(const struct wlf_matrix3x3 *mat3, struct wlf_matrix4x4 *mat4) {
// 	struct wlf_matrix4x4 result = {0};
// 	result.elements[0][0] =
// 		{ mat3->elements[0][0], mat3->elements[0][1], 0, mat3->elements[0][2] },
// 		{ mat3->elements[3][0], mat3->elements[3][1], 0, mat3->elements[3][2] },
// 		{ 0, 0, 1, 0 },
// 		{ 0, 0, 0, 1 },
// 	};

// 	memcpy(mat4, result, sizeof(result));
// }

// void wlf_encode_color_matrix(const struct wlf_matrix3x3 *mat3, struct wlf_matrix4x4 *mat4);
