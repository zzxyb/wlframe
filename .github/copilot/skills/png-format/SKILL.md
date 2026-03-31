---
name: png-format
description: "Use this skill when implementing, integrating, or reviewing PNG read/write C code in wlframe, especially libpng-based parsing and generation in image, buffer, or related modules. Best for PNG format handling, libpng lifecycle and error paths, row-based I/O, and wlframe-style memory ownership review."
---

# PNG Format & libpng Integration Skill (wlframe/C Style)

## When to Apply
Use this skill when implementing, integrating, or reviewing PNG read/write C code in wlframe, especially in image/, buffer/, or related modules, or when using libpng for PNG parsing/generation.

## PNG Format Essentials
- PNG (Portable Network Graphics) is a lossless, chunk-based bitmap format (signature: `89 50 4E 47 0D 0A 1A 0A`).
- Key chunks: IHDR (header), PLTE (palette), IDAT (data), IEND (end), each with CRC.
- Supports multiple color depths, alpha, palette, grayscale, and color.

## libpng Integration & API Design
- Depends on zlib. Official repo: https://github.com/pnggroup/libpng
- Integrate in plain C style, avoid macro/typedef-heavy abstractions. API prefix: `wlf_png_`.
- Typical workflow:
  1. Create `png_structp` (`png_create_read_struct`/`png_create_write_struct`)
  2. Create `png_infop` (`png_create_info_struct`)
  3. Set error handling (`setjmp`, mandatory!)
  4. Bind I/O (`png_init_io`, custom streams supported)
  5. Read/write header (`png_read_info`/`png_write_info`)
  6. Read/write image data row by row (`png_read_row`/`png_write_row`, recommended for large images)
  7. Destroy structs (`png_destroy_read_struct`/`png_destroy_write_struct`)
- Use libpng's color conversion APIs (e.g., `png_set_expand_gray_1_2_4_to_8`).
- Distinguish between premultiplied and non-premultiplied alpha.

## wlframe Style: Memory & Error Handling
- Check all allocations immediately; on failure, log with `wlf_log_errno()` and clean up any allocated resources.
- Use setjmp for error protection; all allocation and I/O failures must have a clear unwind path.
- Constructor/destructor naming: `init/create`, `finish/destroy`. Headers expose only necessary API.
- Struct design follows struct-layout skill:
  - Hot-path fields first, group pointers/handles/flags, avoid meaningless padding.
  - Public structs expose only necessary fields; private implementation details go in C file or `impl` pointer.

## Example API Design
```c
struct wlf_png_image {
	// Hot-path fields first
	uint32_t width, height;
	uint8_t *pixels;
	int stride;
	int format; // e.g., WLF_PIXEL_FORMAT_RGBA8
	// ...other metadata...

	// Private implementation details
	void *impl; // points to libpng state
};

bool wlf_png_load(struct wlf_png_image *out, const char *path);
bool wlf_png_save(const struct wlf_png_image *img, const char *path);
void wlf_png_finish(struct wlf_png_image *img);
```

## Best Practices
- Use only standard chunks; check support for any extension chunks.
- Check all allocation, I/O, and libpng API return values.
- Release resources in strict reverse order of allocation to prevent leaks.
- Separate headers and implementation; public API exposes only struct and function declarations.
- Reference wlframe image/, pixman/, buffer/ modules for API style and error handling.

## References
- [PNG Spec RFC 2083](https://datatracker.ietf.org/doc/html/rfc2083)
- [libpng Manual](http://www.libpng.org/pub/png/libpng-manual.txt)
- [libpng Source & Examples](https://github.com/pnggroup/libpng)

---
This skill combines wlframe, C, and struct-layout best practices for robust PNG format and libpng integration.
