# Zero-Copy Video Decode to Wayland Compositor

## Overview

wlframe's video decode backends support direct export of decoded frames to `wl_buffer`, enabling zero-copy (or minimal-copy) presentation to Wayland compositors.

## Architecture

### Flow Diagram

```
Video Bitstream
     ↓
Hardware Decoder (VA-API/Vulkan/Software)
     ↓
Decoded Frame (GPU Memory / Shared Memory)
     ↓
wl_buffer Export
     ↓
wl_surface_attach() + wl_surface_commit()
     ↓
Wayland Compositor (Direct Scanout)
```

## Backend Implementation Details

### VA-API Backend (Zero-Copy)

**Method**: `vaGetSurfaceBufferWl()`

- Decoded frames stored in VA surfaces (GPU memory)
- Direct export to `wl_buffer` via VA-API Wayland extension
- **True zero-copy**: No CPU involvement, no memory copy
- Compositor can scan out directly from GPU memory

**Advantages**:
- ✓ Best performance
- ✓ No CPU overhead
- ✓ Lowest latency
- ✓ Mature and widely supported

**Requirements**:
- libva with Wayland support
- VA-API driver supporting Wayland
- Wayland compositor

**Code**:
```c
VASurfaceID va_surface = /* decoded surface */;
struct wl_buffer *buffer;

VAStatus status = vaGetSurfaceBufferWl(
    va_display, va_surface,
    VA_FRAME_PICTURE, &buffer);

/* Buffer is ready for attachment */
wl_surface_attach(surface, buffer, 0, 0);
wl_surface_commit(surface);
```

### Vulkan Backend (Zero-Copy via DMA-BUF)

**Method**: DMA-BUF export + linux-dmabuf protocol

- Decoded frames stored in Vulkan images (GPU memory)
- Export image memory as DMA-BUF file descriptor
- Import to Wayland via `zwp_linux_dmabuf_v1` protocol
- **True zero-copy**: Direct GPU memory sharing

**Advantages**:
- ✓ Zero-copy presentation
- ✓ Cross-vendor GPU support
- ✓ Modern and future-proof

**Requirements**:
- VK_KHR_external_memory_fd extension
- VK_EXT_external_memory_dma_buf extension
- Wayland compositor with linux-dmabuf protocol
- DRM/KMS support

**Code**:
```c
/* Export Vulkan memory as DMA-BUF */
VkMemoryGetFdInfoKHR fd_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
    .memory = image_memory,
    .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
};

int dma_buf_fd;
vkGetMemoryFdKHR(device, &fd_info, &dma_buf_fd);

/* Create wl_buffer via linux-dmabuf */
struct zwp_linux_buffer_params_v1 *params =
    zwp_linux_dmabuf_v1_create_params(dmabuf);

zwp_linux_buffer_params_v1_add(params,
    dma_buf_fd,    /* fd */
    0,             /* plane_idx */
    0,             /* offset */
    stride,        /* stride */
    modifier >> 32,/* modifier_hi */
    modifier);     /* modifier_lo */

struct wl_buffer *buffer =
    zwp_linux_buffer_params_v1_create_immed(params,
        width, height, format, 0);
```

### Software Backend (Copy via wl_shm)

**Method**: Shared memory buffer

- Decoded frames in system memory (AVFrame)
- Convert to ARGB8888 format
- Copy to wl_shm buffer (shared memory)
- **Not zero-copy**: Requires format conversion and memory copy

**Advantages**:
- ✓ Always available (no GPU required)
- ✓ Simple and reliable
- ✓ Compatible with all Wayland compositors

**Disadvantages**:
- ✗ Memory copy overhead
- ✗ Format conversion overhead
- ✗ Higher CPU usage

**Requirements**:
- wl_shm support (universal in Wayland)
- FFmpeg with libswscale

**Code**:
```c
/* Create shared memory */
int fd = memfd_create("video-frame", MFD_CLOEXEC);
ftruncate(fd, size);
void *data = mmap(NULL, size, PROT_READ|PROT_WRITE,
    MAP_SHARED, fd, 0);

/* Convert frame format */
struct SwsContext *sws = sws_getContext(
    width, height, av_format,
    width, height, AV_PIX_FMT_BGRA,
    SWS_BILINEAR, NULL, NULL, NULL);

sws_scale(sws, frame->data, frame->linesize,
    0, height, &data, &stride);

/* Create wl_shm buffer */
struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
struct wl_buffer *buffer = wl_shm_pool_create_buffer(
    pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
```

## API Usage

### Basic Usage

```c
#include <wlf/va/wlf_hwdec.h>

/* 1. Create decoder context */
struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("auto", true);
struct wlf_hwdec_device *device = wlf_hwdec_get_device(ctx, codec);

/* 2. Set Wayland display */
wlf_hwdec_set_wayland_display(device, wl_display);

/* 3. Decode frame */
struct wlf_video_image image;
device->impl->decode_frame(device, bitstream, size, &image);

/* 4. Export to wl_buffer */
struct wl_buffer *buffer = wlf_hwdec_export_to_wl_buffer(
    device, &image, NULL);

/* 5. Present to compositor */
wl_surface_attach(surface, buffer, 0, 0);
wl_surface_damage(surface, 0, 0, width, height);
wl_surface_commit(surface);

/* 6. Cleanup */
wl_buffer_destroy(buffer);
```

### Error Handling

```c
struct wl_buffer *buffer = wlf_hwdec_export_to_wl_buffer(
    device, &image, wl_display);

if (!buffer) {
    wlf_log(WLF_ERROR, "Failed to export buffer");

    /* Check if backend supports export */
    if (!device->impl->export_to_wl_buffer) {
        wlf_log(WLF_ERROR, "Backend doesn't support wl_buffer export");
    }

    /* Fallback: render to texture or use alternative path */
    return;
}
```

## Performance Comparison

| Backend  | Memory Copy | CPU Overhead | Latency | Best For |
|----------|-------------|--------------|---------|----------|
| VA-API   | Zero-copy   | Minimal      | Lowest  | Intel/AMD GPUs on Linux |
| Vulkan   | Zero-copy   | Low          | Low     | Modern cross-platform |
| Software | Full copy   | High         | Higher  | CPU-only systems, testing |

## Compositor Compatibility

### Zero-Copy Support

Most modern Wayland compositors support zero-copy presentation:

**Full Support**:
- Weston
- Mutter (GNOME)
- KWin (KDE Plasma)
- Sway
- Hyprland

**Requirements**:
- DRM/KMS support
- GBM (Generic Buffer Management)
- For Vulkan: linux-dmabuf protocol

### Fallback Behavior

If zero-copy is not available:
1. VA-API: Falls back to vaGetImage + wl_shm
2. Vulkan: Falls back to vkCmdCopyImageToBuffer + wl_shm
3. Software: Uses wl_shm (always)

## Best Practices

### 1. Backend Selection

```c
/* Prefer VA-API on Linux for best zero-copy support */
export WLF_HWDEC_BACKEND=vaapi

/* Or let the system auto-select */
export WLF_HWDEC_BACKEND=auto
```

### 2. Buffer Lifecycle Management

```c
/* Keep decoded image alive while buffer is in use */
struct wlf_video_image image;
device->impl->decode_frame(device, bitstream, size, &image);

struct wl_buffer *buffer = wlf_hwdec_export_to_wl_buffer(
    device, &image, NULL);

wl_surface_attach(surface, buffer, 0, 0);
wl_surface_commit(surface);

/* Wait for compositor to release buffer */
wl_buffer_add_listener(buffer, &buffer_listener, NULL);

/* Clean up after release event */
```

### 3. Multiple Buffers for Smooth Playback

```c
#define NUM_BUFFERS 3

struct video_buffer {
    struct wlf_video_image image;
    struct wl_buffer *wl_buffer;
    bool in_use;
};

struct video_buffer buffers[NUM_BUFFERS];

/* Rotate buffers for continuous playback */
int current = 0;
while (playing) {
    struct video_buffer *buf = &buffers[current];

    if (!buf->in_use) {
        decode_frame(&buf->image);
        buf->wl_buffer = export_to_wl_buffer(&buf->image);
        present(buf->wl_buffer);
        buf->in_use = true;
    }

    current = (current + 1) % NUM_BUFFERS;
}
```

## Debugging

### Enable Debug Logging

```bash
export WLF_LOG_LEVEL=debug
export WLF_HWDEC_BACKEND=vaapi
./your_app
```

### Check Backend Capabilities

```c
if (device->impl->export_to_wl_buffer) {
    wlf_log(WLF_INFO, "Backend supports wl_buffer export");
} else {
    wlf_log(WLF_WARN, "Backend doesn't support wl_buffer export");
}
```

### Verify Zero-Copy

For VA-API, check if `vaGetSurfaceBufferWl` succeeds:
```bash
vainfo  # Check VA-API capabilities
```

For Vulkan, check extensions:
```bash
vulkaninfo | grep -i external
# Should show VK_KHR_external_memory_fd
```

## Limitations

### VA-API
- Linux-only
- Requires VA-API Wayland support in driver
- Limited to compatible GPU vendors (Intel, AMD)

### Vulkan
- Requires VK_KHR_external_memory_fd support
- linux-dmabuf protocol implementation needed
- May not work with older compositors

### Software
- Always requires memory copy
- Higher CPU and memory bandwidth usage
- Not suitable for high-resolution/high-framerate content

## Future Improvements

- [ ] Complete Vulkan linux-dmabuf implementation
- [ ] Buffer pool management for smoother playback
- [ ] Automatic format negotiation with compositor
- [ ] HDR metadata passthrough
- [ ] Multi-plane format support
- [ ] Buffer age tracking for damage optimization

## References

- [VA-API Wayland Documentation](https://github.com/intel/libva)
- [Vulkan External Memory Extensions](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_external_memory.html)
- [Wayland linux-dmabuf Protocol](https://wayland.app/protocols/linux-dmabuf-v1)
- [wl_shm Documentation](https://wayland.app/protocols/wayland#wl_shm)
