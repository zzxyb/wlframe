/*
 * Copyright © 2024 wlframe contributors
 *
 * SVG image example
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wlf/image/wlf_svg_image.h>
#include <wlf/image/wlf_image.h>
#include <wlf/svg/wlf_svg_node.h>
#include <wlf/utils/wlf_log.h>

int main(int argc, char *argv[])
{
    wlf_log_info("SVG image example starting");

    // 创建SVG图片
    struct wlf_svg_image *svg_image = wlf_svg_image_create();
    if (!svg_image) {
        wlf_log_error("Failed to create SVG image");
        return 1;
    }

    // 创建简单的SVG内容
    const char *svg_content =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        "width=\"200\" height=\"200\" viewBox=\"0 0 200 200\">\n"
        "  <rect x=\"10\" y=\"10\" width=\"180\" height=\"180\" "
        "fill=\"blue\" stroke=\"red\" stroke-width=\"2\"/>\n"
        "  <circle cx=\"100\" cy=\"100\" r=\"50\" fill=\"yellow\"/>\n"
        "  <text x=\"100\" y=\"100\" text-anchor=\"middle\" "
        "fill=\"black\">Hello SVG</text>\n"
        "</svg>\n";

    // 从字符串加载SVG
    struct wlf_svg_image *loaded_svg = wlf_svg_image_from_string(
        svg_content, strlen(svg_content));
    if (!loaded_svg) {
        wlf_log_error("Failed to load SVG from string");
        wlf_svg_image_destroy(svg_image);
        return 1;
    }

    wlf_log_info("Successfully loaded SVG from string");

    // 测试后端切换
    wlf_log_info("Testing backend switching...");

    wlf_svg_image_set_backend(loaded_svg, WLF_SVG_RENDER_BACKEND_PIXMAN);
    wlf_log_info("Set backend to Pixman");

    wlf_svg_image_set_backend(loaded_svg, WLF_SVG_RENDER_BACKEND_GLES);
    wlf_log_info("Set backend to OpenGL ES");

    wlf_svg_image_set_backend(loaded_svg, WLF_SVG_RENDER_BACKEND_VULKAN);
    wlf_log_info("Set backend to Vulkan");

    // 获取根节点并测试DOM操作
    struct wlf_svg_node *root = wlf_svg_image_get_root_node(loaded_svg);
    if (root) {
        wlf_log_info("Got SVG root node, type: %d", root->type);

        // 创建新的矩形节点
        struct wlf_svg_node *rect_node = wlf_svg_node_create(WLF_SVG_NODE_RECT);
        if (rect_node) {
            wlf_log_info("Created rectangle node");

            // 设置属性
            wlf_svg_node_set_attribute(rect_node, "x", "50");
            wlf_svg_node_set_attribute(rect_node, "y", "50");
            wlf_svg_node_set_attribute(rect_node, "width", "100");
            wlf_svg_node_set_attribute(rect_node, "height", "100");
            wlf_svg_node_set_attribute(rect_node, "fill", "green");
            wlf_svg_node_set_attribute(rect_node, "id", "test-rect");

            // 添加到根节点
            if (wlf_svg_node_add_child(root, rect_node) == 0) {
                wlf_log_info("Added rectangle to SVG");
            }
        }

        // 查找节点
        struct wlf_svg_node *found = wlf_svg_node_find_by_id(root, "test-rect");
        if (found) {
            wlf_log_info("Found node by ID: test-rect");
        }
    }

    // 保存SVG到文件
    if (wlf_svg_image_save(loaded_svg, "test_output.svg") == 0) {
        wlf_log_info("Successfully saved SVG to test_output.svg");
    } else {
        wlf_log_error("Failed to save SVG");
    }

    // 创建目标图像用于渲染测试
    struct wlf_image *target_image = wlf_image_create(WLF_IMAGE_FORMAT_RGBA, 200, 200);
    if (target_image) {
        wlf_log_info("Created target image for rendering");

        // 渲染SVG
        if (wlf_svg_image_render(loaded_svg, target_image) == 0) {
            wlf_log_info("Successfully rendered SVG to image");
        } else {
            wlf_log_error("Failed to render SVG");
        }

        wlf_image_destroy(target_image);
    }

    // 清理
    wlf_svg_image_destroy(svg_image);
    wlf_svg_image_destroy(loaded_svg);

    wlf_log_info("SVG image example completed");
    return 0;
}
