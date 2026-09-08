/**
 * @file allocator.h
 * @brief Metal buffer allocator.
 */

#ifndef WLF_ALLOCATOR_METAL_ALLOCATOR_H
#define WLF_ALLOCATOR_METAL_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <stdbool.h>

struct wlf_mtl_device;

struct wlf_mtl_allocator {
	struct wlf_allocator base;
	struct wlf_mtl_device *device; /**< Borrowed from the renderer. */
};

struct wlf_mtl_allocator *wlf_mtl_allocator_create(
	struct wlf_mtl_device *device);

bool wlf_allocator_is_mtl(const struct wlf_allocator *allocator);

struct wlf_mtl_allocator *wlf_mtl_allocator_from_allocator(
	struct wlf_allocator *allocator);

#endif /* WLF_ALLOCATOR_METAL_ALLOCATOR_H */
