---
name: webp-format
summary: "Skill for implementing, integrating, and reviewing WebP format and libwebp usage in wlframe C code."
description: |
  Use this skill when writing, refactoring, or reviewing C code that reads, writes, or processes WebP images using libwebp, especially in wlframe's image, buffer, or related modules. This skill enforces wlframe's C style, error handling, struct layout, and API design best practices for WebP integration.
---

# WebP Format & libwebp Integration Skill (wlframe/C Style)

## When to Apply
- Implementing or reviewing WebP loading/saving in wlframe (e.g., image/, buffer/)
- Designing C APIs or structs for WebP data
- Integrating libwebp with wlframe's build, error, and memory management

## Key Principles
- Use plain C, avoid macro-heavy or typedef-heavy abstractions
- All allocations and I/O must be checked for failure; log errors with `wlf_log_errno()`
- API naming: use `wlf_webp_` prefix, follow `init/create` and `finish/destroy` patterns
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer
- Public headers expose only necessary structs and function declarations
- Follow wlframe's K&R style, tab indentation, and module boundaries

## WebP Format Essentials
- WebP is a modern image format supporting lossy and lossless compression, alpha, animation
- File is RIFF-based, starts with 'RIFF' and 'WEBP' signatures
- Supports VP8 (lossy), VP8L (lossless), VP8X (extended), alpha, ICC profile, EXIF, XMP

## libwebp Integration
- Use official libwebp (https://github.com/webmproject/libwebp)
- Typical workflow:
  1. Use WebPDecode* or WebPEncode* APIs for decoding/encoding
  2. Set up input/output (file or memory)
  3. Convert pixel data to/from wlframe's internal format as needed
  4. Release all allocated resources

## Example API Design
```c
struct wlf_webp_image {
	uint32_t width, height;
	uint8_t *pixels;
	int stride;
	int format; // e.g., WLF_PIXEL_FORMAT_RGBA8
	void *impl; // internal state (opaque)
};

bool wlf_webp_load(struct wlf_webp_image *out, const char *path);
bool wlf_webp_save(const struct wlf_webp_image *img, const char *path);
void wlf_webp_finish(struct wlf_webp_image *img);
```

## Best Practices
- Check all allocation, I/O, and libwebp API return values
- Release resources in strict reverse order of allocation
- Separate headers and implementation; expose only necessary API
- Reference image/, buffer/ modules for style and error handling

## References
- [WebP Container Spec](https://developers.google.com/speed/webp/docs/riff_container)
- [libwebp Manual](https://developers.google.com/speed/webp/docs/api)

---
This skill combines wlframe, C, and struct-layout best practices for robust WebP format and libwebp integration.
