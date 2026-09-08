/**
 * @file text.h
 * @brief Core Text rasterizer for macOS.
 */

#ifndef WLF_PLATFORM_MACOS_TEXT_H
#define WLF_PLATFORM_MACOS_TEXT_H

#include "wlf/platform/wlf_text.h"

struct wlf_macos_text {
	struct wlf_text base;
};

struct wlf_macos_text *wlf_macos_text_create(void);

#endif /* WLF_PLATFORM_MACOS_TEXT_H */
