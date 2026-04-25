#ifndef VULKAN_VULKAN_H
#define VULKAN_VULKAN_H

#include <vulkan/vulkan.h>
#include <wlf/math/wlf_matrix3x3.h>
#include <wlf/math/wlf_matrix4x4.h>

bool wlf_vk_create_shader_module(VkDevice dev, const uint32_t *code,
	size_t code_size, const char *name, VkShaderModule *out);
float wlf_color_to_linear(float non_linear);
float wlf_color_to_linear_premult(float non_linear, float alpha);
void wlf_encode_proj_matrix(const struct wlf_matrix3x3 *mat3, struct wlf_matrix4x4 *mat4);
void wlf_encode_color_matrix(const struct wlf_matrix3x3 *mat3, struct wlf_matrix4x4 *mat4);

#endif // VULKAN_VULKAN_H
