# VA-API JPEG DMA-BUF Support

本文档介绍如何使用 VA-API 进行 JPEG 图片的硬件加速编解码，并支持 DMA-BUF 导入导出，实现零拷贝工作流。

## 功能特性

### JPEG 解码到 DMA-BUF
- 使用 VA-API 硬件加速解码 JPEG 图片
- 直接导出为 DMA-BUF 文件描述符
- 支持从文件或内存加载 JPEG
- 零拷贝集成到其他硬件加速组件

### DMA-BUF 到 JPEG 编码
- 导入 DMA-BUF 并使用 VA-API 编码为 JPEG
- 可调节 JPEG 质量（1-100）
- 支持保存到文件或内存
- 零拷贝从其他硬件组件获取图像

## API 使用

### 包含头文件

```c
#include "wlf/va/wlf_va_jpeg.h"
#include "wlf/va/wlf_va_display.h"
#include "wlf/dmabuf/wlf_dmabuf.h"
```

### 解码 JPEG 到 DMA-BUF

```c
/* 创建 VA display */
struct wlf_va_display *va_display = wlf_va_display_autocreate(backend);

/* 创建 JPEG 解码器 */
struct wlf_va_jpeg_decoder *decoder = wlf_va_jpeg_decoder_create(va_display);

/* 解码 JPEG 文件到 DMA-BUF */
struct wlf_dmabuf_attributes attribs;
if (wlf_va_jpeg_decode_file_to_dmabuf(decoder, "input.jpg", &attribs)) {
    printf("解码成功: %dx%d, format=0x%x, %d planes\n",
        attribs.width, attribs.height, attribs.format, attribs.n_planes);

    /* 使用 DMA-BUF... */

    /* 清理 */
    wlf_dmabuf_attributes_finish(&attribs);
}

/* 销毁解码器 */
wlf_va_jpeg_decoder_destroy(decoder);
```

### 从内存解码 JPEG

```c
/* 从内存解码 JPEG 数据 */
uint8_t *jpeg_data = ...; /* JPEG 数据 */
size_t jpeg_size = ...;   /* JPEG 数据大小 */

struct wlf_dmabuf_attributes attribs;
if (wlf_va_jpeg_decode_data_to_dmabuf(decoder, jpeg_data, jpeg_size, &attribs)) {
    /* 使用 DMA-BUF... */
    wlf_dmabuf_attributes_finish(&attribs);
}
```

### DMA-BUF 编码为 JPEG

```c
/* 创建 JPEG 编码器，质量设为 90 */
struct wlf_va_jpeg_encoder *encoder = wlf_va_jpeg_encoder_create(va_display, 90);

/* 从 DMA-BUF 编码为 JPEG 文件 */
if (wlf_va_jpeg_encode_dmabuf_to_file(encoder, &attribs, "output.jpg")) {
    printf("编码成功\n");
}

/* 销毁编码器 */
wlf_va_jpeg_encoder_destroy(encoder);
```

### 编码到内存

```c
/* 编码到内存 */
uint8_t *jpeg_data;
size_t jpeg_size;

if (wlf_va_jpeg_encode_dmabuf_to_data(encoder, &attribs, &jpeg_data, &jpeg_size)) {
    printf("编码成功: %zu 字节\n", jpeg_size);

    /* 使用 JPEG 数据... */

    /* 释放内存 */
    free(jpeg_data);
}
```

### 调整编码质量

```c
/* 设置 JPEG 质量 (1-100) */
wlf_va_jpeg_encoder_set_quality(encoder, 95);
```

## 示例程序

### vaapi_jpeg_demo

演示 JPEG 解码到 DMA-BUF 再编码回 JPEG 的完整流程。

```bash
# 编译
cd build
meson compile

# 运行
./examples/vaapi_jpeg_demo -i input.jpg -o output.jpg -q 90
```

参数说明：
- `-i, --input <file>`: 输入 JPEG 文件
- `-o, --output <file>`: 输出 JPEG 文件
- `-q, --quality <1-100>`: JPEG 编码质量（默认 85）
- `-h, --help`: 显示帮助信息

示例：
```bash
# 使用质量 90 重新编码 JPEG
./examples/vaapi_jpeg_demo -i photo.jpg -o photo_reencoded.jpg -q 90

# 使用默认质量 85
./examples/vaapi_jpeg_demo -i photo.jpg -o output.jpg
```

## 工作流集成

### 与视频解码器集成

```c
/* 从视频帧导出 DMA-BUF */
struct wlf_dmabuf_attributes frame_attribs;
/* ... 获取视频帧的 DMA-BUF ... */

/* 将视频帧保存为 JPEG */
struct wlf_va_jpeg_encoder *encoder = wlf_va_jpeg_encoder_create(va_display, 85);
wlf_va_jpeg_encode_dmabuf_to_file(encoder, &frame_attribs, "frame.jpg");
```

### 与 Wayland 集成

```c
/* 解码 JPEG 并显示到 Wayland */
struct wlf_dmabuf_attributes attribs;
wlf_va_jpeg_decode_file_to_dmabuf(decoder, "image.jpg", &attribs);

/* 使用 linux-dmabuf 协议创建 wl_buffer */
struct wl_buffer *buffer = create_dmabuf_buffer(dmabuf_manager, &attribs);
wl_surface_attach(surface, buffer, 0, 0);
wl_surface_commit(surface);
```

## 性能特点

### 硬件加速
- 使用 GPU 进行 JPEG 编解码
- 相比 CPU 软件编解码更快
- 降低 CPU 占用率

### 零拷贝
- DMA-BUF 允许在不同硬件组件间共享内存
- 避免 CPU 内存拷贝
- 提高整体系统性能

### 适用场景
- 高分辨率图像处理
- 视频帧截图
- 实时图像流处理
- 多媒体应用

## 系统要求

### 硬件
- 支持 VA-API 的 GPU（Intel、AMD 等）
- JPEG 编解码硬件加速支持

### 软件
- Linux 内核 5.20+ (用于 DMA-BUF sync_file)
- VA-API 库 (libva)
- DRM 支持

### 检查硬件支持

```bash
# 检查 VA-API 支持
vainfo

# 查看支持的 profile
vainfo | grep JPEG
```

输出示例：
```
VAProfileJPEGBaseline          : VAEntrypointVLD
VAProfileJPEGBaseline          : VAEntrypointEncPicture
```

## 错误处理

所有 API 函数返回 `bool` 表示成功/失败。使用日志系统查看详细错误：

```c
/* 启用调试日志 */
wlf_log_init(WLF_DEBUG);

/* API 调用 */
if (!wlf_va_jpeg_decode_file_to_dmabuf(decoder, "input.jpg", &attribs)) {
    /* 检查日志获取详细错误信息 */
    wlf_log(WLF_ERROR, "JPEG 解码失败");
    return false;
}
```

常见错误：
- **VA_STATUS_ERROR_UNSUPPORTED_PROFILE**: GPU 不支持 JPEG 编解码
- **VA_STATUS_ERROR_ALLOCATION_FAILED**: 内存分配失败
- **VA_STATUS_ERROR_INVALID_SURFACE**: 无效的 surface ID
- **文件 I/O 错误**: 检查文件路径和权限

## 限制和注意事项

### 格式支持
- 当前主要支持 NV12 (YUV420) 格式
- JPEG 基线格式 (Baseline JPEG)
- 不支持渐进式 JPEG 解码（编码器可选）

### DMA-BUF 属性
- 必须正确管理 DMA-BUF 文件描述符生命周期
- 使用完毕后调用 `wlf_dmabuf_attributes_finish()` 清理
- 不要手动关闭 `attribs.fd[]` 中的文件描述符

### 线程安全
- VA-API 调用不是线程安全的
- 需要在同一线程中使用解码器/编码器
- 或使用互斥锁保护

## 相关文档

- [video_wayland_zero_copy.md](../../docs/video_wayland_zero_copy.md) - Wayland 零拷贝视频
- [WAYLAND_BUFFER_EXPORT.md](../../docs/WAYLAND_BUFFER_EXPORT.md) - Wayland 缓冲区导出
- [VA-API 文档](https://01.org/linuxmedia/vaapi)
- [DMA-BUF 文档](https://www.kernel.org/doc/html/latest/driver-api/dma-buf.html)

## 许可证

本项目遵循项目根目录下的 LICENSE 文件。
