/*
 * Copyright © 2024 wlframe contributors
 *
 * SVG image implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wlf/image/wlf_svg_image.h>
#include <wlf/image/wlf_image.h>
#include <wlf/utils/wlf_log.h>
#include <wlf/utils/wlf_utils.h>

// SVG图片结构的内部实现
struct wlf_svg_image {
    struct wlf_image base;              // 继承基础图片结构
    struct wlf_svg_node *root_node;     // SVG根节点
    enum wlf_svg_render_backend backend; // 渲染后端类型
    struct wlf_svg_render_context *render_ctx; // 渲染上下文

    // SVG文档属性
    float width;                        // SVG宽度
    float height;                       // SVG高度
    float view_box_x;                   // 视图框X
    float view_box_y;                   // 视图框Y
    float view_box_width;               // 视图框宽度
    float view_box_height;              // 视图框高度

    // 内部状态
    bool is_dirty;                      // 是否需要重新渲染
    char *source_data;                  // 原始SVG数据
    size_t source_size;                 // 原始数据大小
};

// 默认的SVG解析器（简单的XML解析）
static struct wlf_svg_node *parse_svg_from_string(const char *svg_data, size_t size);
static char *serialize_svg_to_string(struct wlf_svg_node *root, size_t *out_size);

// SVG图片的虚函数表
static void svg_image_destroy(struct wlf_image *image);
static int svg_image_save(struct wlf_image *image, const char *filename);
static struct wlf_image *svg_image_clone(const struct wlf_image *image);

static const struct wlf_image_impl svg_image_impl = {
    .destroy = svg_image_destroy,
    .save = svg_image_save,
    .clone = svg_image_clone,
};

struct wlf_svg_image *wlf_svg_image_create(void)
{
    struct wlf_svg_image *svg_image = calloc(1, sizeof(*svg_image));
    if (!svg_image) {
        wlf_log_error("Failed to allocate SVG image");
        return NULL;
    }

    // 初始化基础图片结构
    wlf_image_init(&svg_image->base, WLF_IMAGE_FORMAT_RGBA, &svg_image_impl);

    // 初始化SVG特有属性
    svg_image->backend = WLF_SVG_RENDER_BACKEND_PIXMAN;
    svg_image->width = 100.0f;
    svg_image->height = 100.0f;
    svg_image->view_box_width = 100.0f;
    svg_image->view_box_height = 100.0f;
    svg_image->is_dirty = true;

    return svg_image;
}

struct wlf_svg_image *wlf_svg_image_load(const char *filename)
{
    if (!filename) {
        wlf_log_error("Invalid filename for SVG loading");
        return NULL;
    }

    // 读取文件
    FILE *file = fopen(filename, "rb");
    if (!file) {
        wlf_log_error("Failed to open SVG file: %s", filename);
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        wlf_log_error("Invalid SVG file size: %s", filename);
        fclose(file);
        return NULL;
    }

    // 读取文件内容
    char *svg_data = malloc(file_size + 1);
    if (!svg_data) {
        wlf_log_error("Failed to allocate memory for SVG data");
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(svg_data, 1, file_size, file);
    fclose(file);

    if (read_size != (size_t)file_size) {
        wlf_log_error("Failed to read complete SVG file");
        free(svg_data);
        return NULL;
    }

    svg_data[file_size] = '\0';

    // 从字符串创建SVG图片
    struct wlf_svg_image *svg_image = wlf_svg_image_from_string(svg_data, file_size);
    free(svg_data);

    return svg_image;
}

struct wlf_svg_image *wlf_svg_image_from_string(const char *svg_data, size_t size)
{
    if (!svg_data || size == 0) {
        wlf_log_error("Invalid SVG data");
        return NULL;
    }

    struct wlf_svg_image *svg_image = wlf_svg_image_create();
    if (!svg_image) {
        return NULL;
    }

    // 保存原始数据
    svg_image->source_data = malloc(size + 1);
    if (!svg_image->source_data) {
        wlf_log_error("Failed to allocate memory for SVG source data");
        wlf_svg_image_destroy(svg_image);
        return NULL;
    }

    memcpy(svg_image->source_data, svg_data, size);
    svg_image->source_data[size] = '\0';
    svg_image->source_size = size;

    // 解析SVG
    svg_image->root_node = parse_svg_from_string(svg_data, size);
    if (!svg_image->root_node) {
        wlf_log_error("Failed to parse SVG data");
        wlf_svg_image_destroy(svg_image);
        return NULL;
    }

    // 从根节点提取文档属性
    if (svg_image->root_node->type == WLF_SVG_NODE_SVG) {
        struct wlf_svg_element_svg *svg_elem =
            (struct wlf_svg_element_svg *)svg_image->root_node->element_data;
        if (svg_elem) {
            svg_image->width = svg_elem->width;
            svg_image->height = svg_elem->height;
            svg_image->view_box_x = svg_elem->view_box.x;
            svg_image->view_box_y = svg_elem->view_box.y;
            svg_image->view_box_width = svg_elem->view_box.width;
            svg_image->view_box_height = svg_elem->view_box.height;
        }
    }

    return svg_image;
}

void wlf_svg_image_destroy(struct wlf_svg_image *svg_image)
{
    if (!svg_image) {
        return;
    }

    // 销毁SVG节点树
    if (svg_image->root_node) {
        wlf_svg_node_destroy(svg_image->root_node);
    }

    // 销毁渲染上下文
    if (svg_image->render_ctx) {
        wlf_svg_render_context_destroy(svg_image->render_ctx);
    }

    // 释放原始数据
    free(svg_image->source_data);

    // 销毁基础图片
    wlf_image_deinit(&svg_image->base);

    free(svg_image);
}

int wlf_svg_image_save(struct wlf_svg_image *svg_image, const char *filename)
{
    if (!svg_image || !filename) {
        wlf_log_error("Invalid parameters for SVG saving");
        return -1;
    }

    if (!svg_image->root_node) {
        wlf_log_error("No SVG content to save");
        return -1;
    }

    // 序列化SVG到字符串
    size_t svg_size;
    char *svg_data = serialize_svg_to_string(svg_image->root_node, &svg_size);
    if (!svg_data) {
        wlf_log_error("Failed to serialize SVG");
        return -1;
    }

    // 写入文件
    FILE *file = fopen(filename, "wb");
    if (!file) {
        wlf_log_error("Failed to open file for writing: %s", filename);
        free(svg_data);
        return -1;
    }

    size_t written = fwrite(svg_data, 1, svg_size, file);
    fclose(file);
    free(svg_data);

    if (written != svg_size) {
        wlf_log_error("Failed to write complete SVG file");
        return -1;
    }

    return 0;
}

int wlf_svg_image_set_backend(struct wlf_svg_image *svg_image,
                              enum wlf_svg_render_backend backend)
{
    if (!svg_image) {
        wlf_log_error("Invalid SVG image");
        return -1;
    }

    if (svg_image->backend == backend) {
        return 0; // 已经是目标后端
    }

    // 销毁旧的渲染上下文
    if (svg_image->render_ctx) {
        wlf_svg_render_context_destroy(svg_image->render_ctx);
        svg_image->render_ctx = NULL;
    }

    // 设置新后端
    svg_image->backend = backend;
    svg_image->is_dirty = true;

    return 0;
}

int wlf_svg_image_render(struct wlf_svg_image *svg_image,
                         struct wlf_image *target_image)
{
    if (!svg_image || !target_image) {
        wlf_log_error("Invalid parameters for SVG rendering");
        return -1;
    }

    if (!svg_image->root_node) {
        wlf_log_error("No SVG content to render");
        return -1;
    }

    // 获取或创建渲染上下文
    if (!svg_image->render_ctx) {
        svg_image->render_ctx = wlf_svg_render_context_create(svg_image->backend);
        if (!svg_image->render_ctx) {
            wlf_log_error("Failed to create SVG render context");
            return -1;
        }
    }

    // 设置渲染参数
    struct wlf_svg_render_params params = {
        .target_image = target_image,
        .viewport_width = target_image->width,
        .viewport_height = target_image->height,
        .scale_x = (float)target_image->width / svg_image->view_box_width,
        .scale_y = (float)target_image->height / svg_image->view_box_height,
        .offset_x = -svg_image->view_box_x,
        .offset_y = -svg_image->view_box_y,
    };

    // 渲染SVG节点树
    int result = wlf_svg_render_node(svg_image->render_ctx,
                                     svg_image->root_node, &params);
    if (result != 0) {
        wlf_log_error("Failed to render SVG");
        return -1;
    }

    svg_image->is_dirty = false;
    return 0;
}

struct wlf_svg_node *wlf_svg_image_get_root_node(struct wlf_svg_image *svg_image)
{
    return svg_image ? svg_image->root_node : NULL;
}

int wlf_svg_image_set_root_node(struct wlf_svg_image *svg_image,
                                 struct wlf_svg_node *root_node)
{
    if (!svg_image) {
        wlf_log_error("Invalid SVG image");
        return -1;
    }

    // 销毁旧的根节点
    if (svg_image->root_node) {
        wlf_svg_node_destroy(svg_image->root_node);
    }

    svg_image->root_node = root_node;
    svg_image->is_dirty = true;

    return 0;
}

// 虚函数实现
static void svg_image_destroy(struct wlf_image *image)
{
    struct wlf_svg_image *svg_image = (struct wlf_svg_image *)image;
    wlf_svg_image_destroy(svg_image);
}

static int svg_image_save(struct wlf_image *image, const char *filename)
{
    struct wlf_svg_image *svg_image = (struct wlf_svg_image *)image;
    return wlf_svg_image_save(svg_image, filename);
}

static struct wlf_image *svg_image_clone(const struct wlf_image *image)
{
    const struct wlf_svg_image *svg_image = (const struct wlf_svg_image *)image;

    if (!svg_image->source_data) {
        wlf_log_error("Cannot clone SVG image without source data");
        return NULL;
    }

    struct wlf_svg_image *cloned = wlf_svg_image_from_string(
        svg_image->source_data, svg_image->source_size);

    if (cloned) {
        cloned->backend = svg_image->backend;
    }

    return (struct wlf_image *)cloned;
}

// 简单的SVG解析器实现（占位符，实际应该使用更完善的XML解析库）
static struct wlf_svg_node *parse_svg_from_string(const char *svg_data, size_t size)
{
    // TODO: 实现完整的SVG解析器
    // 这里只是一个占位符实现，创建一个简单的SVG根节点

    struct wlf_svg_node *root = wlf_svg_node_create(WLF_SVG_NODE_SVG);
    if (!root) {
        return NULL;
    }

    // 创建默认的SVG元素数据
    struct wlf_svg_element_svg *svg_elem = calloc(1, sizeof(*svg_elem));
    if (!svg_elem) {
        wlf_svg_node_destroy(root);
        return NULL;
    }

    svg_elem->width = 100.0f;
    svg_elem->height = 100.0f;
    svg_elem->view_box.width = 100.0f;
    svg_elem->view_box.height = 100.0f;

    root->element_data = svg_elem;

    wlf_log_info("Created placeholder SVG node (parser not yet implemented)");
    return root;
}

static char *serialize_svg_to_string(struct wlf_svg_node *root, size_t *out_size)
{
    // TODO: 实现完整的SVG序列化器
    // 这里只是一个占位符实现

    const char *default_svg =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        "width=\"100\" height=\"100\" viewBox=\"0 0 100 100\">\n"
        "  <!-- SVG content would be here -->\n"
        "</svg>\n";

    size_t size = strlen(default_svg);
    char *result = malloc(size + 1);
    if (!result) {
        return NULL;
    }

    strcpy(result, default_svg);
    if (out_size) {
        *out_size = size;
    }

    wlf_log_info("Generated placeholder SVG content (serializer not yet implemented)");
    return result;
}
