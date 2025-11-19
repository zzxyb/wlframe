# Video Recorder

视频录制功能，支持 dmabuf、PipeWire 和 Wayland SHM 三种后端，参考 Weston 的屏幕录制实现。

## 功能特性

- **多后端支持**
  - **DMA-BUF 后端**: 直接从 DMA-BUF 缓冲区录制，适合直接访问显存的场景
  - **PipeWire 后端**: 通过 PipeWire 协议录制屏幕，支持 Wayland 混成器
  - **Wayland SHM 后端**: 从 Wayland 共享内存缓冲区录制，支持软件渲染

- **视频编码**
  - 支持 H.264、H.265 (HEVC)、AV1 编解码器
  - 可配置的码率控制 (CBR、VBR、CQP)
  - 可调节的质量设置

- **容器格式**
  - MP4
  - WebM
  - Matroska (MKV)
  - AVI

- **高级功能**
  - 暂停/恢复录制
  - 帧队列管理和缓冲
  - 丢帧策略
  - 实时统计信息
  - 多线程编码

## 架构设计

```
┌─────────────────────────────────────────┐
│         wlf_video_recorder              │
│  (Main recorder orchestration)          │
│  - Frame queue management               │
│  - Encoder thread control               │
│  - File I/O                             │
│  - Statistics tracking                  │
└──────────┬──────────────┬───────────────┘
           │              │
           ▼              ▼
    ┌──────────┐   ┌────────────┐
    │  Backend │   │   Encoder  │
    │Interface │   │  (VA-API/  │
    │          │   │   Vulkan)  │
    └────┬─────┘   └────────────┘
         │
    ┌────┴──────────┬─────────────┐
    ▼               ▼             ▼
┌─────────┐   ┌──────────┐   ┌──────┐
│ DMA-BUF │   │ PipeWire │   │Future│
│ Backend │   │ Backend  │   │......│
└─────────┘   └──────────┘   └──────┘
```

### 主要组件

1. **wlf_video_recorder**: 主录制器
   - 管理录制生命周期
   - 协调后端和编码器
   - 处理帧队列
   - 写入输出文件

2. **wlf_recorder_backend**: 后端接口
   - 抽象不同的捕获源
   - 提供统一的帧回调机制

3. **wlf_recorder_dmabuf_backend**: DMA-BUF 后端
   - 接收 DMA-BUF 帧
   - 直接访问 GPU 内存

4. **wlf_recorder_pipewire_backend**: PipeWire 后端
   - 连接 PipeWire 守护进程
   - 捕获 Wayland 混成器输出
   - 处理流事件
5. **wlf_recorder_wayland_shm_backend**: Wayland SHM 后端
   - 接收 Wayland 共享内存缓冲区
   - 支持软件渲染场景
   - 将 SHM 数据转换为编码格式
## API 使用

### 基本用法

```c
#include "wlf/va/wlf_video_recorder.h"
#include "wlf/va/wlf_recorder_backend.h"

/* 配置录制器 */
struct wlf_recorder_config config = {0};
config.output_filename = "recording.mp4";
config.format = WLF_RECORDER_FORMAT_MP4;

/* 编码器配置 */
config.encoder_config.codec = WLF_VIDEO_CODEC_H264;
config.encoder_config.width = 1920;
config.encoder_config.height = 1080;
config.encoder_config.framerate_num = 30;
config.encoder_config.framerate_den = 1;
config.encoder_config.rate_control_mode = WLF_VIDEO_RATE_CONTROL_VBR;
config.encoder_config.target_bitrate = 5000000;  /* 5 Mbps */

/* 创建 PipeWire 后端 */
struct wlf_recorder_backend *backend =
    wlf_recorder_pipewire_backend_create(NULL, 0, NULL, NULL, NULL);

/* 创建录制器 */
struct wlf_video_recorder *recorder = wlf_video_recorder_create(backend, &config);

/* 开始录制 */
wlf_video_recorder_start(recorder);

/* 录制一段时间... */
sleep(10);

/* 停止录制 */
wlf_video_recorder_stop(recorder);

/* 销毁录制器 */
wlf_video_recorder_destroy(recorder);
```

### DMA-BUF 后端

```c
/* 创建 DMA-BUF 后端 */
struct wlf_recorder_backend *backend =
    wlf_recorder_dmabuf_backend_create(NULL, NULL, NULL);

struct wlf_video_recorder *recorder = wlf_video_recorder_create(backend, &config);
wlf_video_recorder_start(recorder);

/* 提交 DMA-BUF 帧 */
struct wlf_dmabuf_attributes attribs;
/* ... 填充 DMA-BUF 属性 ... */
uint64_t timestamp = wlf_time_get_microseconds();
wlf_video_recorder_submit_dmabuf(recorder, &attribs, timestamp);
```

/* 创建 PipeWire 后端 */
uint32_t node_id = 0;  /* 自动检测 */
const char *node_name = NULL;  /* 或指定节点名称 */
struct wlf_recorder_backend *backend =
    wlf_recorder_pipewire_backend_create(NULL, node_id, node_name, NULL, NULL);

struct wlf_video_recorder *recorder = wlf_video_recorder_create(backend,
config.pipewire_node_name = NULL;  /* 或指定节点名称 */

struct wlf_video_recorder *recorder = wlf_video_recorder_create(&config);
wlf_video_recorder_start(recorder);

/* PipeWire 后端会自动捕获帧 */
```

### Wayland SHM 后端

```c
/* 创建 Wayland SHM 后端 */
struct wlf_recorder_backend *backend =
    wlf_recorder_wayland_shm_backend_create(NULL, NULL, NULL);

struct wlf_video_recorder *recorder = wlf_video_recorder_create(backend, &config);
wlf_video_recorder_start(recorder);

/* 提交 Wayland SHM 缓冲区 */
/* 方式 1: 直接传递 SHM 数据指针 */
void *shm_data = /* ... 映射的 SHM 数据 ... */;
uint32_t width = 1920;
uint32_t height = 1080;
uint32_t stride = width * 4;  /* ARGB8888 */
uint32_t format = 0;  /* WL_SHM_FORMAT_ARGB8888 */
uint64_t timestamp = wlf_time_get_microseconds();

wlf_recorder_wayland_shm_backend_submit_buffer(
    backend, shm_data, width, height, stride, format, timestamp);

/* 方式 2: 使用文件描述符 */
int shm_fd = /* ... SHM 文件描述符 ... */;
uint32_t offset = 0;

wlf_recorder_wayland_shm_backend_submit_buffer_fd(
    backend, shm_fd, offset, width, height, stride, format, timestamp);
```

### 暂停和恢复

```c
/* 暂停录制 */
wlf_video_recorder_pause(recorder);

/* 做其他事情... */

/* 恢复录制 */
wlf_video_recorder_resume(recorder);
```

### 获取统计信息

```c
struct wlf_recorder_statistics stats;
wlf_video_recorder_get_statistics(recorder, &stats);

printf("Captured: %lu frames\n", stats.total_frames_captured);
printf("Encoded:  %lu frames\n", stats.total_frames_encoded);
printf("Dropped:  %lu frames\n", stats.total_frames_dropped);
printf("FPS:      %.2f\n", stats.average_fps);
printf("Size:     %lu bytes\n", stats.total_bytes_written);
```

## 示例程序

### 编译

```bash
cd build
ninja
```

### 运行

```bash
# 使用 PipeWire 后端录制 10 秒
./examples/recorder_demo -b pipewire -o screen.mp4 -d 10

# 使用 DMA-BUF 后端，H.265 编码，质量 90
./examples/recorder_demo -b dmabuf -o output.mp4 -c h265 -q 90 -d 30

# 使用 Wayland SHM 后端录制
./examples/recorder_demo -b wayland-shm -o recording.mp4 -d 10

# 自定义分辨率和帧率
./examples/recorder_demo -w 2560 -h 1440 -f 60 -d 5

# 指定 PipeWire 节点 ID
./examples/recorder_demo -b pipewire -n 42 -o recording.mp4
```

### 命令行参数

- `-b, --backend TYPE`: 后端类型 (dmabuf、pipewire 或 wayland-shm)
- `-o, --output FILE`: 输出文件名
- `-w, --width WIDTH`: 视频宽度
- `-h, --height HEIGHT`: 视频高度
- `-f, --fps FPS`: 帧率
- `-c, --codec CODEC`: 编解码器 (h264, h265, av1)
- `-q, --quality QUALITY`: 质量 1-100
- `-d, --duration SECS`: 录制时长（秒）
- `-n, --node-id ID`: PipeWire 节点 ID

## 后端对比

| 特性 | DMA-BUF | PipeWire | Wayland SHM |
|------|---------|----------|-------------|
| **性能** | 最高（零拷贝） | 高 | 中等（需要复制） |
| **硬件加速** | 是 | 是 | 否 |
| **CPU 使用** | 最低 | 低 | 较高 |
| **兼容性** | 需要 GPU 支持 | 广泛支持 | 最广泛 |
| **使用场景** | GPU 渲染应用 | 桌面屏幕录制 | 软件渲染应用 |
| **零拷贝** | 是 | 取决于实现 | 否 |
| **延迟** | 最低 | 低 | 中等 |

## 依赖

### 必需

- libva (VA-API)
- libdrm
- pthread

### 可选

- libpipewire-0.3 (PipeWire 后端)
- libspa-0.2 (PipeWire 后端)

## 性能优化

1. **帧队列大小**: 调整 `max_buffer_frames` 以平衡延迟和丢帧
2. **丢帧策略**: 使用 `drop_frames_on_overflow` 在高负载时丢帧
3. **编码器线程**: 独立线程处理编码，避免阻塞捕获
4. **码率控制**: 使用 VBR 获得更好的质量/大小比

## 限制和已知问题

1. **音频支持**: 目前音频录制尚未实现
2. **容器格式**: MP4 容器需要手动添加 muxer 支持
3. **编码器集成**: 需要与现有的 `wlf_video_encoder` 集成
4. **颜色空间转换**: DMA-BUF 到编码器格式的转换待实现

## TODO

- [ ] 实现音频录制支持
- [ ] 集成 libavformat/ffmpeg 用于容器 muxing
- [ ] 完成 DMA-BUF 到编码器的导入
- [ ] 添加 H.264/H.265 参数集配置
- [ ] 实现动态码率调整
- [ ] 添加硬件加速的颜色空间转换
- [ ] 支持多轨音频
- [ ] 添加字幕/元数据支持

## 参考

- [Weston Screen Recorder](https://gitlab.freedesktop.org/wayland/weston/-/blob/main/libweston/screenshooter.c)
- [PipeWire Screen Cast](https://docs.pipewire.org/page_tutorial5.html)
- [VA-API Encoding Guide](https://github.com/intel/libva-utils)

## 许可证

与 wlframe 项目相同
