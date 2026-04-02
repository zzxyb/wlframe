---
name: jpeg-format
description: "Use this skill when implementing, integrating, or reviewing JPEG read/write C code in wlframe, especially libjpeg-based parsing and generation in image, buffer, or related modules. Best for JPEG marker and colorspace handling, libjpeg lifecycle and error paths, scanline-based I/O, and wlframe-style ownership and cleanup review."
---

# JPEG Format & libjpeg Integration Skill (wlframe/C Style)

## When to Apply
Use this skill when implementing, integrating, or reviewing JPEG read/write C code in wlframe, especially in image/, buffer/, or related modules, or when using libjpeg for JPEG parsing/generation.

## JPEG Format Essentials
- JPEG is a lossy compressed image format, widely used for photographs.
- Uses DCT, quantization, Huffman coding; supports grayscale and color (YCbCr).
- File starts with 0xFFD8 (SOI), ends with 0xFFD9 (EOI), contains various markers (APPn, DQT, DHT, SOF, SOS, etc.).

## libjpeg Integration & API Design
- Use official libjpeg (https://github.com/libjpeg-turbo/libjpeg-turbo).
- Typical workflow:
  1. Create and configure `jpeg_decompress_struct`/`jpeg_compress_struct`.
  2. Set up error handling (setjmp/longjmp or custom error manager).
  3. Bind input/output (file or memory).
  4. Read/write headers.
  5. Read/write scanlines.
  6. Finish and destroy structs.
- Convert pixel data to/from wlframe's internal format as needed.
- API prefix: `wlf_jpeg_`. Use `init/create` and `finish/destroy` patterns.
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer.

## Example API Design
```c
struct wlf_jpeg_image {
	uint32_t width, height;
	uint8_t *pixels;
	int stride;
	int format; // e.g., WLF_PIXEL_FORMAT_RGBA8
	void *impl; // internal state (opaque)
};

bool wlf_jpeg_load(struct wlf_jpeg_image *out, const char *path);
bool wlf_jpeg_save(const struct wlf_jpeg_image *img, const char *path);
void wlf_jpeg_finish(struct wlf_jpeg_image *img);
```

## Best Practices
- Check all allocation, I/O, and libjpeg API return values.
- Release resources in strict reverse order of allocation.
- Separate headers and implementation; expose only necessary API.
- Reference image/, buffer/ modules for style and error handling.

## References
- https://www.w3.org/Graphics/JPEG/itu-t81.pdf
- https://libjpeg-turbo.org/Documentation/Documentation

---
This skill combines wlframe, C, and struct-layout best practices for robust JPEG format and libjpeg integration.
