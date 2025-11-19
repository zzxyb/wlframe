# wlframe Video Hardware Decode/Encode Backend Architecture

## Overview

The wlframe video subsystem implements a multi-backend architecture inspired by mpv, supporting:

- **Vulkan Video** - Modern cross-platform hardware video acceleration
- **VA-API** - Linux video acceleration API (Intel, AMD)
- **Software Decode** - FFmpeg-based fallback for maximum compatibility

## Architecture Design

### Backend Abstraction Layer

The `wlf_hwdec` system provides a unified interface for hardware decode:

```c
struct wlf_hwdec_device_impl {
    const char *name;
    enum wlf_hwdec_type type;

    bool (*init)(struct wlf_hwdec_device *device);
    void (*destroy)(struct wlf_hwdec_device *device);
    bool (*supports_codec)(struct wlf_hwdec_device *device,
        enum wlf_video_codec codec);
    bool (*decode_frame)(struct wlf_hwdec_device *device,
        const uint8_t *bitstream, size_t size,
        struct wlf_video_image *output);
};
```

### Backend Priority

When in **auto mode** (default), backends are selected in this priority order:

1. **VA-API** - Preferred for Linux systems (mature, stable)
2. **Vulkan** - Modern cross-platform alternative
3. **Software** - Always available as final fallback

This prioritization ensures best compatibility on Linux while maintaining flexibility.

### Environment Variable Control

The backend can be controlled via the `WLF_HWDEC_BACKEND` environment variable:

```bash
# Use VA-API backend
export WLF_HWDEC_BACKEND=vaapi

# Use Vulkan backend
export WLF_HWDEC_BACKEND=vulkan

# Use software decode only
export WLF_HWDEC_BACKEND=software

# Auto-select (prefers VA-API > Vulkan > Software)
export WLF_HWDEC_BACKEND=auto
# or simply don't set the variable
```

The environment variable takes precedence over the `preferred_backend` parameter in code.

### Automatic Backend Selection

```c
/* Auto-select best available backend (prefers VA-API) */
struct wlf_hwdec_context *ctx = wlf_hwdec_context_create(
    "auto", true  /* enable_fallback */
);
/* or */
ctx = wlf_hwdec_context_create(NULL, true);

/* Select backend for specific codec */
struct wlf_hwdec_device *device = wlf_hwdec_get_device(
    ctx, WLF_VIDEO_CODEC_H264
);

/* Decode frame */
bool result = device->impl->decode_frame(device, bitstream, size, &output);
```

## Backend Details

### Vulkan Video Backend

**Features:**
- H.264, H.265, AV1 decode/encode
- Cross-platform (Linux, Windows)
- Modern GPU requirements

**API Extensions:**
- `VK_KHR_video_queue`
- `VK_KHR_video_decode_queue`
- `VK_KHR_video_encode_queue`
- Codec-specific extensions

**Limitations:**
- Limited VP9 support
- Requires recent GPU drivers

### VA-API Backend

**Features:**
- H.264, H.265, VP9, AV1 decode
- Hardware encode support
- Wayland integration
- Mature Linux support

**Requirements:**
- libva >= 1.0
- Wayland or X11 display
- Compatible GPU (Intel, AMD)

**Wayland Integration:**
```c
VADisplay va_display = wlf_va_display_create_wayland(wl_display);
bool decode_ok, encode_ok;
wlf_va_query_codec_support(va_display, WLF_VIDEO_CODEC_H264,
    &decode_ok, &encode_ok);
```

### Software Backend

**Features:**
- All codecs supported via FFmpeg
- CPU-based decode
- No hardware requirements

**Use Cases:**
- Systems without GPU
- Unsupported codecs
- Development/testing
- Fallback when hardware fails

## Usage Examples

### Basic Decode

```c
#include <wlf/va/wlf_hwdec.h>

/* Initialize with auto backend selection (prefers VA-API) */
struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("auto", true);

/* Select device for H.264 */
struct wlf_hwdec_device *device = wlf_hwdec_get_device(
    ctx, WLF_VIDEO_CODEC_H264);

/* Decode bitstream */
struct wlf_video_image output;
bool result = device->impl->decode_frame(device,
    bitstream_data, bitstream_size, &output);

/* Cleanup */
wlf_hwdec_context_destroy(ctx);
```

### Environment Variable Override

```bash
# Force use of Vulkan backend
export WLF_HWDEC_BACKEND=vulkan
./my_application

# Use VA-API
export WLF_HWDEC_BACKEND=vaapi
./my_application

# Software decode only (no hardware acceleration)
export WLF_HWDEC_BACKEND=software
./my_application
```

### Specific Backend

```c
/* Force Vulkan backend in code */
struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("vulkan", false);

if (!ctx) {
    /* Vulkan not available, try VA-API */
    ctx = wlf_hwdec_context_create("vaapi", false);
}
```

### Query Capabilities

```c
struct wlf_hwdec_device *device = wlf_hwdec_get_device(ctx, codec);

if (device && device->impl->supports_codec) {
    bool supported = device->impl->supports_codec(device, WLF_VIDEO_CODEC_AV1);
    /* AV1 supported? */
}

if (device && device->impl->supports_format) {
    bool supported = device->impl->supports_format(device, format);
    /* Format supported? */
}
```

## Build Configuration

### Meson Options

Enable/disable backends:

```bash
# Configure with all backends
meson setup build -Dvulkan=enabled -Dlibva=enabled

# Vulkan only
meson setup build -Dvulkan=enabled -Dlibva=disabled

# Software only (no hardware acceleration)
meson setup build -Dvulkan=disabled -Dlibva=disabled
```

### Compile-Time Flags

- `WLF_HAS_VULKAN` - Vulkan backend available
- `WLF_HAS_VAAPI` - VA-API backend available

## Codec Support Matrix

| Codec | Vulkan | VA-API | Software |
|-------|--------|--------|----------|
| H.264 | ✓      | ✓      | ✓        |
| H.265 | ✓      | ✓      | ✓        |
| VP9   | ✗      | ✓      | ✓        |
| AV1   | ✓      | ✓      | ✓        |

## Performance Considerations

### Backend Selection Strategy

1. **Vulkan First** - Best performance on modern systems
2. **VA-API Fallback** - Good performance on Linux
3. **Software Last** - Guaranteed compatibility

### Memory Management

- Vulkan: GPU memory allocation, zero-copy to compositor
- VA-API: Shared DRM buffer, direct scanout possible
- Software: System memory, requires copy

### Latency

- Hardware decode: 1-3 frames latency
- Software decode: 0-1 frames latency (but higher CPU usage)

## Integration with wlframe

### Wayland Integration

VA-API integrates directly with Wayland via `wlf_va_display_create_wayland()`:

```c
struct wl_display *wl_display = ...;
VADisplay va_display = wlf_va_display_create_wayland(wl_display);

/* Use VA display for decode */
```

### Zero-Copy Rendering

Future work: Direct buffer sharing between decoder and compositor.

## Error Handling

```c
struct wlf_hwdec_context *ctx = wlf_hwdec_context_create(
    WLF_HWDEC_AUTO, true);

if (!ctx) {
    /* No backends available */
    wlf_log(WLF_ERROR, "Failed to initialize any decode backend");
    return -1;
}

struct wlf_hwdec_device *device = wlf_hwdec_context_select_device(
    ctx, codec, 0);

if (!device) {
    /* Codec not supported by any backend */
    wlf_log(WLF_ERROR, "Codec %d not supported", codec);
}
```

## References

- [mpv hwdec architecture](https://github.com/mpv-player/mpv/tree/master/video/decode)
- [Vulkan Video Samples](https://github.com/KhronosGroup/Vulkan-Video-Samples)
- [VA-API Documentation](https://github.com/intel/libva)
- [FFmpeg libavcodec](https://ffmpeg.org/libavcodec.html)

## Future Enhancements

- [ ] VDPAU backend (NVIDIA Linux)
- [ ] D3D11VA backend (Windows)
- [ ] VideoToolbox backend (macOS/iOS)
- [ ] OpenMAX backend (Raspberry Pi)
- [ ] Hardware encode via hwdec abstraction
- [ ] Zero-copy buffer sharing
- [ ] Multi-threaded software decode
