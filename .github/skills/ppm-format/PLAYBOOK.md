---
name: ppm-format
summary: "Skill for implementing, integrating, and reviewing PPM format usage in wlframe C code."
description: |
  Use this skill when writing, refactoring, or reviewing C code that reads, writes, or processes PPM images, especially in wlframe's image, buffer, or related modules. This skill enforces wlframe's C style, error handling, struct layout, and API design best practices for PPM integration.
---

# PPM Format Integration Skill (wlframe/C Style)

## When to Apply
- Implementing or reviewing PPM loading/saving in wlframe (e.g., image/, buffer/)
- Designing C APIs or structs for PPM data
- Integrating PPM support with wlframe's build, error, and memory management

## Key Principles
- Use plain C, avoid macro-heavy or typedef-heavy abstractions
- All allocations and I/O must be checked for failure; log errors with `wlf_log_errno()`
- API naming: use `wlf_ppm_` prefix, follow `init/create` and `finish/destroy` patterns
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer
- Public headers expose only necessary structs and function declarations
- Follow wlframe's K&R style, tab indentation, and module boundaries

## PPM Format Essentials
- PPM (Portable Pixmap) is a simple, uncompressed ASCII (P3) or binary (P6) image format
- Header: magic number (P3 or P6), width, height, max color value, then pixel data
- Pixel data: RGB triplets, row-major order

## Integration
- Parse header strictly; validate magic number, dimensions, max value
- Support both ASCII and binary variants
- Convert pixel data to wlframe's internal format as needed

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
- Check all allocation, I/O, and header parsing for errors
- Release resources in strict reverse order of allocation
- Separate headers and implementation; expose only necessary API
- Reference image/, buffer/ modules for style and error handling

## References
- [Netpbm Format Spec](https://netpbm.sourceforge.net/doc/ppm.html)

---
This skill combines wlframe, C, and struct-layout best practices for robust PPM format integration.
