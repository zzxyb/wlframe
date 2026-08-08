/**
 * @file        vulkan.h
 * @brief       Shared Vulkan rendering helpers.
 * @details     Provides shader-module, color-transfer, and matrix conversion
 *              helpers used by the Vulkan renderer and rendering passes.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef VULKAN_VULKAN_H
#define VULKAN_VULKAN_H

#include <vulkan/vulkan.h>
#include <wlf/math/wlf_matrix3x3.h>
#include <wlf/math/wlf_matrix4x4.h>

/**
 * @brief Creates a Vulkan shader module from SPIR-V bytecode.
 *
 * The bytecode is passed directly to vkCreateShaderModule. The optional name
 * is used only for debug labelling when the renderer supports it.
 *
 * @param dev Logical Vulkan device.
 * @param code SPIR-V words.
 * @param code_size Size of @p code in bytes.
 * @param name Debug name for the module, when supported.
 * @param out Output shader-module handle.
 * @return true on success, false on failure.
 */
bool wlf_vk_create_shader_module(VkDevice dev, const uint32_t *code,
	size_t code_size, const char *name, VkShaderModule *out);

/**
 * @brief Converts an sRGB-like non-linear color component to linear light.
 * @param non_linear Non-linear color component.
 * @return Linear-light color component.
 */
float wlf_color_to_linear(float non_linear);

/**
 * @brief Converts a premultiplied non-linear color component to linear light.
 * @param non_linear Premultiplied non-linear color component.
 * @param alpha Unpremultiplied alpha component.
 * @return Premultiplied linear-light color component.
 */
float wlf_color_to_linear_premult(float non_linear, float alpha);

/**
 * @brief Encodes a 3x3 projection matrix into the 4x4 layout used by Vulkan.
 * @param mat3 Source 3x3 matrix.
 * @param mat4 Destination 4x4 matrix.
 */
void wlf_encode_proj_matrix(const struct wlf_matrix3x3 *mat3, struct wlf_matrix4x4 *mat4);

/**
 * @brief Encodes a 3x3 color matrix into the 4x4 layout used by Vulkan.
 * @param mat3 Source 3x3 matrix.
 * @param mat4 Destination 4x4 matrix.
 */
void wlf_encode_color_matrix(const struct wlf_matrix3x3 *mat3, struct wlf_matrix4x4 *mat4);

#endif // VULKAN_VULKAN_H
