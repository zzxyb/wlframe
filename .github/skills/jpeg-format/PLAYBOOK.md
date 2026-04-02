---
name: jpeg-format
summary: "Skill for implementing, integrating, and reviewing JPEG format and libjpeg usage in wlframe C code."
description: |
  Use this skill when writing, refactoring, or reviewing C code that reads, writes, or processes JPEG images using libjpeg, especially in wlframe's image, buffer, or related modules. This skill enforces wlframe's C style, error handling, struct layout, and API design best practices for JPEG integration.
---

# JPEG Format & libjpeg Integration Skill (wlframe/C Style)

## When to Apply
- Implementing or reviewing JPEG loading/saving in wlframe (e.g., image/, buffer/)
- Designing C APIs or structs for JPEG data
- Integrating libjpeg with wlframe's build, error, and memory management

## Key Principles
- Use plain C, avoid macro-heavy or typedef-heavy abstractions
- All allocations and I/O must be checked for failure; log errors with `wlf_log_errno()`
- Use libjpeg error handling (setjmp/longjmp or custom error manager)
- API naming: use `wlf_jpeg_` prefix, follow `init/create` and `finish/destroy` patterns
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer
- Public headers expose only necessary structs and function declarations
- Follow wlframe's K&R style, tab indentation, and module boundaries

## JPEG Format Essentials
- JPEG is a lossy compressed image format, widely used for photographs
- Uses DCT, quantization, Huffman coding; supports grayscale and color (YCbCr)
- File starts with 0xFFD8 (SOI), ends with 0xFFD9 (EOI), contains various markers (APPn, DQT, DHT, SOF, SOS, etc.)

## libjpeg Integration
- Use official libjpeg (https://github.com/libjpeg-turbo/libjpeg-turbo)
- Typical workflow:
  1. Create and configure `jpeg_decompress_struct`/`jpeg_compress_struct`
  2. Set up error handling (setjmp/longjmp or custom error manager)
  3. Bind input/output (file or memory)
  4. Read/write headers
  5. Read/write scanlines
  6. Finish and destroy structs
- Convert pixel data to/from wlframe's internal format as needed

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
- Check all allocation, I/O, and libjpeg API return values
- Release resources in strict reverse order of allocation
- Separate headers and implementation; expose only necessary API
- Reference image/, buffer/ modules for style and error handling

## References
- [JPEG File Interchange Format](https://www.w3.org/Graphics/JPEG/itu-t81.pdf)
- [libjpeg-turbo Manual](https://libjpeg-turbo.org/Documentation/Documentation)

---
This skill combines wlframe, C, and struct-layout best practices for robust JPEG format and libjpeg integration.
