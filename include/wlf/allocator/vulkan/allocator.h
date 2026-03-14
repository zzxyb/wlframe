#ifndef VULKAN_ALLOCATOR_H
#define VULKAN_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <stdbool.h>

struct wlf_vk_allocator {
	struct wlf_allocator base;  /**< Base allocator structure */
};

struct wlf_allocator *wlf_vk_allocator_create(void);
struct wlf_vk_allocator *wlf_vk_allocator_from_allocator(
	struct wlf_allocator *allocator);
bool wlf_allocator_is_vk(const struct wlf_allocator *allocator);

#endif // VULKAN_ALLOCATOR_H
