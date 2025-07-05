#include "../../include/wlf/texture/wlf_gl_texture.h"

GLuint wlf_texture_get_gl_id(struct wlf_texture* texture) {
    if (!texture) {
        return 0;
    }

    // 假设所有OpenGL纹理都是 wlf_gl_texture 类型
    // 这里需要一些类型检查机制，但为了简化，直接转换
    struct wlf_gl_texture* gl_texture = (struct wlf_gl_texture*)texture;
    return gl_texture->tex;
}
