/**
 * @file pointer.h
 * @brief AppKit pointer and cursor objects.
 */

#ifndef WLF_TYPES_MACOS_POINTER_H
#define WLF_TYPES_MACOS_POINTER_H

#include "wlf/types/wlf_pointer.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_macos_pointer {
	struct wlf_pointer base;
	uint32_t serial;
};

struct wlf_macos_pointer *wlf_macos_pointer_create(void);

bool wlf_pointer_is_macos(const struct wlf_pointer *pointer);

struct wlf_macos_pointer *wlf_macos_pointer_from_pointer(
	struct wlf_pointer *pointer);

/** Advances and returns the non-zero serial used by native input events. */
uint32_t wlf_macos_pointer_next_serial(struct wlf_macos_pointer *pointer);

#endif /* WLF_TYPES_MACOS_POINTER_H */
