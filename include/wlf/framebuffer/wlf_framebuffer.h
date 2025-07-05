#ifndef RENDER_WLF_FRAMEBUFFER_H
#define RENDER_WLF_FRAMEBUFFER_H

#include "../math/wlf_rect.h"
#include "../math/wlf_vector2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file wlf_framebuffer.h
 * @brief Framebuffer abstraction layer
 *
 * 提供跨图形API的帧缓冲抽象，支持OpenGL ES和Vulkan
 */

// 前向声明
struct wlf_texture;
struct wlf_render_context;
struct wlf_framebuffer;

/**
 * @brief 帧缓冲格式
 */
enum wlf_framebuffer_format {
    WLF_FB_FORMAT_RGBA8 = 0,        /**< RGBA 8位 */
    WLF_FB_FORMAT_RGBA16F,          /**< RGBA 16位浮点 */
    WLF_FB_FORMAT_RGBA32F,          /**< RGBA 32位浮点 */
    WLF_FB_FORMAT_RGB8,             /**< RGB 8位 */
    WLF_FB_FORMAT_DEPTH24,          /**< 24位深度 */
    WLF_FB_FORMAT_DEPTH32F,         /**< 32位浮点深度 */
    WLF_FB_FORMAT_STENCIL8,         /**< 8位模板 */
};

/**
 * @brief 帧缓冲附件类型
 */
enum wlf_framebuffer_attachment {
    WLF_FB_ATTACHMENT_COLOR0 = 0,   /**< 颜色附件0 */
    WLF_FB_ATTACHMENT_COLOR1,       /**< 颜色附件1 */
    WLF_FB_ATTACHMENT_COLOR2,       /**< 颜色附件2 */
    WLF_FB_ATTACHMENT_COLOR3,       /**< 颜色附件3 */
    WLF_FB_ATTACHMENT_DEPTH,        /**< 深度附件 */
    WLF_FB_ATTACHMENT_STENCIL,      /**< 模板附件 */
    WLF_FB_ATTACHMENT_DEPTH_STENCIL, /**< 深度模板附件 */
};

/**
 * @brief 帧缓冲操作接口
 */
struct wlf_framebuffer_vtable {
    /** @brief 销毁帧缓冲 */
    void (*destroy)(struct wlf_framebuffer* fb);
    
    /** @brief 绑定帧缓冲 */
    bool (*bind)(struct wlf_framebuffer* fb);
    
    /** @brief 解绑帧缓冲 */
    void (*unbind)(struct wlf_framebuffer* fb);
    
    /** @brief 添加颜色附件 */
    bool (*attach_color)(struct wlf_framebuffer* fb, 
                        enum wlf_framebuffer_attachment attachment,
                        struct wlf_texture* texture,
                        int mip_level);
    
    /** @brief 添加深度附件 */
    bool (*attach_depth)(struct wlf_framebuffer* fb, 
                        struct wlf_texture* texture,
                        int mip_level);
    
    /** @brief 添加模板附件 */
    bool (*attach_stencil)(struct wlf_framebuffer* fb, 
                          struct wlf_texture* texture,
                          int mip_level);
    
    /** @brief 检查帧缓冲完整性 */
    bool (*is_complete)(struct wlf_framebuffer* fb);
    
    /** @brief 清除帧缓冲 */
    void (*clear)(struct wlf_framebuffer* fb, 
                  float r, float g, float b, float a,
                  float depth, int stencil);
    
    /** @brief 设置视口 */
    void (*set_viewport)(struct wlf_framebuffer* fb, struct wlf_rect viewport);
    
    /** @brief 读取像素数据 */
    bool (*read_pixels)(struct wlf_framebuffer* fb,
                       struct wlf_rect region,
                       enum wlf_framebuffer_format format,
                       void* data);
};

/**
 * @brief 帧缓冲基类
 */
struct wlf_framebuffer {
    const struct wlf_framebuffer_vtable* vtable; /**< 虚函数表 */
    
    int width;                      /**< 宽度 */
    int height;                     /**< 高度 */
    enum wlf_framebuffer_format format; /**< 格式 */
    
    struct wlf_render_context* context; /**< 渲染上下文 */
    
    bool is_bound;                  /**< 是否已绑定 */
    struct wlf_rect viewport;       /**< 视口 */
    
    // 附件纹理
    struct wlf_texture* color_attachments[4]; /**< 颜色附件 */
    struct wlf_texture* depth_attachment;     /**< 深度附件 */
    struct wlf_texture* stencil_attachment;   /**< 模板附件 */
};

// ===== 创建和销毁 =====

/**
 * @brief 创建帧缓冲
 * @param context 渲染上下文
 * @param width 宽度
 * @param height 高度
 * @param format 格式
 * @return 新创建的帧缓冲指针，失败时返回 NULL
 */
struct wlf_framebuffer* wlf_framebuffer_create(struct wlf_render_context* context,
                                              int width, int height,
                                              enum wlf_framebuffer_format format);

/**
 * @brief 销毁帧缓冲
 * @param fb 帧缓冲指针
 */
void wlf_framebuffer_destroy(struct wlf_framebuffer* fb);

// ===== 绑定操作 =====

/**
 * @brief 绑定帧缓冲
 * @param fb 帧缓冲指针
 * @return 成功返回 true，失败返回 false
 */
bool wlf_framebuffer_bind(struct wlf_framebuffer* fb);

/**
 * @brief 解绑当前帧缓冲
 * @param fb 帧缓冲指针
 */
void wlf_framebuffer_unbind(struct wlf_framebuffer* fb);

// ===== 附件管理 =====

/**
 * @brief 添加颜色附件
 * @param fb 帧缓冲指针
 * @param attachment 附件索引
 * @param texture 纹理指针
 * @param mip_level mipmap 级别
 * @return 成功返回 true，失败返回 false
 */
bool wlf_framebuffer_attach_color(struct wlf_framebuffer* fb,
                                 enum wlf_framebuffer_attachment attachment,
                                 struct wlf_texture* texture,
                                 int mip_level);

/**
 * @brief 添加深度附件
 * @param fb 帧缓冲指针
 * @param texture 纹理指针
 * @param mip_level mipmap 级别
 * @return 成功返回 true，失败返回 false
 */
bool wlf_framebuffer_attach_depth(struct wlf_framebuffer* fb,
                                 struct wlf_texture* texture,
                                 int mip_level);

/**
 * @brief 添加模板附件
 * @param fb 帧缓冲指针
 * @param texture 纹理指针
 * @param mip_level mipmap 级别
 * @return 成功返回 true，失败返回 false
 */
bool wlf_framebuffer_attach_stencil(struct wlf_framebuffer* fb,
                                   struct wlf_texture* texture,
                                   int mip_level);

// ===== 状态查询 =====

/**
 * @brief 检查帧缓冲完整性
 * @param fb 帧缓冲指针
 * @return 完整返回 true，否则返回 false
 */
bool wlf_framebuffer_is_complete(struct wlf_framebuffer* fb);

/**
 * @brief 获取帧缓冲大小
 * @param fb 帧缓冲指针
 * @return 大小向量
 */
struct wlf_vector2 wlf_framebuffer_get_size(const struct wlf_framebuffer* fb);

/**
 * @brief 获取帧缓冲格式
 * @param fb 帧缓冲指针
 * @return 格式枚举
 */
enum wlf_framebuffer_format wlf_framebuffer_get_format(const struct wlf_framebuffer* fb);

// ===== 渲染操作 =====

/**
 * @brief 清除帧缓冲
 * @param fb 帧缓冲指针
 * @param r 红色分量 (0.0-1.0)
 * @param g 绿色分量 (0.0-1.0)
 * @param b 蓝色分量 (0.0-1.0)
 * @param a alpha分量 (0.0-1.0)
 * @param depth 深度值 (0.0-1.0)
 * @param stencil 模板值
 */
void wlf_framebuffer_clear(struct wlf_framebuffer* fb,
                          float r, float g, float b, float a,
                          float depth, int stencil);

/**
 * @brief 设置视口
 * @param fb 帧缓冲指针
 * @param viewport 视口矩形
 */
void wlf_framebuffer_set_viewport(struct wlf_framebuffer* fb, struct wlf_rect viewport);

/**
 * @brief 读取像素数据
 * @param fb 帧缓冲指针
 * @param region 读取区域
 * @param format 数据格式
 * @param data 输出数据缓冲区
 * @return 成功返回 true，失败返回 false
 */
bool wlf_framebuffer_read_pixels(struct wlf_framebuffer* fb,
                                struct wlf_rect region,
                                enum wlf_framebuffer_format format,
                                void* data);

#ifdef __cplusplus
}
#endif

#endif // RENDER_WLF_FRAMEBUFFER_H
