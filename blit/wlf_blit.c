#include "wlf/blit/wlf_blit.h"
#include "wlf/blit/wlf_gl_blit.h"
#include "wlf/framebuffer/wlf_framebuffer.h"
#include "wlf/texture/wlf_texture.h"

#include <stdlib.h>

struct wlf_blit* wlf_blit_create(struct wlf_render_context* context) {
	if (!context) {
		return NULL;
	}

	struct wlf_blit* blit = calloc(1, sizeof(struct wlf_blit));
	if (!blit) {
		return NULL;
	}

	blit->context = context;

	blit->impl = wlf_gl_blit_get_vtable();

	return blit;
}

void wlf_blit_destroy(struct wlf_blit* blit) {
	if (!blit) {
		return;
	}

	free(blit);
}

bool wlf_blit_framebuffer_to_framebuffer(struct wlf_blit* blit,
										struct wlf_framebuffer* src,
										struct wlf_framebuffer* dst,
										struct wlf_rect src_rect,
										struct wlf_rect dst_rect,
										enum wlf_blit_filter filter) {
	if (!blit || !blit->impl || !blit->impl->framebuffer_to_framebuffer) {
		return false;
	}

	return blit->impl->framebuffer_to_framebuffer(blit->context, src, dst, src_rect, dst_rect, filter);
}

bool wlf_blit_texture_to_framebuffer(struct wlf_blit* blit,
									struct wlf_texture* src,
									struct wlf_framebuffer* dst,
									struct wlf_rect src_rect,
									struct wlf_rect dst_rect,
									enum wlf_blit_filter filter) {
	if (!blit || !blit->impl || !blit->impl->texture_to_framebuffer) {
		return false;
	}

	return blit->impl->texture_to_framebuffer(blit->context, src, dst, src_rect, dst_rect, filter);
}

bool wlf_blit_framebuffer_to_texture(struct wlf_blit* blit,
									struct wlf_framebuffer* src,
									struct wlf_texture* dst,
									struct wlf_rect src_rect,
									struct wlf_rect dst_rect,
									enum wlf_blit_filter filter) {
	if (!blit || !blit->impl || !blit->impl->framebuffer_to_texture) {
		return false;
	}

	return blit->impl->framebuffer_to_texture(blit->context, src, dst, src_rect, dst_rect, filter);
}

bool wlf_blit_texture_to_texture(struct wlf_blit* blit,
								struct wlf_texture* src,
								struct wlf_texture* dst,
								struct wlf_rect src_rect,
								struct wlf_rect dst_rect,
								enum wlf_blit_filter filter) {
	if (!blit || !blit->impl || !blit->impl->texture_to_texture) {
		return false;
	}

	return blit->impl->texture_to_texture(blit->context, src, dst, src_rect, dst_rect, filter);
}

bool wlf_blit_framebuffer_full(struct wlf_blit* blit,
								struct wlf_framebuffer* src,
								struct wlf_framebuffer* dst,
								enum wlf_blit_filter filter) {
	if (!blit || !src || !dst) {
		return false;
	}

	struct wlf_rect src_rect = {0, 0, src->width, src->height};
	struct wlf_rect dst_rect = {0, 0, dst->width, dst->height};

	return wlf_blit_framebuffer_to_framebuffer(blit, src, dst, src_rect, dst_rect, filter);
}

bool wlf_blit_texture_full(struct wlf_blit* blit,
							struct wlf_texture* src,
							struct wlf_texture* dst,
							enum wlf_blit_filter filter) {
	if (!blit || !src || !dst) {
		return false;
	}

	struct wlf_rect src_rect = {0, 0, wlf_texture_get_width(src), wlf_texture_get_height(src)};
	struct wlf_rect dst_rect = {0, 0, wlf_texture_get_width(dst), wlf_texture_get_height(dst)};

	return wlf_blit_texture_to_texture(blit, src, dst, src_rect, dst_rect, filter);
}

void wlf_blit_sync(struct wlf_blit* blit) {
	if (!blit || !blit->impl || !blit->impl->sync) {
		return;
	}

	blit->impl->sync(blit->context);
}
