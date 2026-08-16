/**
 * @file text.h
 * @brief Native Windows text rasterizer.
 */

#ifndef WLF_PLATFORM_WINDOWS_TEXT_H
#define WLF_PLATFORM_WINDOWS_TEXT_H

#include "wlf/platform/wlf_text.h"

struct wlf_windows_text {
	struct wlf_text base;
};

struct wlf_windows_text *wlf_windows_text_create(void);

#endif /* WLF_PLATFORM_WINDOWS_TEXT_H */
