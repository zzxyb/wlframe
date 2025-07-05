#include "wlf/framebuffer/wlf_gl_framebuffer.h"
#include "wlf/texture/wlf_gl_texture.h"

#include <stdlib.h>
#include <string.h>

static void gl_framebuffer_destroy(struct wlf_framebuffer* fb) {
    if (!fb) return;

    struct wlf_gl_framebuffer* gl_fb = (struct wlf_gl_framebuffer*)fb;

    if (gl_fb->fbo != 0) {
        glDeleteFramebuffers(1, &gl_fb->fbo);
    }

    // 删除纹理（如果是我们创建的）
    for (int i = 0; i < 4; i++) {
        if (gl_fb->color_textures[i] != 0) {
            glDeleteTextures(1, &gl_fb->color_textures[i]);
        }
    }

    if (gl_fb->depth_texture != 0) {
        glDeleteTextures(1, &gl_fb->depth_texture);
    }

    if (gl_fb->stencil_texture != 0) {
        glDeleteTextures(1, &gl_fb->stencil_texture);
    }

    free(gl_fb);
}

static bool gl_framebuffer_bind(struct wlf_framebuffer* fb) {
    if (!fb) return false;

    struct wlf_gl_framebuffer* gl_fb = (struct wlf_gl_framebuffer*)fb;

    glBindFramebuffer(GL_FRAMEBUFFER, gl_fb->fbo);

    // 设置绘制缓冲区
    if (gl_fb->num_color_attachments > 0) {
        glDrawBuffers(gl_fb->num_color_attachments, gl_fb->draw_buffers);
    } else {
        // OpenGL ES 不支持 glDrawBuffer，使用 glDrawBuffers
        GLenum none_buffer = GL_NONE;
        glDrawBuffers(1, &none_buffer);
    }

    fb->is_bound = true;
    return true;
}

static void gl_framebuffer_unbind(struct wlf_framebuffer* fb) {
    if (!fb) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fb->is_bound = false;
}

static bool gl_framebuffer_attach_color(struct wlf_framebuffer* fb,
                                       enum wlf_framebuffer_attachment attachment,
                                       struct wlf_texture* texture,
                                       int mip_level) {
    if (!fb || attachment > WLF_FB_ATTACHMENT_COLOR3) return false;

    struct wlf_gl_framebuffer* gl_fb = (struct wlf_gl_framebuffer*)fb;

    GLuint texture_id = wlf_texture_get_gl_id(texture);

    GLenum color_attachment = GL_COLOR_ATTACHMENT0 + (attachment - WLF_FB_ATTACHMENT_COLOR0);

    glBindFramebuffer(GL_FRAMEBUFFER, gl_fb->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment, GL_TEXTURE_2D, texture_id, mip_level);

    // 更新绘制缓冲区数组
    int index = attachment - WLF_FB_ATTACHMENT_COLOR0;
    gl_fb->draw_buffers[index] = color_attachment;
    if (index >= gl_fb->num_color_attachments) {
        gl_fb->num_color_attachments = index + 1;
    }

    fb->color_attachments[index] = texture;

    return true;
}

static bool gl_framebuffer_attach_depth(struct wlf_framebuffer* fb,
                                       struct wlf_texture* texture,
                                       int mip_level) {
    if (!fb) return false;

    struct wlf_gl_framebuffer* gl_fb = (struct wlf_gl_framebuffer*)fb;

    GLuint texture_id = wlf_texture_get_gl_id(texture);

    glBindFramebuffer(GL_FRAMEBUFFER, gl_fb->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_id, mip_level);

    fb->depth_attachment = texture;

    return true;
}

static bool gl_framebuffer_attach_stencil(struct wlf_framebuffer* fb,
                                         struct wlf_texture* texture,
                                         int mip_level) {
    if (!fb) return false;

    struct wlf_gl_framebuffer* gl_fb = (struct wlf_gl_framebuffer*)fb;

    GLuint texture_id = wlf_texture_get_gl_id(texture);

    glBindFramebuffer(GL_FRAMEBUFFER, gl_fb->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture_id, mip_level);

    fb->stencil_attachment = texture;

    return true;
}

static bool gl_framebuffer_is_complete(struct wlf_framebuffer* fb) {
    if (!fb) return false;

    struct wlf_gl_framebuffer* gl_fb = (struct wlf_gl_framebuffer*)fb;

    glBindFramebuffer(GL_FRAMEBUFFER, gl_fb->fbo);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    return status == GL_FRAMEBUFFER_COMPLETE;
}

static void gl_framebuffer_clear(struct wlf_framebuffer* fb,
                                float r, float g, float b, float a,
                                float depth, int stencil) {
    if (!fb) return;

    glClearColor(r, g, b, a);
    glClearDepthf(depth);
    glClearStencil(stencil);

    GLbitfield mask = GL_COLOR_BUFFER_BIT;
    if (fb->depth_attachment) {
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (fb->stencil_attachment) {
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);
}

static void gl_framebuffer_set_viewport(struct wlf_framebuffer* fb, struct wlf_rect viewport) {
    if (!fb) return;

    glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

static bool gl_framebuffer_read_pixels(struct wlf_framebuffer* fb,
                                      struct wlf_rect region,
                                      enum wlf_framebuffer_format format,
                                      void* data) {
    if (!fb || !data) return false;

    GLenum gl_format, gl_type, gl_external_format;
    wlf_gl_framebuffer_convert_format(format, &gl_format, &gl_type, &gl_external_format);

    glReadPixels(region.x, region.y, region.width, region.height,
                gl_external_format, gl_type, data);

    return glGetError() == GL_NO_ERROR;
}

// 虚函数表
static const struct wlf_framebuffer_vtable gl_framebuffer_vtable = {
    .destroy = gl_framebuffer_destroy,
    .bind = gl_framebuffer_bind,
    .unbind = gl_framebuffer_unbind,
    .attach_color = gl_framebuffer_attach_color,
    .attach_depth = gl_framebuffer_attach_depth,
    .attach_stencil = gl_framebuffer_attach_stencil,
    .is_complete = gl_framebuffer_is_complete,
    .clear = gl_framebuffer_clear,
    .set_viewport = gl_framebuffer_set_viewport,
    .read_pixels = gl_framebuffer_read_pixels,
};

// ===== 公共函数 =====

struct wlf_framebuffer* wlf_gl_framebuffer_create(struct wlf_render_context* context,
                                                 int width, int height,
                                                 enum wlf_framebuffer_format format) {
    if (!context || width <= 0 || height <= 0) {
        return NULL;
    }

    struct wlf_gl_framebuffer* gl_fb = calloc(1, sizeof(struct wlf_gl_framebuffer));
    if (!gl_fb) {
        return NULL;
    }

    // 初始化基类
    gl_fb->base.vtable = &gl_framebuffer_vtable;
    gl_fb->base.width = width;
    gl_fb->base.height = height;
    gl_fb->base.format = format;
    gl_fb->base.context = context;
    gl_fb->base.is_bound = false;
    gl_fb->base.viewport = (struct wlf_rect){0, 0, width, height};

    // 创建OpenGL帧缓冲对象
    glGenFramebuffers(1, &gl_fb->fbo);
    if (gl_fb->fbo == 0) {
        free(gl_fb);
        return NULL;
    }

    // 初始化绘制缓冲区数组
    for (int i = 0; i < 4; i++) {
        gl_fb->draw_buffers[i] = GL_NONE;
    }
    gl_fb->num_color_attachments = 0;

    return (struct wlf_framebuffer*)gl_fb;
}

void wlf_gl_framebuffer_convert_format(enum wlf_framebuffer_format format,
                                      GLenum* gl_format,
                                      GLenum* gl_type,
                                      GLenum* gl_external_format) {
    switch (format) {
        case WLF_FB_FORMAT_RGBA8:
            *gl_format = GL_RGBA8;
            *gl_type = GL_UNSIGNED_BYTE;
            *gl_external_format = GL_RGBA;
            break;

        case WLF_FB_FORMAT_RGBA16F:
            *gl_format = GL_RGBA16F;
            *gl_type = GL_HALF_FLOAT;
            *gl_external_format = GL_RGBA;
            break;

        case WLF_FB_FORMAT_RGBA32F:
            *gl_format = GL_RGBA32F;
            *gl_type = GL_FLOAT;
            *gl_external_format = GL_RGBA;
            break;

        case WLF_FB_FORMAT_RGB8:
            *gl_format = GL_RGB8;
            *gl_type = GL_UNSIGNED_BYTE;
            *gl_external_format = GL_RGB;
            break;

        case WLF_FB_FORMAT_DEPTH24:
            *gl_format = GL_DEPTH_COMPONENT24;
            *gl_type = GL_UNSIGNED_INT;
            *gl_external_format = GL_DEPTH_COMPONENT;
            break;

        case WLF_FB_FORMAT_DEPTH32F:
            *gl_format = GL_DEPTH_COMPONENT32F;
            *gl_type = GL_FLOAT;
            *gl_external_format = GL_DEPTH_COMPONENT;
            break;

        case WLF_FB_FORMAT_STENCIL8:
            *gl_format = GL_STENCIL_INDEX8;
            *gl_type = GL_UNSIGNED_BYTE;
            *gl_external_format = GL_STENCIL_INDEX8;
            break;

        default:
            *gl_format = GL_RGBA8;
            *gl_type = GL_UNSIGNED_BYTE;
            *gl_external_format = GL_RGBA;
            break;
    }
}
