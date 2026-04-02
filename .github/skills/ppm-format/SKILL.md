---
name: ppm-format
description: "Use this skill when implementing, integrating, or reviewing PPM read/write C code in wlframe, especially portable pixmap parsing and generation in image, buffer, or related modules. Best for P3/P6 header parsing, max-value validation, ASCII and binary pixel decoding, and wlframe-style ownership and cleanup review."
---

# PPM Format Integration Skill (wlframe/C Style)

## When to Apply
Use this skill when implementing, integrating, or reviewing PPM read/write C code in wlframe, especially in image/, buffer/, or related modules.

## PPM Format Essentials
- PPM (Portable Pixmap) is a simple, uncompressed ASCII (P3) or binary (P6) image format.
- Header: magic number (P3 or P6), width, height, max color value, then pixel data.
- Pixel data: RGB triplets, row-major order.

## Integration & API Design
- Parse header strictly; validate magic number, dimensions, max value.
- Support both ASCII and binary variants.
- Convert pixel data to wlframe's internal format as needed.
- API prefix: `wlf_ppm_`. Use `init/create` and `finish/destroy` patterns.
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer.

## Example API Design
```c
struct wlf_ppm_image {
	uint32_t width, height;
	uint8_t *pixels;
	int stride;
	int format; // e.g., WLF_PIXEL_FORMAT_RGBA8
	void *impl; // internal state (opaque)
};

bool wlf_ppm_load(struct wlf_ppm_image *out, const char *path);
bool wlf_ppm_save(const struct wlf_ppm_image *img, const char *path);
void wlf_ppm_finish(struct wlf_ppm_image *img);
```

## Best Practices
- Check all allocation, I/O, and header parsing for errors.
- Release resources in strict reverse order of allocation.
- Separate headers and implementation; expose only necessary API.
- Reference image/, buffer/ modules for style and error handling.

## References
- https://netpbm.sourceforge.net/doc/ppm.html

---
This skill combines wlframe, C, and struct-layout best practices for robust PPM format integration.
