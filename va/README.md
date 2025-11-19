# wlframe Video Codec Library

A multi-backend hardware-accelerated video encoding and decoding library for wlframe.

## Backend Architecture

The library supports multiple hardware acceleration backends with automatic fallback:

- **VA-API** - Linux hardware video acceleration (Intel, AMD)
- **Vulkan Video** - Cross-platform modern GPU acceleration
- **Software** - FFmpeg-based CPU decode (fallback)

### Environment Variable Control

Control the backend via the `WLF_HWDEC_BACKEND` environment variable:

```bash
# Use VA-API (default in auto mode on Linux)
export WLF_HWDEC_BACKEND=vaapi

# Use Vulkan Video
export WLF_HWDEC_BACKEND=vulkan

# Use software decode only
export WLF_HWDEC_BACKEND=software

# Auto mode (prefers VA-API > Vulkan > Software)
export WLF_HWDEC_BACKEND=auto
# Or don't set it at all
```

## Features

### Multi-Backend Support
- **VA-API Backend**: Linux hardware acceleration (mature, stable)
- **Vulkan Backend**: Cross-platform modern GPU acceleration
- **Software Backend**: FFmpeg-based fallback (always available)
- **Automatic Selection**: Smart backend selection with fallback
- **Runtime Control**: Environment variable override

### Wayland Zero-Copy Integration
- **Direct wl_buffer Export**: Convert decoded frames to wl_buffer
- **VA-API**: Zero-copy via `vaGetSurfaceBufferWl()`
- **Vulkan**: DMA-BUF export via `VK_KHR_external_memory_fd`
- **Software**: wl_shm buffer (shared memory)
- **Compositor Ready**: Buffers can be directly attached and committed

### Video Decoding
- **Codec Support**: H.264/AVC, H.265/HEVC, AV1, VP9
- **Chroma Formats**: 4:0:0, 4:2:0, 4:2:2, 4:4:4
- **Bit Depth**: 8-bit, 10-bit, 12-bit support
- **Features**:
  - Hardware-accelerated decoding
  - Decoded Picture Buffer (DPB) management
  - Reference frame handling
  - Multi-threaded support

### Video Encoding
- **Codec Support**: H.264/AVC, H.265/HEVC, AV1
- **Rate Control**: CBR, VBR, CQP modes
- **GOP Structure**: Configurable I/P/B frame patterns
- **Features**:
  - Hardware-accelerated encoding
  - Multiple encoding profiles and levels
  - Adaptive bitrate control
  - B-frame support

## Architecture

The library follows wlframe's modular design with multi-backend support:

```
va/
├── wlf_hwdec.h              # Backend abstraction layer
├── wlf_hwdec.c              # Backend manager
├── wlf_hwdec_vulkan.c       # Vulkan backend implementation
├── wlf_hwdec_vaapi.c        # VA-API backend implementation
├── wlf_hwdec_software.c     # Software backend implementation
├── wlf_video_common.h       # Common definitions and types
├── wlf_video_decoder.h/c    # Decoder API
├── wlf_video_encoder.h/c    # Encoder API
└── wayland/
    └── va_display_wayland.c # Wayland integration
```

## API Design

### Backend Selection

```c
#include <wlf/va/wlf_hwdec.h>

/* Auto-select best backend (prefers VA-API) */
struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("auto", true);

/* Or specify backend */
ctx = wlf_hwdec_context_create("vaapi", false);
ctx = wlf_hwdec_context_create("vulkan", false);
ctx = wlf_hwdec_context_create("software", false);

/* Get device for specific codec */
struct wlf_hwdec_device *device = wlf_hwdec_get_device(ctx, WLF_VIDEO_CODEC_H264);

/* Decode frame */
device->impl->decode_frame(device, bitstream_data, size, &output_image);

/* Export to wl_buffer for Wayland compositing */
wlf_hwdec_set_wayland_display(device, wl_display);
struct wl_buffer *buffer = wlf_hwdec_export_to_wl_buffer(device, &output_image, NULL);

/* Use buffer with Wayland compositor */
wl_surface_attach(surface, buffer, 0, 0);
wl_surface_damage(surface, 0, 0, width, height);
wl_surface_commit(surface);

/* Cleanup */
wl_buffer_destroy(buffer);
wlf_hwdec_context_destroy(ctx);
```

### Wayland Zero-Copy Integration

```c
#include <wlf/va/wlf_hwdec.h>
#include <wayland-client.h>

/* Setup hardware decoder */
struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("auto", true);
struct wlf_hwdec_device *device = wlf_hwdec_get_device(ctx, WLF_VIDEO_CODEC_H264);

/* Connect Wayland display to decoder */
wlf_hwdec_set_wayland_display(device, wl_display);

/* Decode video frame */
struct wlf_video_image decoded_frame;
device->impl->decode_frame(device, bitstream, size, &decoded_frame);

/* Export directly to wl_buffer (zero-copy for VA-API/Vulkan) */
struct wl_buffer *buffer = wlf_hwdec_export_to_wl_buffer(
    device, &decoded_frame, wl_display);

if (buffer) {
    /* Attach and commit to Wayland surface */
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_commit(surface);

    /* Compositor can now scan out directly from GPU memory */
}
```

### Legacy Decoder API (Vulkan-specific)

```c
#include "wlf/va/wlf_video_decoder.h"

/* Configure decoder */
struct wlf_video_decoder_config config = {
    .codec = WLF_VIDEO_CODEC_H264,
    .max_width = 1920,
    .max_height = 1080,
    .max_dpb_slots = 16,
    .chroma = WLF_VIDEO_CHROMA_420,
    .bit_depth = 8,
};

/* Create decoder */
struct wlf_video_decoder *decoder = wlf_video_decoder_create(
    device, physical_device, &config);

/* Decode frame */
struct wlf_video_image output_image;
wlf_video_decoder_decode_frame(decoder, bitstream_data, size, &output_image);

/* Cleanup */
wlf_video_decoder_destroy(decoder);
```

### Encoder Example

```c
#include "wlf/video/wlf_video_encoder.h"

/* Configure encoder */
struct wlf_video_encoder_config config = {
    .codec = WLF_VIDEO_CODEC_H264,
    .width = 1920,
    .height = 1080,
    .framerate_num = 30,
    .framerate_den = 1,
    .rate_control_mode = WLF_VIDEO_RATE_CONTROL_CBR,
    .target_bitrate = 5000000,
    .gop_size = 60,
    .num_b_frames = 2,
};

/* Create encoder */
struct wlf_video_encoder *encoder = wlf_video_encoder_create(
    device, physical_device, &config);

/* Encode frame */
struct wlf_video_encoded_frame output_frame;
wlf_video_encoder_encode_frame(encoder, input_image, &output_frame);

/* Cleanup */
wlf_video_encoder_destroy(encoder);
```

## Implementation Details

### Vulkan Video Extensions

The library uses the following Vulkan extensions:
- `VK_KHR_video_queue` - Core video queue support
- `VK_KHR_video_decode_queue` - Video decode operations
- `VK_KHR_video_encode_queue` - Video encode operations
- `VK_KHR_video_decode_h264` - H.264 decode
- `VK_KHR_video_decode_h265` - H.265 decode
- `VK_KHR_video_decode_av1` - AV1 decode
- `VK_KHR_video_encode_h264` - H.264 encode
- `VK_KHR_video_encode_h265` - H.265 encode
- `VK_KHR_video_encode_av1` - AV1 encode

### Memory Management

- **Reference Counting**: All video resources use reference counting
- **Buffer Pooling**: Reusable buffer pools for bitstreams
- **DPB Management**: Automatic decoded picture buffer management
- **Zero-Copy**: Minimal data copying between CPU and GPU

### Threading Model

- **Async Operations**: Video operations execute asynchronously
- **Command Buffers**: Vulkan command buffers for GPU work submission
- **Synchronization**: Fences and semaphores for CPU-GPU sync
- **Thread Safety**: Safe for multi-threaded applications

## Code Style

Follows wlframe conventions:
- Snake_case for functions and variables: `wlf_video_decoder_create()`
- Struct-based objects: `struct wlf_video_decoder`
- Implementation interface pattern: `struct wlf_video_decoder_impl`
- Signal/event system: `decoder->events.frame_decoded`
- Comprehensive error logging via `wlf_log()`
- Doxygen documentation comments

## Building

The library integrates with wlframe's meson build system:

```bash
# Build with video support
meson setup build/ -Dvideo=enabled
ninja -C build/

# Build examples
cd build/examples/video/
./video_decoder_example
./video_encoder_example
```

## Requirements

- Vulkan 1.3 or later
- GPU with Vulkan Video support
- Vulkan Video extensions enabled
- wlframe core libraries

### Supported Hardware

- **NVIDIA**: RTX 30/40 series, A-series (driver 525.60.11+)
- **AMD**: RDNA 3+ (driver TBD)
- **Intel**: Arc series (driver TBD)

## Testing

Query decoder capabilities:
```c
VkVideoCapabilitiesKHR caps;
wlf_video_decoder_query_capabilities(physical_device, WLF_VIDEO_CODEC_H264, &caps);
```

Query encoder capabilities:
```c
VkVideoCapabilitiesKHR caps;
wlf_video_encoder_query_capabilities(physical_device, WLF_VIDEO_CODEC_H264, &caps);
```

## Future Enhancements

- [ ] VP9 encoding support
- [ ] HDR metadata handling
- [ ] Hardware overlay rendering
- [ ] Adaptive streaming support
- [ ] Transcoding pipeline
- [ ] Multi-GPU encoding/decoding
- [ ] Video filter integration

## References

- [Vulkan Video Samples](https://github.com/KhronosGroup/Vulkan-Video-Samples)
- [Vulkan Video Specification](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html)
- [Vulkan Video Deep Dive](https://www.khronos.org/assets/uploads/apis/Vulkan-Video-Deep-Dive-Apr21.pdf)

## License

Same license as wlframe.

## Author

YaoBing Xiao - 2026-01-23
