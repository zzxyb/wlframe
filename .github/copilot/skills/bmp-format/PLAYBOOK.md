---
name: bmp-format
summary: "Skill for implementing, integrating, and reviewing BMP format usage in wlframe C code."
description: |
  Use this skill when writing, refactoring, or reviewing C code that reads, writes, or processes BMP images, especially in wlframe's image, buffer, or related modules. This skill enforces wlframe's C style, error handling, struct layout, and API design best practices for BMP integration.
---

# BMP Format Integration Skill (wlframe/C Style)

## When to Apply
- Implementing or reviewing BMP loading/saving in wlframe (e.g., image/, buffer/)
- Designing C APIs or structs for BMP data
- Integrating BMP support with wlframe's build, error, and memory management

## Key Principles
- Use plain C, avoid macro-heavy or typedef-heavy abstractions
- All allocations and I/O must be checked for failure; log errors with `wlf_log_errno()`
- API naming: use `wlf_bmp_` prefix, follow `init/create` and `finish/destroy` patterns
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer
- Public headers expose only necessary structs and function declarations
- Follow wlframe's K&R style, tab indentation, and module boundaries

## BMP Format Essentials
- BMP (Bitmap) is a simple, uncompressed or RLE-compressed raster image format
- File starts with 'BM' signature, followed by file header, DIB header, pixel data
- Supports various bit depths (1, 4, 8, 16, 24, 32 bpp), optional color palette
- Rows are aligned to 4-byte boundaries (padding may be present)

## Integration
- Parse headers carefully; validate all fields (size, offset, bit depth, compression)
- Support bottom-up (default) and top-down (negative height) row order
- Handle palette for indexed color images
- Convert pixel data to wlframe's internal format as needed

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
- Check all allocation, I/O, and header parsing for errors
- Release resources in strict reverse order of allocation
- Separate headers and implementation; expose only necessary API
- Reference image/, buffer/ modules for style and error handling

## References
- [BMP File Format Spec](https://en.wikipedia.org/wiki/BMP_file_format)

---
This skill combines wlframe, C, and struct-layout best practices for robust BMP format integration.
