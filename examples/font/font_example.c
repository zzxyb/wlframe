/**
 * @file        font_example.c
 * @brief       Example program demonstrating wlf_font library usage.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 */

#include "wlf/font/wlf_font.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("WLF Font Library Example\n");
    printf("========================\n\n");

    // Initialize font subsystem
    if (!wlf_font_init()) {
        fprintf(stderr, "Failed to initialize font subsystem\n");
        return 1;
    }

    // Load a font
    struct wlf_font_options options = WLF_FONT_OPTIONS_DEFAULT;
    options.antialias = true;
    options.hinting = WLF_FONT_HINTING_SLIGHT;

    struct wlf_font *font = wlf_font_load("Monospace:size=16", &options);
    if (!font) {
        fprintf(stderr, "Failed to load font\n");
        wlf_font_cleanup();
        return 1;
    }

    printf("Font loaded successfully:\n");
    printf("  Family: %s\n", font->family);
    printf("  Style: %s\n", font->style);
    printf("  Size: %d pixels\n", font->size);
    printf("  Height: %d pixels\n", font->height);
    printf("  Ascent: %d pixels\n", font->ascent);
    printf("  Descent: %d pixels\n", font->descent);
    printf("\n");

    // Test glyph rasterization
    printf("Testing glyph rasterization:\n");

    const char *test_chars = "Hello, World! 123";
    for (const char *c = test_chars; *c; c++) {
        uint32_t codepoint = (uint32_t)*c;

        if (wlf_font_has_glyph(font, codepoint)) {
            struct wlf_glyph *glyph = wlf_font_rasterize_glyph(font, codepoint);
            if (glyph) {
                printf("  Glyph '%c' (U+%04X): %dx%d pixels, advance (%d,%d)\n",
                       *c, codepoint,
                       glyph->size.width, glyph->size.height,
                       glyph->advance.x, glyph->advance.y);
                wlf_glyph_destroy(glyph);
            }
        }
    }
    printf("\n");

    // Test text metrics
    printf("Testing text metrics:\n");
    const char *test_text = "Hello, World!";
    struct wlf_text_metrics metrics;

    if (wlf_font_get_text_metrics(font, test_text, &metrics)) {
        printf("  Text: \"%s\"\n", test_text);
        printf("  Size: %dx%d pixels\n", metrics.size.width, metrics.size.height);
        printf("  Baseline Y: %d pixels\n", metrics.baseline_y);
        printf("  Advance X: %d pixels\n", metrics.advance_x);
    }
    printf("\n");

    // Test text rasterization
    printf("Testing text rasterization:\n");
    struct wlf_glyph *text_glyph = wlf_font_rasterize_text(font, test_text, 0xFFFFFFFF);
    if (text_glyph) {
        printf("  Text bitmap: %dx%d pixels\n",
               text_glyph->size.width, text_glyph->size.height);
        printf("  Is color: %s\n", text_glyph->is_color ? "yes" : "no");
        wlf_glyph_destroy(text_glyph);
    }
    printf("\n");

    // Test cache functionality
    printf("Testing cache functionality:\n");
    printf("  Initial cache size: %zu glyphs\n", wlf_font_get_cache_size(font));

    // Rasterize some glyphs to populate cache
    for (char c = 'A'; c <= 'Z'; c++) {
        struct wlf_glyph *glyph = wlf_font_rasterize_glyph(font, (uint32_t)c);
        if (glyph) {
            wlf_glyph_destroy(glyph);
        }
    }

    printf("  Cache size after rasterizing A-Z: %zu glyphs\n", wlf_font_get_cache_size(font));

    // Test cache hit
    struct wlf_glyph *cached_glyph = wlf_font_rasterize_glyph(font, 'A');
    if (cached_glyph) {
        printf("  Successfully retrieved cached glyph 'A'\n");
        wlf_glyph_destroy(cached_glyph);
    }

    // Clear cache
    wlf_font_clear_cache(font);
    printf("  Cache size after clearing: %zu glyphs\n", wlf_font_get_cache_size(font));
    printf("\n");

    // Test kerning
    printf("Testing kerning:\n");
    int kerning_x, kerning_y;
    wlf_font_get_kerning(font, 'A', 'V', &kerning_x, &kerning_y);
    printf("  Kerning between 'A' and 'V': (%d, %d)\n", kerning_x, kerning_y);
    printf("\n");

    // Cleanup
    wlf_font_destroy(font);
    wlf_font_cleanup();

    printf("Example completed successfully!\n");
    return 0;
}
