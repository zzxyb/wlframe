# VA-API JPEG DMA-BUF 功能

本次更新添加了使用 VA-API 硬件加速的 JPEG 编解码功能，支持 DMA-BUF 导入导出。

## 新增文件

### 头文件
- `include/wlf/va/wlf_va_jpeg.h` - VA-API JPEG 编解码 API 定义

### 实现文件
- `va/wlf_va_jpeg.c` - VA-API JPEG 编解码实现

### 示例程序
- `examples/va/vaapi_jpeg_demo.c` - JPEG 编解码演示程序

### 文档
- `va/VA_API_JPEG_DMABUF.md` - 详细使用文档
- `test_vaapi_jpeg.sh` - 自动化测试脚本

## 核心功能

### 1. JPEG 解码到 DMA-BUF
```c
struct wlf_va_jpeg_decoder *decoder = wlf_va_jpeg_decoder_create(va_display);
struct wlf_dmabuf_attributes attribs;
wlf_va_jpeg_decode_file_to_dmabuf(decoder, "input.jpg", &attribs);
```

### 2. DMA-BUF 编码为 JPEG
```c
struct wlf_va_jpeg_encoder *encoder = wlf_va_jpeg_encoder_create(va_display, 85);
wlf_va_jpeg_encode_dmabuf_to_file(encoder, &attribs, "output.jpg");
```

## 主要 API

### 解码器 API
- `wlf_va_jpeg_decoder_create()` - 创建解码器
- `wlf_va_jpeg_decoder_destroy()` - 销毁解码器
- `wlf_va_jpeg_decode_file_to_dmabuf()` - 从文件解码到 DMA-BUF
- `wlf_va_jpeg_decode_data_to_dmabuf()` - 从内存解码到 DMA-BUF
- `wlf_va_jpeg_decoder_get_surface()` - 获取 VA surface ID

### 编码器 API
- `wlf_va_jpeg_encoder_create()` - 创建编码器
- `wlf_va_jpeg_encoder_destroy()` - 销毁编码器
- `wlf_va_jpeg_encode_dmabuf_to_file()` - 从 DMA-BUF 编码到文件
- `wlf_va_jpeg_encode_dmabuf_to_data()` - 从 DMA-BUF 编码到内存
- `wlf_va_jpeg_encoder_set_quality()` - 设置编码质量

## 使用场景

1. **视频帧截图**: 将解码的视频帧保存为 JPEG
2. **图像预处理**: 使用硬件加速快速处理 JPEG 图片
3. **零拷贝工作流**: 在 GPU 组件间共享图像数据
4. **Wayland 显示**: 解码 JPEG 并直接显示到 Wayland 表面

## 编译和测试

### 编译
```bash
meson setup build
meson compile -C build
```

### 运行示例
```bash
./build/examples/vaapi_jpeg_demo -i input.jpg -o output.jpg -q 90
```

### 运行测试
```bash
./test_vaapi_jpeg.sh
```

## 技术特性

- ✅ 硬件加速编解码
- ✅ 零拷贝 DMA-BUF 共享
- ✅ 支持 NV12 格式
- ✅ 可调节 JPEG 质量（1-100）
- ✅ 文件和内存 I/O
- ✅ 完整的错误处理

## 系统要求

- 支持 VA-API 的 GPU（Intel、AMD 等）
- JPEG 编解码硬件支持
- Linux 内核 5.20+
- libva 库

检查硬件支持：
```bash
vainfo | grep JPEG
```

## 性能优势

相比传统 CPU 软件编解码：
- 更快的编解码速度
- 更低的 CPU 占用
- 零拷贝内存共享
- 更高的能效比

## 相关文档

详细使用说明请参阅：
- [VA_API_JPEG_DMABUF.md](va/VA_API_JPEG_DMABUF.md) - 完整 API 文档和示例
- [video_wayland_zero_copy.md](docs/video_wayland_zero_copy.md) - Wayland 零拷贝集成

## 许可证

与项目主许可证相同。
