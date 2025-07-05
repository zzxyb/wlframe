#ifndef RENDER_WLF_VK_FRAMEBUFFER_H
#define RENDER_WLF_VK_FRAMEBUFFER_H

#include "wlf_framebuffer.h"
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file wlf_vk_framebuffer.h
 * @brief Vulkan framebuffer implementation
 *
 * Vulkan 帧缓冲的具体实现
 */

/**
 * @brief Vulkan 帧缓冲实现
 */
struct wlf_vk_framebuffer {
    struct wlf_framebuffer base;    /**< 基类 */

    VkFramebuffer framebuffer;      /**< Vulkan 帧缓冲对象 */
    VkRenderPass render_pass;       /**< 渲染通道 */

    VkImage color_images[4];        /**< 颜色图像 */
    VkImageView color_views[4];     /**< 颜色图像视图 */
    VkDeviceMemory color_memory[4]; /**< 颜色图像内存 */

    VkImage depth_image;            /**< 深度图像 */
    VkImageView depth_view;         /**< 深度图像视图 */
    VkDeviceMemory depth_memory;    /**< 深度图像内存 */

    VkImage stencil_image;          /**< 模板图像 */
    VkImageView stencil_view;       /**< 模板图像视图 */
    VkDeviceMemory stencil_memory;  /**< 模板图像内存 */

    VkDevice device;                /**< Vulkan 设备 */
    VkPhysicalDevice physical_device; /**< 物理设备 */

    int num_color_attachments;      /**< 颜色附件数量 */
};

// ===== 创建和销毁 =====

/**
 * @brief 创建Vulkan帧缓冲
 * @param context 渲染上下文
 * @param width 宽度
 * @param height 高度
 * @param format 格式
 * @return 新创建的帧缓冲指针，失败时返回 NULL
 */
struct wlf_framebuffer* wlf_vk_framebuffer_create(struct wlf_render_context* context,
                                                 int width, int height,
                                                 enum wlf_framebuffer_format format);

// ===== 内部函数 =====

/**
 * @brief 转换格式到Vulkan格式
 * @param format 通用格式
 * @return Vulkan格式
 */
VkFormat wlf_vk_framebuffer_convert_format(enum wlf_framebuffer_format format);

/**
 * @brief 创建图像和内存
 * @param device Vulkan设备
 * @param physical_device 物理设备
 * @param width 宽度
 * @param height 高度
 * @param format Vulkan格式
 * @param usage 用途标志
 * @param image 输出图像
 * @param memory 输出内存
 * @return 成功返回 VK_SUCCESS
 */
VkResult wlf_vk_framebuffer_create_image(VkDevice device,
                                        VkPhysicalDevice physical_device,
                                        uint32_t width, uint32_t height,
                                        VkFormat format,
                                        VkImageUsageFlags usage,
                                        VkImage* image,
                                        VkDeviceMemory* memory);

/**
 * @brief 创建图像视图
 * @param device Vulkan设备
 * @param image 图像
 * @param format 格式
 * @param aspect_mask 方面掩码
 * @param view 输出视图
 * @return 成功返回 VK_SUCCESS
 */
VkResult wlf_vk_framebuffer_create_image_view(VkDevice device,
                                             VkImage image,
                                             VkFormat format,
                                             VkImageAspectFlags aspect_mask,
                                             VkImageView* view);

#ifdef __cplusplus
}
#endif

#endif // RENDER_WLF_VK_FRAMEBUFFER_H
