/**
 * @file        wlf_vk_blit.h
 * @brief       Vulkan blit implementation for hardware-accelerated memory operations.
 * @details     This file provides Vulkan specific implementation of blit operations.
 *              It utilizes GPU capabilities for efficient data transfer between graphics resources.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef RENDER_WLF_VK_BLIT_H
#define RENDER_WLF_VK_BLIT_H

#include "wlf_blit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets the Vulkan blit implementation function table.
 *
 * Returns a pointer to the virtual function table containing Vulkan specific
 * implementations of all blit operations.
 *
 * @return Pointer to Vulkan blit function table.
 */
extern const struct wlf_blit_impl* wlf_vk_blit_get_vtable(void);

#ifdef __cplusplus
}
#endif

#endif // RENDER_WLF_VK_BLIT_H
