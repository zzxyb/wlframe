
#ifndef TYPES_WLF_CAPTURE_H
#define TYPES_WLF_CAPTURE_H

struct wlf_capture;

struct wlf_capture_impl {
	void (*destroy)(struct wlf_capture *capture);
};

struct wlf_capture {
	const struct wlf_capture_impl *impl;
};

void wlf_capture_init(struct wlf_capture *capture,
	const struct wlf_capture_impl *impl);
void wlf_capture_destroy(struct wlf_capture *capture);

#endif // TYPES_WLF_CAPTURE_H
