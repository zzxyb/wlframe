# PNG Format & libpng Integration Skill (wlframe/C Style)

## When to Apply
- Implementing or reviewing PNG loading/saving in wlframe (e.g., image/, buffer/)
- Designing C APIs or structs for PNG data
- Integrating libpng with wlframe's build, error, and memory management

## Key Principles
- Use plain C, avoid macro-heavy or typedef-heavy abstractions
- All allocations and I/O must be checked for failure; log errors with `wlf_log_errno()`
- Use `setjmp` for libpng error handling; always unwind resources on error
- API naming: use `wlf_png_` prefix, follow `init/create` and `finish/destroy` patterns
- Struct layout: hot fields first, group pointers/handles/flags, hide implementation details via `impl` pointer
- Public headers expose only necessary structs and function declarations
- Follow wlframe's K&R style, tab indentation, and module boundaries

## PNG Format Essentials
- PNG is a lossless, chunk-based bitmap format (signature: `89 50 4E 47 0D 0A 1A 0A`)
- Key chunks: IHDR (header), PLTE (palette), IDAT (data), IEND (end), each with CRC
- Supports multiple color depths, alpha, palette, grayscale, color

## libpng Integration
- Use official libpng (https://github.com/pnggroup/libpng), requires zlib
- Typical workflow:
  1. Create `png_structp` (`png_create_read_struct`/`png_create_write_struct`)
  2. Create `png_infop` (`png_create_info_struct`)
  3. Set error handling (`setjmp`)
  4. Bind I/O (`png_init_io` or custom stream)
  5. Read/write header (`png_read_info`/`png_write_info`)
  6. Read/write image data row by row (`png_read_row`/`png_write_row`)
  7. Destroy structs (`png_destroy_read_struct`/`png_destroy_write_struct`)
- Use libpng's color conversion APIs (e.g., `png_set_expand_gray_1_2_4_to_8`)
- Distinguish between premultiplied and non-premultiplied alpha

## Example API Design
```c
struct wlf_png_image {
	uint32_t width, height;
	uint8_t *pixels;
	int stride;
	int format; // e.g., WLF_PIXEL_FORMAT_RGBA8
	void *impl; // libpng state (opaque)
};

bool wlf_png_load(struct wlf_png_image *out, const char *path);
bool wlf_png_save(const struct wlf_png_image *img, const char *path);
void wlf_png_finish(struct wlf_png_image *img);
```

## Best Practices
- Only use standard chunks; check support for any extensions
- Check all allocation, I/O, and libpng API return values
- Release resources in strict reverse order of allocation
- Separate headers and implementation; expose only necessary API
- Follow wlframe's error handling and struct layout conventions
- Reference image/, pixman/, buffer/ modules for style and error handling

## References
- [PNG Spec RFC 2083](https://datatracker.ietf.org/doc/html/rfc2083)
- [libpng Manual](http://www.libpng.org/pub/png/libpng-manual.txt)
- [libpng Source & Examples](https://github.com/pnggroup/libpng)

---
This skill combines wlframe, C, and struct-layout best practices for robust PNG format and libpng integration.
