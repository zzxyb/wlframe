# wlf_vulkan_video - Vulkan Video Codec Library

## 项目概述

wlf_vulkan_video 是为 wlframe 框架开发的硬件加速视频编解码库，基于 Vulkan Video 扩展实现。该库参考了 Khronos Group 的 [Vulkan-Video-Samples](https://github.com/KhronosGroup/Vulkan-Video-Samples) 项目，完全符合 wlframe 的代码风格和架构设计。

## 主要特性

### 视频解码 (Video Decoding)

- **编解码器支持**
  - H.264/AVC (Baseline, Main, High profiles)
  - H.265/HEVC (Main, Main10 profiles)
  - AV1 (Main profile)
  - VP9 (Profile 0, 2)

- **色度格式**
  - 4:0:0 (Monochrome)
  - 4:2:0 (最常用)
  - 4:2:2 (专业视频)
  - 4:4:4 (无色度压缩)

- **位深度**
  - 8-bit (标准)
  - 10-bit (HDR)
  - 12-bit (专业应用)

- **高级特性**
  - DPB (Decoded Picture Buffer) 自动管理
  - 参考帧缓存与复用
  - 多线程解码支持
  - Film Grain 合成 (AV1)
  - 异步解码操作

### 视频编码 (Video Encoding)

- **编解码器支持**
  - H.264/AVC
  - H.265/HEVC
  - AV1

- **码率控制模式**
  - CBR (Constant Bitrate) - 恒定码率
  - VBR (Variable Bitrate) - 可变码率
  - CQP (Constant Quantization Parameter) - 恒定量化参数
  - Disabled - 无码率控制

- **GOP 结构**
  - 可配置的 I/P/B 帧模式
  - 开放 GOP / 闭合 GOP
  - B 帧数量可调
  - 自定义 GOP 大小

- **编码配置**
  - 多种编码 Profile 和 Level
  - 自适应码率调整
  - 场景切换检测
  - 多线程编码

## 目录结构

```
video/
├── README.md                    # 本文档
├── meson.build                  # Meson 构建配置
├── wlf_video_decoder.c          # 解码器实现
├── wlf_video_encoder.c          # 编码器实现
├── wlf_video_buffer.c           # 缓冲区管理
└── wlf_video_session.c          # Vulkan 会话管理

include/wlf/video/
├── wlf_video_common.h           # 公共定义和类型
├── wlf_video_decoder.h          # 解码器 API
├── wlf_video_encoder.h          # 编码器 API
└── wlf_video_session.h          # 会话管理 API

examples/video/
├── meson.build                  # 示例构建配置
├── video_decoder_example.c      # 解码器示例
└── video_encoder_example.c      # 编码器示例
```

## API 设计

### 核心设计原则

1. **符合 wlframe 风格**
   - 使用 snake_case 命名
   - 基于结构体的对象设计
   - 实现接口模式 (impl pattern)
   - 信号/事件系统集成
   - 统一的错误日志

2. **参考 Vulkan-Video-Samples**
   - Vulkan Video 最佳实践
   - 高效的资源管理
   - 异步操作模式
   - 正确的同步机制

3. **模块化与可扩展**
   - 清晰的接口定义
   - 独立的编解码实现
   - 插件式架构支持

### 解码器 API

```c
/* 1. 查询设备能力 */
VkVideoCapabilitiesKHR caps;
wlf_video_decoder_query_capabilities(physical_device,
    WLF_VIDEO_CODEC_H264, &caps);

/* 2. 配置解码器 */
struct wlf_video_decoder_config config = {
    .codec = WLF_VIDEO_CODEC_H264,
    .max_width = 1920,
    .max_height = 1080,
    .max_dpb_slots = 16,
    .max_active_references = 16,
    .chroma = WLF_VIDEO_CHROMA_420,
    .bit_depth = 8,
    .enable_film_grain = false,
};

/* 3. 创建解码器 */
struct wlf_video_decoder *decoder = wlf_video_decoder_create(
    device, physical_device, &config);

/* 4. 注册事件处理 */
wlf_signal_add(&decoder->events.frame_decoded, on_frame_decoded);

/* 5. 解码帧 */
struct wlf_video_image output;
bool success = wlf_video_decoder_decode_frame(
    decoder, bitstream_data, bitstream_size, &output);

/* 6. 刷新缓冲 */
wlf_video_decoder_flush(decoder);

/* 7. 销毁解码器 */
wlf_video_decoder_destroy(decoder);
```

### 编码器 API

```c
/* 1. 查询设备能力 */
VkVideoCapabilitiesKHR caps;
wlf_video_encoder_query_capabilities(physical_device,
    WLF_VIDEO_CODEC_H264, &caps);

/* 2. 配置编码器 */
struct wlf_video_encoder_config config = {
    .codec = WLF_VIDEO_CODEC_H264,
    .width = 1920,
    .height = 1080,
    .framerate_num = 30,
    .framerate_den = 1,
    .chroma = WLF_VIDEO_CHROMA_420,
    .bit_depth = 8,

    /* 码率控制 */
    .rate_control_mode = WLF_VIDEO_RATE_CONTROL_CBR,
    .target_bitrate = 5000000,      // 5 Mbps
    .max_bitrate = 6000000,         // 6 Mbps

    /* GOP 结构 */
    .gop_size = 60,                 // 每60帧一个I帧
    .num_b_frames = 2,              // 2个B帧
    .use_open_gop = false,

    /* Profile/Level */
    .profile = 100,                 // High Profile
    .level = 41,                    // Level 4.1
};

/* 3. 创建编码器 */
struct wlf_video_encoder *encoder = wlf_video_encoder_create(
    device, physical_device, &config);

/* 4. 注册事件处理 */
wlf_signal_add(&encoder->events.frame_encoded, on_frame_encoded);

/* 5. 编码帧 */
struct wlf_video_encoded_frame output;
bool success = wlf_video_encoder_encode_frame(
    encoder, input_image, &output);

/* 6. 刷新缓冲 */
wlf_video_encoder_flush(encoder);

/* 7. 销毁编码器 */
wlf_video_encoder_destroy(encoder);
```

## 实现细节

### Vulkan Video 扩展

库使用以下 Vulkan 扩展：

**核心扩展**
- `VK_KHR_video_queue` - 视频队列核心支持
- `VK_KHR_video_decode_queue` - 视频解码队列
- `VK_KHR_video_encode_queue` - 视频编码队列

**解码扩展**
- `VK_KHR_video_decode_h264` - H.264 解码
- `VK_KHR_video_decode_h265` - H.265 解码
- `VK_KHR_video_decode_av1` - AV1 解码
- `VK_KHR_video_decode_vp9` - VP9 解码

**编码扩展**
- `VK_KHR_video_encode_h264` - H.264 编码
- `VK_KHR_video_encode_h265` - H.265 编码
- `VK_KHR_video_encode_av1` - AV1 编码

### 内存管理

- **引用计数**: 所有视频资源使用引用计数管理生命周期
- **缓冲池**: 码流缓冲采用池化管理，减少分配开销
- **DPB 管理**: 自动管理解码图像缓冲，优化内存使用
- **零拷贝**: CPU 和 GPU 之间最小化数据拷贝

### 线程模型

- **异步操作**: 视频操作异步执行，不阻塞主线程
- **命令缓冲**: 使用 Vulkan 命令缓冲提交 GPU 工作
- **同步机制**: Fence 和 Semaphore 实现 CPU-GPU 同步
- **线程安全**: 支持多线程应用场景

### 编码风格

完全遵循 wlframe 编码规范：

1. **命名约定**
   ```c
   // 函数: snake_case
   bool wlf_video_decoder_create();

   // 结构体: struct + snake_case
   struct wlf_video_decoder;

   // 枚举: 全大写 + 下划线
   enum WLF_VIDEO_CODEC_H264;
   ```

2. **对象设计**
   ```c
   // 实现接口模式
   struct wlf_video_decoder {
       const struct wlf_video_decoder_impl *impl;
       // ...
   };

   struct wlf_video_decoder_impl {
       bool (*decode_frame)(...);
       void (*flush)(...);
       void (*destroy)(...);
   };
   ```

3. **事件系统**
   ```c
   struct wlf_video_decoder {
       struct {
           struct wlf_signal frame_decoded;
           struct wlf_signal destroy;
       } events;
   };
   ```

4. **错误处理**
   ```c
   if (!decoder) {
       wlf_log(WLF_ERROR, "Failed to create decoder");
       return NULL;
   }
   ```

5. **文档注释**
   ```c
   /**
    * @brief Create a video decoder.
    * @param device Vulkan device
    * @return Pointer to created decoder, or NULL on failure
    */
   ```

## 构建与使用

### 编译要求

- Vulkan SDK 1.3+
- wlframe 核心库
- Meson 构建系统
- 支持 Vulkan Video 的 GPU 驱动

### 构建步骤

```bash
# 配置构建
meson setup build/ --buildtype=debug

# 编译
ninja -C build/

# 运行示例
./build/examples/video/video_decoder_example
./build/examples/video/video_encoder_example
```

### 集成到项目

```meson
# meson.build
wlframe_dep = dependency('wlframe-0.0')

executable('my_video_app',
    'main.c',
    dependencies: [wlframe_dep],
)
```

```c
// main.c
#include <wlf/video/wlf_video_decoder.h>
#include <wlf/video/wlf_video_encoder.h>

// 使用 API...
```

## 硬件支持

### 当前支持

- **NVIDIA**
  - RTX 30 系列 (Ampere)
  - RTX 40 系列 (Ada Lovelace)
  - A 系列数据中心 GPU
  - 驱动版本: 525.60.11+

### 未来支持

- **AMD**
  - RDNA 3+ 架构
  - 需要驱动支持

- **Intel**
  - Arc 系列
  - 需要驱动支持

## 性能优化

### 解码优化

1. **缓冲预分配**: 预先分配足够的 DPB 槽位
2. **异步操作**: 使用异步 API 避免阻塞
3. **批量处理**: 批量提交解码命令
4. **零拷贝路径**: 直接使用 GPU 内存

### 编码优化

1. **GOP 优化**: 根据内容调整 GOP 结构
2. **码率自适应**: 动态调整目标码率
3. **B 帧使用**: 合理使用 B 帧提高压缩率
4. **多线程**: 并行编码多个帧

## 示例代码

详见 `examples/video/` 目录：

- `video_decoder_example.c` - 完整的解码器使用示例
- `video_encoder_example.c` - 完整的编码器使用示例

## 已知限制

1. **硬件依赖**: 需要支持 Vulkan Video 的 GPU
2. **驱动要求**: 需要最新的驱动程序
3. **格式支持**: 某些格式可能受硬件限制
4. **实时性**: 编码延迟取决于硬件性能

## 未来计划

- [ ] VP9 编码支持
- [ ] 10-bit 编码优化
- [ ] HDR 元数据处理
- [ ] 硬件 Overlay 渲染
- [ ] 自适应流支持
- [ ] 转码管道
- [ ] 多 GPU 支持
- [ ] 视频滤镜集成

## 参考资料

- [Vulkan Video Samples](https://github.com/KhronosGroup/Vulkan-Video-Samples)
- [Vulkan Specification](https://www.khronos.org/registry/vulkan/)
- [Vulkan Video Deep Dive](https://www.khronos.org/assets/uploads/apis/Vulkan-Video-Deep-Dive-Apr21.pdf)
- [wlframe Documentation](../docs/)

## 许可证

与 wlframe 相同的许可证。

## 作者与贡献

- 作者: YaoBing Xiao
- 日期: 2026-01-23
- 版本: v1.0

参考实现: Khronos Group Vulkan-Video-Samples

---

如有问题或建议，请提交 Issue 或 Pull Request。
