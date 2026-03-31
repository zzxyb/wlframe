---
name: bmp-format
description: "Use this skill when implementing, integrating, or reviewing BMP read/write C code in wlframe, especially bitmap parsing and generation in image, buffer, or related modules. Best for BMP header validation, row padding and orientation handling, palette and bit-depth parsing, and wlframe-style ownership and cleanup review."
---

# BMP Format Integration Skill (wlframe/C Style)

## When to Apply
Use this skill when implementing, integrating, or reviewing BMP read/write C code in wlframe, especially in image/, buffer/, or related modules.

## BMP Format Essentials
- BMP (Bitmap) is a simple, uncompressed or RLE-compressed raster image format.
- File starts with 'BM' signature, followed by file header, DIB header, pixel data.
- Supports various bit depths (1, 4, 8, 16, 24, 32 bpp), optional color palette.
- Rows are aligned to 4-byte boundaries (may include padding).

## Integration & API Design
- Parse headers carefully; validate all fields (size, offset, bit depth, compression).
- Support bottom-up (default) and top-down (negative height) row order.
- Handle palette for indexed color images.
- Convert pixel data to wlframe's internal format as needed.
- API prefix: `wlf_bmp_`. Use `init/create` and `finish/destroy` patterns.
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer.

## Example API Design
```c
struct wlf_bmp_image {
	uint32_t width, height;
	uint8_t *pixels;
	int stride;
	int format; // e.g., WLF_PIXEL_FORMAT_RGBA8
	void *impl; // internal state (opaque)
};

bool wlf_bmp_load(struct wlf_bmp_image *out, const char *path);
bool wlf_bmp_save(const struct wlf_bmp_image *img, const char *path);
void wlf_bmp_finish(struct wlf_bmp_image *img);
```

## Best Practices
- Check all allocation, I/O, and header parsing for errors.
- Release resources in strict reverse order of allocation.
- Separate headers and implementation; expose only necessary API.
- Reference image/, buffer/ modules for style and error handling.

## References
- https://en.wikipedia.org/wiki/BMP_file_format

---
This skill combines wlframe, C, and struct-layout best practices for robust BMP format integration.
