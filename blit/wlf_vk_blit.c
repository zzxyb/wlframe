#include "wlf/blit/wlf_blit.h"
#include "wlf/blit/wlf_vk_blit.h"
#include "wlf/framebuffer/wlf_vk_framebuffer.h"
#include "wlf/texture/wlf_vk_texture.h"

#include <vulkan/vulkan.h>

static bool vk_blit_framebuffer_to_framebuffer(struct wlf_render_context* context,
												struct wlf_framebuffer* src,
												struct wlf_framebuffer* dst,
												struct wlf_rect src_rect,
												struct wlf_rect dst_rect,
												enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	struct wlf_vk_framebuffer* vk_src = (struct wlf_vk_framebuffer*)src;
	struct wlf_vk_framebuffer* vk_dst = (struct wlf_vk_framebuffer*)dst;

	VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;

	if (cmd_buffer == VK_NULL_HANDLE) {
		return false;
	}

	VkImageBlit blit_region = {0};

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.mipLevel = 0;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcOffsets[0] = (VkOffset3D){src_rect.x, src_rect.y, 0};
	blit_region.srcOffsets[1] = (VkOffset3D){src_rect.x + src_rect.width, src_rect.y + src_rect.height, 1};

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.mipLevel = 0;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstOffsets[0] = (VkOffset3D){dst_rect.x, dst_rect.y, 0};
	blit_region.dstOffsets[1] = (VkOffset3D){dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height, 1};

	VkFilter vk_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

	vkCmdBlitImage(
		cmd_buffer,
		vk_src->color_images[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		vk_dst->color_images[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit_region,
		vk_filter
	);

	return true;
}

static bool vk_blit_texture_to_framebuffer(struct wlf_render_context* context,
											struct wlf_texture* src,
											struct wlf_framebuffer* dst,
											struct wlf_rect src_rect,
											struct wlf_rect dst_rect,
											enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	struct wlf_vk_framebuffer* vk_dst = (struct wlf_vk_framebuffer*)dst;
	VkImage src_image = wlf_texture_get_vk_image(src);

	if (src_image == VK_NULL_HANDLE) {
		return false;
	}

	VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;

	if (cmd_buffer == VK_NULL_HANDLE) {
		return false;
	}

	VkImageBlit blit_region = {0};

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.mipLevel = 0;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcOffsets[0] = (VkOffset3D){src_rect.x, src_rect.y, 0};
	blit_region.srcOffsets[1] = (VkOffset3D){src_rect.x + src_rect.width, src_rect.y + src_rect.height, 1};

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.mipLevel = 0;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstOffsets[0] = (VkOffset3D){dst_rect.x, dst_rect.y, 0};
	blit_region.dstOffsets[1] = (VkOffset3D){dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height, 1};

	VkFilter vk_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

	vkCmdBlitImage(
		cmd_buffer,
		src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		vk_dst->color_images[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit_region,
		vk_filter
	);

	return true;
}

static bool vk_blit_framebuffer_to_texture(struct wlf_render_context* context,
											struct wlf_framebuffer* src,
											struct wlf_texture* dst,
											struct wlf_rect src_rect,
											struct wlf_rect dst_rect,
											enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	struct wlf_vk_framebuffer* vk_src = (struct wlf_vk_framebuffer*)src;
	VkImage dst_image = wlf_texture_get_vk_image(dst);

	if (dst_image == VK_NULL_HANDLE) {
		return false;
	}

	VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;

	if (cmd_buffer == VK_NULL_HANDLE) {
		return false;
	}

	VkImageBlit blit_region = {0};

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.mipLevel = 0;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcOffsets[0] = (VkOffset3D){src_rect.x, src_rect.y, 0};
	blit_region.srcOffsets[1] = (VkOffset3D){src_rect.x + src_rect.width, src_rect.y + src_rect.height, 1};

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.mipLevel = 0;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstOffsets[0] = (VkOffset3D){dst_rect.x, dst_rect.y, 0};
	blit_region.dstOffsets[1] = (VkOffset3D){dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height, 1};

	VkFilter vk_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

	vkCmdBlitImage(
		cmd_buffer,
		vk_src->color_images[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit_region,
		vk_filter
	);

	return true;
}

static bool vk_blit_texture_to_texture(struct wlf_render_context* context,
										struct wlf_texture* src,
										struct wlf_texture* dst,
										struct wlf_rect src_rect,
										struct wlf_rect dst_rect,
										enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	VkImage src_image = wlf_texture_get_vk_image(src);
	VkImage dst_image = wlf_texture_get_vk_image(dst);

	if (src_image == VK_NULL_HANDLE || dst_image == VK_NULL_HANDLE) {
		return false;
	}

	VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;

	if (cmd_buffer == VK_NULL_HANDLE) {
		return false;
	}

	VkImageBlit blit_region = {0};

	blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.srcSubresource.mipLevel = 0;
	blit_region.srcSubresource.baseArrayLayer = 0;
	blit_region.srcSubresource.layerCount = 1;
	blit_region.srcOffsets[0] = (VkOffset3D){src_rect.x, src_rect.y, 0};
	blit_region.srcOffsets[1] = (VkOffset3D){src_rect.x + src_rect.width, src_rect.y + src_rect.height, 1};

	blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit_region.dstSubresource.mipLevel = 0;
	blit_region.dstSubresource.baseArrayLayer = 0;
	blit_region.dstSubresource.layerCount = 1;
	blit_region.dstOffsets[0] = (VkOffset3D){dst_rect.x, dst_rect.y, 0};
	blit_region.dstOffsets[1] = (VkOffset3D){dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height, 1};

	VkFilter vk_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

	vkCmdBlitImage(
		cmd_buffer,
		src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit_region,
		vk_filter
	);

	return true;
}

static void vk_blit_sync(struct wlf_render_context* context) {
	(void)context;
}

static const struct wlf_blit_impl vk_blit_vtable = {
	.framebuffer_to_framebuffer = vk_blit_framebuffer_to_framebuffer,
	.texture_to_framebuffer = vk_blit_texture_to_framebuffer,
	.framebuffer_to_texture = vk_blit_framebuffer_to_texture,
	.texture_to_texture = vk_blit_texture_to_texture,
	.sync = vk_blit_sync,
};

const struct wlf_blit_impl* wlf_vk_blit_get_vtable(void) {
	return &vk_blit_vtable;
}
