/**
 * @file output.h
 * @brief Win32 monitor implementation of wlf_output.
 */

#ifndef WLF_PLATFORM_WINDOWS_OUTPUT_H
#define WLF_PLATFORM_WINDOWS_OUTPUT_H

#include "wlf/types/wlf_output.h"

#include <stdbool.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct wlf_windows_output {
	struct wlf_output base;
	HMONITOR monitor;
};

struct wlf_output *wlf_windows_output_create(HMONITOR monitor);
bool wlf_output_is_windows(const struct wlf_output *output);
struct wlf_windows_output *wlf_windows_output_from_output(
	struct wlf_output *output);

#endif /* WLF_PLATFORM_WINDOWS_OUTPUT_H */
