#include "wlf/framebuffer/wlf_vk_framebuffer.h"

#include <stdlib.h>
#include <string.h>

static void vk_framebuffer_destroy(struct wlf_framebuffer* fb) {
    if (!fb) return;

    struct wlf_vk_framebuffer* vk_fb = (struct wlf_vk_framebuffer*)fb;

    // 销毁Vulkan对象
    if (vk_fb->framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(vk_fb->device, vk_fb->framebuffer, NULL);
    }

    if (vk_fb->render_pass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(vk_fb->device, vk_fb->render_pass, NULL);
    }

    // 销毁图像视图和图像
    for (int i = 0; i < 4; i++) {
        if (vk_fb->color_views[i] != VK_NULL_HANDLE) {
            vkDestroyImageView(vk_fb->device, vk_fb->color_views[i], NULL);
        }
        if (vk_fb->color_images[i] != VK_NULL_HANDLE) {
            vkDestroyImage(vk_fb->device, vk_fb->color_images[i], NULL);
        }
        if (vk_fb->color_memory[i] != VK_NULL_HANDLE) {
            vkFreeMemory(vk_fb->device, vk_fb->color_memory[i], NULL);
        }
    }

    if (vk_fb->depth_view != VK_NULL_HANDLE) {
        vkDestroyImageView(vk_fb->device, vk_fb->depth_view, NULL);
    }
    if (vk_fb->depth_image != VK_NULL_HANDLE) {
        vkDestroyImage(vk_fb->device, vk_fb->depth_image, NULL);
    }
    if (vk_fb->depth_memory != VK_NULL_HANDLE) {
        vkFreeMemory(vk_fb->device, vk_fb->depth_memory, NULL);
    }

    if (vk_fb->stencil_view != VK_NULL_HANDLE) {
        vkDestroyImageView(vk_fb->device, vk_fb->stencil_view, NULL);
    }
    if (vk_fb->stencil_image != VK_NULL_HANDLE) {
        vkDestroyImage(vk_fb->device, vk_fb->stencil_image, NULL);
    }
    if (vk_fb->stencil_memory != VK_NULL_HANDLE) {
        vkFreeMemory(vk_fb->device, vk_fb->stencil_memory, NULL);
    }

    free(vk_fb);
}

static bool vk_framebuffer_bind(struct wlf_framebuffer* fb) {
    if (!fb) return false;

    // Vulkan 中帧缓冲的绑定是通过渲染通道实现的
    // 这里只是标记状态
    fb->is_bound = true;
    return true;
}

static void vk_framebuffer_unbind(struct wlf_framebuffer* fb) {
    if (!fb) return;

    fb->is_bound = false;
}

static bool vk_framebuffer_attach_color(struct wlf_framebuffer* fb,
                                       enum wlf_framebuffer_attachment attachment,
                                       struct wlf_texture* texture,
                                       int mip_level) {
    if (!fb || attachment > WLF_FB_ATTACHMENT_COLOR3) return false;

    struct wlf_vk_framebuffer* vk_fb = (struct wlf_vk_framebuffer*)fb;

    int index = attachment - WLF_FB_ATTACHMENT_COLOR0;

    // 获取纹理的Vulkan图像
    // VkImage image = wlf_texture_get_vk_image(texture);
    // VkImageView view = wlf_texture_get_vk_image_view(texture);

    // 暂时使用空值
    vk_fb->color_images[index] = VK_NULL_HANDLE;
    vk_fb->color_views[index] = VK_NULL_HANDLE;

    if (index >= vk_fb->num_color_attachments) {
        vk_fb->num_color_attachments = index + 1;
    }

    fb->color_attachments[index] = texture;

    return true;
}

static bool vk_framebuffer_attach_depth(struct wlf_framebuffer* fb,
                                       struct wlf_texture* texture,
                                       int mip_level) {
    if (!fb) return false;

    // struct wlf_vk_framebuffer* vk_fb = (struct wlf_vk_framebuffer*)fb;

    // 获取纹理的Vulkan图像
    // vk_fb->depth_image = wlf_texture_get_vk_image(texture);
    // vk_fb->depth_view = wlf_texture_get_vk_image_view(texture);

    fb->depth_attachment = texture;

    return true;
}

static bool vk_framebuffer_attach_stencil(struct wlf_framebuffer* fb,
                                         struct wlf_texture* texture,
                                         int mip_level) {
    if (!fb) return false;

    // struct wlf_vk_framebuffer* vk_fb = (struct wlf_vk_framebuffer*)fb;

    // 获取纹理的Vulkan图像
    // vk_fb->stencil_image = wlf_texture_get_vk_image(texture);
    // vk_fb->stencil_view = wlf_texture_get_vk_image_view(texture);

    fb->stencil_attachment = texture;

    return true;
}

static bool vk_framebuffer_is_complete(struct wlf_framebuffer* fb) {
    if (!fb) return false;

    struct wlf_vk_framebuffer* vk_fb = (struct wlf_vk_framebuffer*)fb;

    // 检查Vulkan帧缓冲是否有效
    return vk_fb->framebuffer != VK_NULL_HANDLE && vk_fb->render_pass != VK_NULL_HANDLE;
}

static void vk_framebuffer_clear(struct wlf_framebuffer* fb,
                                float r, float g, float b, float a,
                                float depth, int stencil) {
    if (!fb) return;

    // Vulkan 中清除操作通常在渲染通道开始时执行
    // 这里可以存储清除值供后续使用
    (void)r; (void)g; (void)b; (void)a; (void)depth; (void)stencil;
}

static void vk_framebuffer_set_viewport(struct wlf_framebuffer* fb, struct wlf_rect viewport) {
    if (!fb) return;

    // Vulkan 中视口通过命令缓冲设置
    // 这里只是存储视口信息
    fb->viewport = viewport;
}

static bool vk_framebuffer_read_pixels(struct wlf_framebuffer* fb,
                                      struct wlf_rect region,
                                      enum wlf_framebuffer_format format,
                                      void* data) {
    if (!fb || !data) return false;

    // Vulkan 中读取像素需要创建暂存缓冲区并执行拷贝命令
    // 这是一个复杂的操作，这里简化实现
    return false;
}

// Vulkan 虚函数表
static const struct wlf_framebuffer_vtable vk_framebuffer_vtable = {
    .destroy = vk_framebuffer_destroy,
    .bind = vk_framebuffer_bind,
    .unbind = vk_framebuffer_unbind,
    .attach_color = vk_framebuffer_attach_color,
    .attach_depth = vk_framebuffer_attach_depth,
    .attach_stencil = vk_framebuffer_attach_stencil,
    .is_complete = vk_framebuffer_is_complete,
    .clear = vk_framebuffer_clear,
    .set_viewport = vk_framebuffer_set_viewport,
    .read_pixels = vk_framebuffer_read_pixels,
};

// ===== 公共函数 =====

struct wlf_framebuffer* wlf_vk_framebuffer_create(struct wlf_render_context* context,
                                                 int width, int height,
                                                 enum wlf_framebuffer_format format) {
    if (!context || width <= 0 || height <= 0) {
        return NULL;
    }

    struct wlf_vk_framebuffer* vk_fb = calloc(1, sizeof(struct wlf_vk_framebuffer));
    if (!vk_fb) {
        return NULL;
    }

    // 初始化基类
    vk_fb->base.vtable = &vk_framebuffer_vtable;
    vk_fb->base.width = width;
    vk_fb->base.height = height;
    vk_fb->base.format = format;
    vk_fb->base.context = context;
    vk_fb->base.is_bound = false;
    vk_fb->base.viewport = (struct wlf_rect){0, 0, width, height};

    // 从上下文获取Vulkan设备
    // vk_fb->device = wlf_render_context_get_vk_device(context);
    // vk_fb->physical_device = wlf_render_context_get_vk_physical_device(context);

    // 暂时设置为空
    vk_fb->device = VK_NULL_HANDLE;
    vk_fb->physical_device = VK_NULL_HANDLE;
    vk_fb->framebuffer = VK_NULL_HANDLE;
    vk_fb->render_pass = VK_NULL_HANDLE;

    vk_fb->num_color_attachments = 0;

    return (struct wlf_framebuffer*)vk_fb;
}

VkFormat wlf_vk_framebuffer_convert_format(enum wlf_framebuffer_format format) {
    switch (format) {
        case WLF_FB_FORMAT_RGBA8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case WLF_FB_FORMAT_RGBA16F:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case WLF_FB_FORMAT_RGBA32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case WLF_FB_FORMAT_RGB8:
            return VK_FORMAT_R8G8B8_UNORM;
        case WLF_FB_FORMAT_DEPTH24:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case WLF_FB_FORMAT_DEPTH32F:
            return VK_FORMAT_D32_SFLOAT;
        case WLF_FB_FORMAT_STENCIL8:
            return VK_FORMAT_S8_UINT;
        default:
            return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

VkResult wlf_vk_framebuffer_create_image(VkDevice device,
                                        VkPhysicalDevice physical_device,
                                        uint32_t width, uint32_t height,
                                        VkFormat format,
                                        VkImageUsageFlags usage,
                                        VkImage* image,
                                        VkDeviceMemory* memory) {
    // 创建图像
    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkResult result = vkCreateImage(device, &image_info, NULL, image);
    if (result != VK_SUCCESS) {
        return result;
    }

    // 获取内存需求
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device, *image, &mem_requirements);

    // 分配内存
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = 0, // 需要查找合适的内存类型
    };

    result = vkAllocateMemory(device, &alloc_info, NULL, memory);
    if (result != VK_SUCCESS) {
        vkDestroyImage(device, *image, NULL);
        return result;
    }

    // 绑定内存
    return vkBindImageMemory(device, *image, *memory, 0);
}

VkResult wlf_vk_framebuffer_create_image_view(VkDevice device,
                                             VkImage image,
                                             VkFormat format,
                                             VkImageAspectFlags aspect_mask,
                                             VkImageView* view) {
    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange.aspectMask = aspect_mask,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };

    return vkCreateImageView(device, &view_info, NULL, view);
}
