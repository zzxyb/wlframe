# WLF Font Library

A simple and efficient font loading and glyph rasterization library for wlframe, inspired by the [fcft library](https://codeberg.org/dnkl/fcft).

## Design Philosophy

The WLF Font library follows fcft's core principles:

- **Speed**: Optimized for fast glyph rasterization with aggressive caching
- **Simplicity**: Clean, minimal API focused on essential functionality
- **Thread-safety**: Designed to support multi-threaded rendering (future enhancement)
- **Memory efficiency**: Smart caching with LRU eviction

## Features

### Core Features
- Font loading from FontConfig patterns or file paths
- Glyph rasterization with RGBA bitmap output
- Aggressive glyph caching with LRU eviction
- Text metrics calculation
- Text string rasterization
- Multiple antialiasing modes
- Subpixel rendering support
- Font hinting control

### Rendering Options
- **Antialiasing**: None, grayscale, or subpixel
- **Subpixel layouts**: RGB/BGR horizontal/vertical
- **Hinting**: None, slight, medium, or full
- **Filtering**: None, bilinear, or Lanczos3
- **Font effects**: Embolden, oblique transformation

### Cache Management
- Configurable cache size limits
- LRU-based eviction policy
- Cache statistics and control
- Per-font cache isolation

## API Overview

### Initialization
```c
// Initialize the font subsystem
bool wlf_font_init(void);

// Cleanup when done
void wlf_font_cleanup(void);
```

### Font Loading
```c
// Load font from FontConfig pattern
struct wlf_font *font = wlf_font_load("Monospace:size=12", NULL);

// Load font from file
struct wlf_font *font = wlf_font_load_from_file("/path/to/font.ttf", 12, NULL);

// Destroy font when done
wlf_font_destroy(font);
```

### Glyph Rasterization
```c
// Rasterize single glyph
struct wlf_glyph *glyph = wlf_font_rasterize_glyph(font, 'A');

// Rasterize text string
struct wlf_glyph *text = wlf_font_rasterize_text(font, "Hello", 0xFFFFFFFF);

// Get text metrics without rasterizing
struct wlf_text_metrics metrics;
wlf_font_get_text_metrics(font, "Hello", &metrics);

// Cleanup glyphs
wlf_glyph_destroy(glyph);
```

### Cache Management
```c
// Get cache statistics
size_t cached_glyphs = wlf_font_get_cache_size(font);

// Set cache limits
wlf_font_set_max_cache_size(font, 2048);

// Clear cache
wlf_font_clear_cache(font);
```

## Configuration Options

```c
struct wlf_font_options options = {
    .subpixel = WLF_FONT_SUBPIXEL_HORIZONTAL_RGB,
    .filter = WLF_FONT_FILTER_BILINEAR,
    .hinting = WLF_FONT_HINTING_SLIGHT,
    .antialias = true,
    .autohint = false,
    .embolden = false,
    .oblique = 0.0,
    .dpi = 96.0,
};

struct wlf_font *font = wlf_font_load("Arial:size=14", &options);
```

## Data Structures

### Font Instance
```c
struct wlf_font {
    char *family;                    // Font family name
    char *style;                     // Font style
    int size;                        // Font size in pixels
    int height;                      // Line height
    int ascent, descent;             // Baseline metrics
    struct {
        int x, y;                    // Maximum advance coordinates
    } max_advance;                   // Maximum advance
    // ... underline, strikeout, options, etc.
};
```

### Glyph Data
```c
struct wlf_glyph {
    uint32_t codepoint;              // Unicode codepoint
    struct {
        int width, height;           // Bitmap dimensions
    } size;                          // Bitmap size
    struct {
        int x, y;                    // Glyph bearing coordinates
    } bearing;                       // Glyph bearing
    struct {
        int x, y;                    // Advance coordinates
    } advance;                       // Advance to next glyph
    uint8_t *bitmap;                 // RGBA bitmap data
    bool is_color;                   // Color glyph flag
    bool cached;                     // Cache status
};
```

## Performance Characteristics

### Cache Performance
- **Hash table**: O(1) average lookup time
- **LRU eviction**: O(n) worst case for eviction (where n = cache size)
- **Memory usage**: ~4 bytes per pixel + metadata per cached glyph

### Typical Cache Sizes
- **Small applications**: 256-512 glyphs
- **Text editors**: 1024-2048 glyphs
- **Large applications**: 4096+ glyphs

### Memory Usage Example
For a 16px font with 1024 cached glyphs:
- Average glyph size: ~16x16 pixels = 1KB bitmap
- Total cache memory: ~1MB + metadata

## Comparison with fcft

### Similarities (Borrowed Features)
- ✅ Aggressive glyph caching with LRU eviction
- ✅ FontConfig integration for font loading
- ✅ Multiple antialiasing and subpixel modes
- ✅ Simple, focused API design
- ✅ Optimized for speed over features

### Differences (Simplified)
- ❌ No HarfBuzz text shaping (simplified text handling)
- ❌ No complex script support (focus on Latin scripts)
- ❌ No pixman integration (direct RGBA output)
- ❌ No SVG glyph support (bitmap fonts only)
- ❌ No multi-threading (single-threaded for now)

### Advantages of Simplification
- **Smaller footprint**: Fewer dependencies
- **Easier integration**: Simpler build requirements
- **Better understanding**: Clearer code for learning
- **Faster compilation**: Less complex build process

## Future Enhancements

### Planned Features
1. **Real FreeType integration**: Replace placeholder implementation
2. **FontConfig support**: Proper pattern parsing and font matching
3. **Thread safety**: Mutex protection for cache operations
4. **Better text shaping**: Basic ligature and kerning support
5. **Color font support**: Emoji and color bitmap fonts
6. **Subpixel positioning**: More precise glyph placement

### Possible Extensions
- **Font fallback chains**: Automatic fallback font selection
- **Font variations**: Variable font support
- **Advanced caching**: Multi-level cache hierarchy
- **GPU acceleration**: OpenGL/Vulkan glyph upload helpers

## Dependencies

### Current (Placeholder)
- Standard C library
- Basic C integer types for coordinates and dimensions
- wlframe logging utilities

### Future (Real Implementation)
- **FreeType2**: Font loading and glyph rasterization
- **FontConfig**: Font discovery and matching
- **HarfBuzz** (optional): Advanced text shaping
- **Pixman** (optional): High-quality image operations

## Building

```bash
# As part of wlframe build
meson build
ninja -C build

# Run example
./build/examples/font_example
```

## License

Same as wlframe project license.

## Acknowledgments

- **fcft library**: Primary inspiration for design and architecture
- **FreeType project**: Font rasterization technology
- **FontConfig**: Font configuration and matching
- **HarfBuzz**: Text shaping algorithms
