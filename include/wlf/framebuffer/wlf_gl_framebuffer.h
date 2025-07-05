#ifndef RENDER_WLF_GL_FRAMEBUFFER_H
#define RENDER_WLF_GL_FRAMEBUFFER_H

#include "wlf_framebuffer.h"
#include <GLES3/gl3.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file wlf_gl_framebuffer.h
 * @brief OpenGL ES framebuffer implementation
 *
 * OpenGL ES 帧缓冲的具体实现
 */

/**
 * @brief OpenGL ES 帧缓冲实现
 */
struct wlf_gl_framebuffer {
    struct wlf_framebuffer base;    /**< 基类 */

    GLuint fbo;                     /**< OpenGL 帧缓冲对象 */
    GLuint color_textures[4];       /**< 颜色纹理ID */
    GLuint depth_texture;           /**< 深度纹理ID */
    GLuint stencil_texture;         /**< 模板纹理ID */

    GLenum draw_buffers[4];         /**< 绘制缓冲区 */
    int num_color_attachments;      /**< 颜色附件数量 */
};

// ===== 创建和销毁 =====

/**
 * @brief 创建OpenGL ES帧缓冲
 * @param context 渲染上下文
 * @param width 宽度
 * @param height 高度
 * @param format 格式
 * @return 新创建的帧缓冲指针，失败时返回 NULL
 */
struct wlf_framebuffer* wlf_gl_framebuffer_create(struct wlf_render_context* context,
                                                 int width, int height,
                                                 enum wlf_framebuffer_format format);

// ===== 内部函数 =====

/**
 * @brief 转换格式到OpenGL格式
 * @param format 通用格式
 * @param gl_format 输出OpenGL内部格式
 * @param gl_type 输出OpenGL数据类型
 * @param gl_external_format 输出OpenGL外部格式
 */
void wlf_gl_framebuffer_convert_format(enum wlf_framebuffer_format format,
                                      GLenum* gl_format,
                                      GLenum* gl_type,
                                      GLenum* gl_external_format);

#ifdef __cplusplus
}
#endif

#endif // RENDER_WLF_GL_FRAMEBUFFER_H
