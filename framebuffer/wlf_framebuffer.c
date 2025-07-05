#include "wlf/framebuffer/wlf_framebuffer.h"
#include "wlf/framebuffer/wlf_gl_framebuffer.h"
#include <stdlib.h>
#include <string.h>

// ===== 基础实现 =====

struct wlf_framebuffer* wlf_framebuffer_create(struct wlf_render_context* context,
                                              int width, int height,
                                              enum wlf_framebuffer_format format) {
    if (!context || width <= 0 || height <= 0) {
        return NULL;
    }

    // 根据渲染上下文类型选择实现
    // 这里需要检查上下文的API类型
    // 假设有个函数可以获取API类型
    // enum wlf_render_api api = wlf_render_context_get_api(context);

    // 暂时假设使用OpenGL ES
    return wlf_gl_framebuffer_create(context, width, height, format);
}

void wlf_framebuffer_destroy(struct wlf_framebuffer* fb) {
    if (!fb || !fb->vtable || !fb->vtable->destroy) {
        return;
    }

    fb->vtable->destroy(fb);
}

bool wlf_framebuffer_bind(struct wlf_framebuffer* fb) {
    if (!fb || !fb->vtable || !fb->vtable->bind) {
        return false;
    }

    return fb->vtable->bind(fb);
}

void wlf_framebuffer_unbind(struct wlf_framebuffer* fb) {
    if (!fb || !fb->vtable || !fb->vtable->unbind) {
        return;
    }

    fb->vtable->unbind(fb);
}

bool wlf_framebuffer_attach_color(struct wlf_framebuffer* fb,
                                 enum wlf_framebuffer_attachment attachment,
                                 struct wlf_texture* texture,
                                 int mip_level) {
    if (!fb || !fb->vtable || !fb->vtable->attach_color) {
        return false;
    }

    return fb->vtable->attach_color(fb, attachment, texture, mip_level);
}

bool wlf_framebuffer_attach_depth(struct wlf_framebuffer* fb,
                                 struct wlf_texture* texture,
                                 int mip_level) {
    if (!fb || !fb->vtable || !fb->vtable->attach_depth) {
        return false;
    }

    return fb->vtable->attach_depth(fb, texture, mip_level);
}

bool wlf_framebuffer_attach_stencil(struct wlf_framebuffer* fb,
                                   struct wlf_texture* texture,
                                   int mip_level) {
    if (!fb || !fb->vtable || !fb->vtable->attach_stencil) {
        return false;
    }

    return fb->vtable->attach_stencil(fb, texture, mip_level);
}

bool wlf_framebuffer_is_complete(struct wlf_framebuffer* fb) {
    if (!fb || !fb->vtable || !fb->vtable->is_complete) {
        return false;
    }

    return fb->vtable->is_complete(fb);
}

struct wlf_vector2 wlf_framebuffer_get_size(const struct wlf_framebuffer* fb) {
    if (!fb) {
        return (struct wlf_vector2){0.0f, 0.0f};
    }

    return (struct wlf_vector2){(float)fb->width, (float)fb->height};
}

enum wlf_framebuffer_format wlf_framebuffer_get_format(const struct wlf_framebuffer* fb) {
    return fb ? fb->format : WLF_FB_FORMAT_RGBA8;
}

void wlf_framebuffer_clear(struct wlf_framebuffer* fb,
                          float r, float g, float b, float a,
                          float depth, int stencil) {
    if (!fb || !fb->vtable || !fb->vtable->clear) {
        return;
    }

    fb->vtable->clear(fb, r, g, b, a, depth, stencil);
}

void wlf_framebuffer_set_viewport(struct wlf_framebuffer* fb, struct wlf_rect viewport) {
    if (!fb || !fb->vtable || !fb->vtable->set_viewport) {
        return;
    }

    fb->viewport = viewport;
    fb->vtable->set_viewport(fb, viewport);
}

bool wlf_framebuffer_read_pixels(struct wlf_framebuffer* fb,
                                struct wlf_rect region,
                                enum wlf_framebuffer_format format,
                                void* data) {
    if (!fb || !fb->vtable || !fb->vtable->read_pixels || !data) {
        return false;
    }

    return fb->vtable->read_pixels(fb, region, format, data);
}
