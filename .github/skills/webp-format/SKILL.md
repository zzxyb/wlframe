---
name: webp-format
description: "Use this skill when implementing, integrating, or reviewing WebP read/write C code in wlframe, especially libwebp-based parsing and generation in image, buffer, or related modules. Best for RIFF/WebP container handling, lossy and lossless decode or encode flows, alpha and extended feature handling, and wlframe-style ownership and cleanup review."
---

# WebP Format & libwebp Integration Skill (wlframe/C Style)

## When to Apply
Use this skill when implementing, integrating, or reviewing WebP read/write C code in wlframe, especially in image/, buffer/, or related modules, or when using libwebp for WebP parsing/generation.

## WebP Format Essentials
- WebP is a modern image format supporting lossy and lossless compression, alpha, animation.
- File is RIFF-based, starts with 'RIFF' and 'WEBP' signatures.
- Supports VP8 (lossy), VP8L (lossless), VP8X (extended), alpha, ICC profile, EXIF, XMP.

## libwebp Integration & API Design
- Use official libwebp (https://github.com/webmproject/libwebp).
- Typical workflow:
  1. Use WebPDecode* or WebPEncode* APIs for decoding/encoding.
  2. Set up input/output (file or memory).
  3. Convert pixel data to/from wlframe's internal format as needed.
  4. Release all allocated resources.
- API prefix: `wlf_webp_`. Use `init/create` and `finish/destroy` patterns.
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer.

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
- Check all allocation, I/O, and libwebp API return values.
- Release resources in strict reverse order of allocation.
- Separate headers and implementation; expose only necessary API.
- Reference image/, buffer/ modules for style and error handling.

## References
- https://developers.google.com/speed/webp/docs/riff_container
- https://developers.google.com/speed/webp/docs/api

---
This skill combines wlframe, C, and struct-layout best practices for robust WebP format and libwebp integration.
