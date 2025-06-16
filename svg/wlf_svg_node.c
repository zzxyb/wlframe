/*
 * Copyright © 2024 wlframe contributors
 *
 * SVG DOM node implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wlf/svg/wlf_svg_node.h>
#include <wlf/svg/wlf_svg_elements.h>
#include <wlf/utils/wlf_log.h>
#include <wlf/utils/wlf_utils.h>

// 节点impl函数的默认实现
static int default_parse_attributes(struct wlf_svg_node *node, const char **attrs);
static char *default_serialize_attributes(struct wlf_svg_node *node);
static struct wlf_svg_node *default_clone(const struct wlf_svg_node *node);
static int default_validate(const struct wlf_svg_node *node);
static void default_destroy_element(struct wlf_svg_node *node);

// 各类型节点的impl实现
static const struct wlf_svg_node_impl svg_impl;
static const struct wlf_svg_node_impl g_impl;
static const struct wlf_svg_node_impl rect_impl;
static const struct wlf_svg_node_impl circle_impl;
static const struct wlf_svg_node_impl ellipse_impl;
static const struct wlf_svg_node_impl line_impl;
static const struct wlf_svg_node_impl polyline_impl;
static const struct wlf_svg_node_impl polygon_impl;
static const struct wlf_svg_node_impl path_impl;
static const struct wlf_svg_node_impl text_impl;

// 节点类型到impl的映射
static const struct wlf_svg_node_impl *get_node_impl(enum wlf_svg_node_type type)
{
    switch (type) {
        case WLF_SVG_NODE_SVG:
            return &svg_impl;
        case WLF_SVG_NODE_G:
            return &g_impl;
        case WLF_SVG_NODE_RECT:
            return &rect_impl;
        case WLF_SVG_NODE_CIRCLE:
            return &circle_impl;
        case WLF_SVG_NODE_ELLIPSE:
            return &ellipse_impl;
        case WLF_SVG_NODE_LINE:
            return &line_impl;
        case WLF_SVG_NODE_POLYLINE:
            return &polyline_impl;
        case WLF_SVG_NODE_POLYGON:
            return &polygon_impl;
        case WLF_SVG_NODE_PATH:
            return &path_impl;
        case WLF_SVG_NODE_TEXT:
            return &text_impl;
        default:
            return NULL;
    }
}

// 节点类型到名称的映射
static const char *get_node_type_name(enum wlf_svg_node_type type)
{
    switch (type) {
        case WLF_SVG_NODE_SVG:     return "svg";
        case WLF_SVG_NODE_G:       return "g";
        case WLF_SVG_NODE_RECT:    return "rect";
        case WLF_SVG_NODE_CIRCLE:  return "circle";
        case WLF_SVG_NODE_ELLIPSE: return "ellipse";
        case WLF_SVG_NODE_LINE:    return "line";
        case WLF_SVG_NODE_POLYLINE:return "polyline";
        case WLF_SVG_NODE_POLYGON: return "polygon";
        case WLF_SVG_NODE_PATH:    return "path";
        case WLF_SVG_NODE_TEXT:    return "text";
        default:                   return "unknown";
    }
}

struct wlf_svg_node *wlf_svg_node_create(enum wlf_svg_node_type type)
{
    struct wlf_svg_node *node = calloc(1, sizeof(*node));
    if (!node) {
        wlf_log_error("Failed to allocate SVG node");
        return NULL;
    }

    node->type = type;
    node->impl = get_node_impl(type);

    if (!node->impl) {
        wlf_log_error("Unknown SVG node type: %d", type);
        free(node);
        return NULL;
    }

    // 根据类型分配特定的元素数据
    switch (type) {
        case WLF_SVG_NODE_SVG:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_svg));
            break;
        case WLF_SVG_NODE_G:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_g));
            break;
        case WLF_SVG_NODE_RECT:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_rect));
            break;
        case WLF_SVG_NODE_CIRCLE:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_circle));
            break;
        case WLF_SVG_NODE_ELLIPSE:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_ellipse));
            break;
        case WLF_SVG_NODE_LINE:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_line));
            break;
        case WLF_SVG_NODE_POLYLINE:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_polyline));
            break;
        case WLF_SVG_NODE_POLYGON:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_polygon));
            break;
        case WLF_SVG_NODE_PATH:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_path));
            break;
        case WLF_SVG_NODE_TEXT:
            node->element_data = calloc(1, sizeof(struct wlf_svg_element_text));
            break;
    }

    if (!node->element_data) {
        wlf_log_error("Failed to allocate element data for SVG node");
        free(node);
        return NULL;
    }

    // 初始化样式
    wlf_svg_style_init(&node->style);

    return node;
}

void wlf_svg_node_destroy(struct wlf_svg_node *node)
{
    if (!node) {
        return;
    }

    // 销毁子节点
    struct wlf_svg_node *child = node->first_child;
    while (child) {
        struct wlf_svg_node *next = child->next_sibling;
        wlf_svg_node_destroy(child);
        child = next;
    }

    // 调用impl的销毁函数
    if (node->impl && node->impl->destroy_element) {
        node->impl->destroy_element(node);
    }

    // 释放通用数据
    free(node->id);
    free(node->class_name);
    wlf_svg_style_deinit(&node->style);
    free(node->element_data);
    free(node);
}

int wlf_svg_node_add_child(struct wlf_svg_node *parent, struct wlf_svg_node *child)
{
    if (!parent || !child) {
        wlf_log_error("Invalid parameters for adding child node");
        return -1;
    }

    if (child->parent) {
        wlf_log_error("Child node already has a parent");
        return -1;
    }

    // 设置父子关系
    child->parent = parent;

    // 添加到子节点链表
    if (!parent->first_child) {
        parent->first_child = child;
        parent->last_child = child;
    } else {
        parent->last_child->next_sibling = child;
        child->prev_sibling = parent->last_child;
        parent->last_child = child;
    }

    parent->child_count++;
    return 0;
}

int wlf_svg_node_remove_child(struct wlf_svg_node *parent, struct wlf_svg_node *child)
{
    if (!parent || !child || child->parent != parent) {
        wlf_log_error("Invalid parameters for removing child node");
        return -1;
    }

    // 从子节点链表中移除
    if (child->prev_sibling) {
        child->prev_sibling->next_sibling = child->next_sibling;
    } else {
        parent->first_child = child->next_sibling;
    }

    if (child->next_sibling) {
        child->next_sibling->prev_sibling = child->prev_sibling;
    } else {
        parent->last_child = child->prev_sibling;
    }

    // 清除父子关系
    child->parent = NULL;
    child->prev_sibling = NULL;
    child->next_sibling = NULL;

    parent->child_count--;
    return 0;
}

int wlf_svg_node_set_attribute(struct wlf_svg_node *node, const char *name, const char *value)
{
    if (!node || !name) {
        wlf_log_error("Invalid parameters for setting attribute");
        return -1;
    }

    // 处理通用属性
    if (strcmp(name, "id") == 0) {
        free(node->id);
        node->id = value ? strdup(value) : NULL;
        return 0;
    } else if (strcmp(name, "class") == 0) {
        free(node->class_name);
        node->class_name = value ? strdup(value) : NULL;
        return 0;
    }

    // 处理样式属性
    if (wlf_svg_style_set_attribute(&node->style, name, value) == 0) {
        return 0;
    }

    // 调用impl的属性解析函数
    if (node->impl && node->impl->parse_attributes) {
        const char *attrs[] = { name, value, NULL };
        return node->impl->parse_attributes(node, attrs);
    }

    wlf_log_warning("Unknown attribute '%s' for node type '%s'",
                    name, get_node_type_name(node->type));
    return -1;
}

const char *wlf_svg_node_get_attribute(struct wlf_svg_node *node, const char *name)
{
    if (!node || !name) {
        return NULL;
    }

    // 处理通用属性
    if (strcmp(name, "id") == 0) {
        return node->id;
    } else if (strcmp(name, "class") == 0) {
        return node->class_name;
    }

    // 处理样式属性
    const char *style_value = wlf_svg_style_get_attribute(&node->style, name);
    if (style_value) {
        return style_value;
    }

    // TODO: 处理元素特定属性
    return NULL;
}

struct wlf_svg_node *wlf_svg_node_clone(const struct wlf_svg_node *node)
{
    if (!node) {
        return NULL;
    }

    if (node->impl && node->impl->clone) {
        return node->impl->clone(node);
    }

    return default_clone(node);
}

int wlf_svg_node_validate(const struct wlf_svg_node *node)
{
    if (!node) {
        return -1;
    }

    if (node->impl && node->impl->validate) {
        return node->impl->validate(node);
    }

    return default_validate(node);
}

struct wlf_svg_node *wlf_svg_node_find_by_id(struct wlf_svg_node *root, const char *id)
{
    if (!root || !id) {
        return NULL;
    }

    // 检查当前节点
    if (root->id && strcmp(root->id, id) == 0) {
        return root;
    }

    // 递归检查子节点
    struct wlf_svg_node *child = root->first_child;
    while (child) {
        struct wlf_svg_node *found = wlf_svg_node_find_by_id(child, id);
        if (found) {
            return found;
        }
        child = child->next_sibling;
    }

    return NULL;
}

// 样式相关函数
void wlf_svg_style_init(struct wlf_svg_style *style)
{
    if (!style) {
        return;
    }

    memset(style, 0, sizeof(*style));

    // 设置默认值
    style->fill.type = WLF_SVG_PAINT_COLOR;
    style->fill.color.r = 0.0f;
    style->fill.color.g = 0.0f;
    style->fill.color.b = 0.0f;
    style->fill.color.a = 1.0f;

    style->stroke.type = WLF_SVG_PAINT_NONE;
    style->stroke_width = 1.0f;
    style->opacity = 1.0f;
    style->fill_opacity = 1.0f;
    style->stroke_opacity = 1.0f;
}

void wlf_svg_style_deinit(struct wlf_svg_style *style)
{
    if (!style) {
        return;
    }

    // 释放字符串属性
    free(style->fill.url);
    free(style->stroke.url);
    free(style->font_family);
}

int wlf_svg_style_set_attribute(struct wlf_svg_style *style, const char *name, const char *value)
{
    if (!style || !name) {
        return -1;
    }

    if (!value) {
        // 清除属性
        return 0;
    }

    // 解析样式属性
    if (strcmp(name, "fill") == 0) {
        return wlf_svg_parse_paint(&style->fill, value);
    } else if (strcmp(name, "stroke") == 0) {
        return wlf_svg_parse_paint(&style->stroke, value);
    } else if (strcmp(name, "stroke-width") == 0) {
        style->stroke_width = strtof(value, NULL);
        return 0;
    } else if (strcmp(name, "opacity") == 0) {
        style->opacity = strtof(value, NULL);
        return 0;
    } else if (strcmp(name, "fill-opacity") == 0) {
        style->fill_opacity = strtof(value, NULL);
        return 0;
    } else if (strcmp(name, "stroke-opacity") == 0) {
        style->stroke_opacity = strtof(value, NULL);
        return 0;
    } else if (strcmp(name, "font-family") == 0) {
        free(style->font_family);
        style->font_family = strdup(value);
        return 0;
    } else if (strcmp(name, "font-size") == 0) {
        style->font_size = strtof(value, NULL);
        return 0;
    }

    return -1; // 未知属性
}

const char *wlf_svg_style_get_attribute(const struct wlf_svg_style *style, const char *name)
{
    // TODO: 实现样式属性的获取
    // 这需要将内部表示转换回字符串形式
    return NULL;
}

int wlf_svg_parse_paint(struct wlf_svg_paint *paint, const char *value)
{
    if (!paint || !value) {
        return -1;
    }

    // 清除旧值
    free(paint->url);
    paint->url = NULL;

    if (strcmp(value, "none") == 0) {
        paint->type = WLF_SVG_PAINT_NONE;
        return 0;
    } else if (strncmp(value, "url(", 4) == 0) {
        // URL引用（如渐变）
        paint->type = WLF_SVG_PAINT_URL;
        const char *url_start = value + 4;
        const char *url_end = strchr(url_start, ')');
        if (url_end) {
            size_t url_len = url_end - url_start;
            if (url_start[0] == '#') {
                url_start++;
                url_len--;
            }
            paint->url = malloc(url_len + 1);
            if (paint->url) {
                memcpy(paint->url, url_start, url_len);
                paint->url[url_len] = '\0';
            }
        }
        return 0;
    } else {
        // 颜色值
        paint->type = WLF_SVG_PAINT_COLOR;
        return wlf_svg_parse_color(&paint->color, value);
    }
}

int wlf_svg_parse_color(struct wlf_svg_color *color, const char *value)
{
    if (!color || !value) {
        return -1;
    }

    // 简单的颜色解析（支持基本格式）
    if (value[0] == '#') {
        // 十六进制颜色
        unsigned int hex_value = strtoul(value + 1, NULL, 16);
        if (strlen(value) == 7) { // #RRGGBB
            color->r = ((hex_value >> 16) & 0xFF) / 255.0f;
            color->g = ((hex_value >> 8) & 0xFF) / 255.0f;
            color->b = (hex_value & 0xFF) / 255.0f;
            color->a = 1.0f;
            return 0;
        } else if (strlen(value) == 4) { // #RGB
            color->r = ((hex_value >> 8) & 0xF) / 15.0f;
            color->g = ((hex_value >> 4) & 0xF) / 15.0f;
            color->b = (hex_value & 0xF) / 15.0f;
            color->a = 1.0f;
            return 0;
        }
    } else if (strncmp(value, "rgb(", 4) == 0) {
        // RGB颜色
        int r, g, b;
        if (sscanf(value, "rgb(%d,%d,%d)", &r, &g, &b) == 3) {
            color->r = r / 255.0f;
            color->g = g / 255.0f;
            color->b = b / 255.0f;
            color->a = 1.0f;
            return 0;
        }
    } else if (strncmp(value, "rgba(", 5) == 0) {
        // RGBA颜色
        int r, g, b;
        float a;
        if (sscanf(value, "rgba(%d,%d,%d,%f)", &r, &g, &b, &a) == 4) {
            color->r = r / 255.0f;
            color->g = g / 255.0f;
            color->b = b / 255.0f;
            color->a = a;
            return 0;
        }
    }

    // 命名颜色
    if (strcmp(value, "black") == 0) {
        color->r = color->g = color->b = 0.0f;
        color->a = 1.0f;
        return 0;
    } else if (strcmp(value, "white") == 0) {
        color->r = color->g = color->b = 1.0f;
        color->a = 1.0f;
        return 0;
    } else if (strcmp(value, "red") == 0) {
        color->r = 1.0f;
        color->g = color->b = 0.0f;
        color->a = 1.0f;
        return 0;
    } else if (strcmp(value, "green") == 0) {
        color->g = 1.0f;
        color->r = color->b = 0.0f;
        color->a = 1.0f;
        return 0;
    } else if (strcmp(value, "blue") == 0) {
        color->b = 1.0f;
        color->r = color->g = 0.0f;
        color->a = 1.0f;
        return 0;
    }

    return -1;
}

// 默认impl实现
static int default_parse_attributes(struct wlf_svg_node *node, const char **attrs)
{
    // 默认不处理任何属性
    return -1;
}

static char *default_serialize_attributes(struct wlf_svg_node *node)
{
    // 默认返回空字符串
    return strdup("");
}

static struct wlf_svg_node *default_clone(const struct wlf_svg_node *node)
{
    struct wlf_svg_node *cloned = wlf_svg_node_create(node->type);
    if (!cloned) {
        return NULL;
    }

    // 复制通用属性
    if (node->id) {
        cloned->id = strdup(node->id);
    }
    if (node->class_name) {
        cloned->class_name = strdup(node->class_name);
    }

    // 复制样式
    wlf_svg_style_deinit(&cloned->style);
    cloned->style = node->style;
    if (node->style.fill.url) {
        cloned->style.fill.url = strdup(node->style.fill.url);
    }
    if (node->style.stroke.url) {
        cloned->style.stroke.url = strdup(node->style.stroke.url);
    }
    if (node->style.font_family) {
        cloned->style.font_family = strdup(node->style.font_family);
    }

    // 递归克隆子节点
    struct wlf_svg_node *child = node->first_child;
    while (child) {
        struct wlf_svg_node *cloned_child = wlf_svg_node_clone(child);
        if (cloned_child) {
            wlf_svg_node_add_child(cloned, cloned_child);
        }
        child = child->next_sibling;
    }

    return cloned;
}

static int default_validate(const struct wlf_svg_node *node)
{
    // 默认验证总是通过
    return 0;
}

static void default_destroy_element(struct wlf_svg_node *node)
{
    // 默认不需要特殊销毁逻辑
}

// 各类型节点的impl定义（占位符）
static const struct wlf_svg_node_impl svg_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl g_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl rect_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl circle_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl ellipse_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl line_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl polyline_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl polygon_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl path_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};

static const struct wlf_svg_node_impl text_impl = {
    .parse_attributes = default_parse_attributes,
    .serialize_attributes = default_serialize_attributes,
    .clone = default_clone,
    .validate = default_validate,
    .destroy_element = default_destroy_element,
};
