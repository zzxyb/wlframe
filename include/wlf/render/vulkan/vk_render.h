#ifndef VULKAN_VK_RENDER_H
#define VULKAN_VK_RENDER_H

#include "wlf/render/wlf_render.h"
#include "wlf/utils/wlf_log.h"

#include <vulkan/vulkan.h>

#include <stdbool.h>

struct wlf_vk_instance;
struct wlf_vk_device;
struct wlf_backend;

struct wlf_vk_render {
	struct wlf_render base;
	struct wlf_backend *backend;
	struct wlf_vk_device *dev;

	VkCommandPool command_pool;

	VkSemaphore timeline_semaphore;
};

struct wlf_render *wlf_vk_render_create_from_backend(
	struct wlf_backend *backend);
void wlf_vk_render_destroy(struct wlf_vk_render *vk_render);

bool wlf_render_is_vk(struct wlf_render *wlf_render);
struct wlf_vk_render *wlf_vk_render_from_render(struct wlf_render *wlf_render);

bool check_extension(const VkExtensionProperties *avail,
	uint32_t avail_len, const char *name);
struct wlf_render *wlr_vk_render_create_for_device(struct wlf_vk_device *dev);

const char *wlf_vulkan_strerror(VkResult err);

#if __STDC_VERSION__ >= 202311L

#define wlf_vk_error(fmt, res, ...) wlf_log(WLF_ERROR, fmt ": %s (%d)", \
	wlf_vulkan_strerror(res), res __VA_OPT__(,) __VA_ARGS__)

#else

#define wlf_vk_error(fmt, res, ...) wlf_log(WLF_ERROR, fmt ": %s (%d)", \
	wlf_vulkan_strerror(res), res, ##__VA_ARGS__)

#endif

#endif // VULKAN_VK_RENDER_H
