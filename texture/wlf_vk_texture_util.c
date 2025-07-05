#include "../../include/wlf/texture/wlf_vk_texture.h"

VkImage wlf_texture_get_vk_image(struct wlf_texture* texture) {
    if (!texture) {
        return VK_NULL_HANDLE;
    }

    // 假设所有Vulkan纹理都是 wlf_vk_texture 类型
    // 这里需要一些类型检查机制，但为了简化，直接转换
    struct wlf_vk_texture* vk_texture = (struct wlf_vk_texture*)texture;
    return vk_texture->image;
}
