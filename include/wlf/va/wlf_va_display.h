#ifndef VA_WLF_VA_DISPLAY_H
#define VA_WLF_VA_DISPLAY_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/platform/wlf_backend.h"

#include <stdbool.h>

#include <va/va.h>

struct wlf_va_display;

struct wlf_va_display_impl {
	const char *name;
	void (*destroy)(struct wlf_va_display *display);
};

struct wlf_va_display {
	const struct wlf_va_display_impl *impl;
	VADisplay display;
	struct wlf_backend *backend;
	struct {
		struct wlf_signal destroy;
	} events;
};

struct wlf_va_display *wlf_va_display_autocreate(struct wlf_backend *backend);

void wlf_va_display_init(struct wlf_va_display *display,
	const struct wlf_va_display_impl *impl, struct wlf_backend *backend);

void wlf_va_display_destroy(struct wlf_va_display *display);

#endif // VA_WLF_VA_DISPLAY_H
