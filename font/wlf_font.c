#include "wlf/font/wlf_font.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Forward declarations for internal structures
struct glyph_cache_entry {
    uint32_t codepoint;
    struct wlf_glyph *glyph;
    struct glyph_cache_entry *next;
    uint64_t last_used;
};

struct glyph_cache {
    struct glyph_cache_entry **buckets;
    size_t bucket_count;
    size_t entry_count;
    size_t max_entries;
    uint64_t access_counter;
};

// Global state
static bool g_font_initialized = false;
static void *g_ft_library = NULL;  // FT_Library (opaque)

// Hash function for glyph cache
static size_t hash_codepoint(uint32_t codepoint, size_t bucket_count) {
    // Simple hash function
    return (codepoint * 2654435761U) % bucket_count;
}

// Create glyph cache
static struct glyph_cache* glyph_cache_create(size_t max_entries) {
    struct glyph_cache *cache = calloc(1, sizeof(struct glyph_cache));
    if (!cache) {
        return NULL;
    }

    cache->bucket_count = 256;  // Start with 256 buckets
    cache->buckets = calloc(cache->bucket_count, sizeof(struct glyph_cache_entry*));
    if (!cache->buckets) {
        free(cache);
        return NULL;
    }

    cache->max_entries = max_entries > 0 ? max_entries : 1024;  // Default max 1024 glyphs
    cache->access_counter = 0;

    return cache;
}

// Destroy glyph cache
static void glyph_cache_destroy(struct glyph_cache *cache) {
    if (!cache) {
        return;
    }

    for (size_t i = 0; i < cache->bucket_count; i++) {
        struct glyph_cache_entry *entry = cache->buckets[i];
        while (entry) {
            struct glyph_cache_entry *next = entry->next;
            wlf_glyph_destroy(entry->glyph);
            free(entry);
            entry = next;
        }
    }

    free(cache->buckets);
    free(cache);
}

// Find glyph in cache
static struct wlf_glyph* glyph_cache_get(struct glyph_cache *cache, uint32_t codepoint) {
    if (!cache) {
        return NULL;
    }

    size_t bucket = hash_codepoint(codepoint, cache->bucket_count);
    struct glyph_cache_entry *entry = cache->buckets[bucket];

    while (entry) {
        if (entry->codepoint == codepoint) {
            entry->last_used = ++cache->access_counter;
            return entry->glyph;
        }
        entry = entry->next;
    }

    return NULL;
}

// Add glyph to cache
static void glyph_cache_put(struct glyph_cache *cache, uint32_t codepoint, struct wlf_glyph *glyph) {
    if (!cache || !glyph) {
        return;
    }

    // Check if we need to evict old entries
    if (cache->entry_count >= cache->max_entries) {
        // Simple LRU eviction - find oldest entry
        uint64_t oldest_access = UINT64_MAX;
        struct glyph_cache_entry *oldest_entry = NULL;
        size_t oldest_bucket = 0;

        for (size_t i = 0; i < cache->bucket_count; i++) {
            struct glyph_cache_entry *entry = cache->buckets[i];
            while (entry) {
                if (entry->last_used < oldest_access) {
                    oldest_access = entry->last_used;
                    oldest_entry = entry;
                    oldest_bucket = i;
                }
                entry = entry->next;
            }
        }

        if (oldest_entry) {
            // Remove from bucket
            if (cache->buckets[oldest_bucket] == oldest_entry) {
                cache->buckets[oldest_bucket] = oldest_entry->next;
            } else {
                struct glyph_cache_entry *prev = cache->buckets[oldest_bucket];
                while (prev && prev->next != oldest_entry) {
                    prev = prev->next;
                }
                if (prev) {
                    prev->next = oldest_entry->next;
                }
            }

            wlf_glyph_destroy(oldest_entry->glyph);
            free(oldest_entry);
            cache->entry_count--;
        }
    }

    // Add new entry
    size_t bucket = hash_codepoint(codepoint, cache->bucket_count);
    struct glyph_cache_entry *entry = malloc(sizeof(struct glyph_cache_entry));
    if (!entry) {
        return;
    }

    entry->codepoint = codepoint;
    entry->glyph = glyph;
    entry->last_used = ++cache->access_counter;
    entry->next = cache->buckets[bucket];
    cache->buckets[bucket] = entry;
    cache->entry_count++;

    glyph->cached = true;
}

bool wlf_font_init(void) {
    if (g_font_initialized) {
        return true;
    }

    // Initialize FreeType library
    // This would normally be: FT_Init_FreeType(&g_ft_library);
    // For now, we'll simulate it
    g_ft_library = malloc(1);  // Placeholder
    if (!g_ft_library) {
        wlf_log(WLF_ERROR, "Failed to initialize FreeType library");
        return false;
    }

    g_font_initialized = true;
    wlf_log(WLF_INFO, "Font subsystem initialized");
    return true;
}

void wlf_font_cleanup(void) {
    if (!g_font_initialized) {
        return;
    }

    // Cleanup FreeType library
    // This would normally be: FT_Done_FreeType(g_ft_library);
    free(g_ft_library);
    g_ft_library = NULL;

    g_font_initialized = false;
    wlf_log(WLF_INFO, "Font subsystem cleaned up");
}

struct wlf_font* wlf_font_load(const char *pattern, const struct wlf_font_options *options) {
    if (!g_font_initialized) {
        wlf_log(WLF_ERROR, "Font subsystem not initialized");
        return NULL;
    }

    if (!pattern) {
        wlf_log(WLF_ERROR, "Font pattern cannot be NULL");
        return NULL;
    }

    struct wlf_font *font = calloc(1, sizeof(struct wlf_font));
    if (!font) {
        wlf_log(WLF_ERROR, "Failed to allocate font structure");
        return NULL;
    }

    // Copy options or use defaults
    if (options) {
        font->options = *options;
    } else {
        font->options = WLF_FONT_OPTIONS_DEFAULT;
    }

    // Parse pattern (simplified - would normally use FontConfig)
    // For now, just extract basic info
    font->family = strdup("Monospace");  // Default fallback
    font->style = strdup("Regular");
    font->size = 12;  // Default size

    // Create glyph cache
    font->glyph_cache = glyph_cache_create(1024);
    if (!font->glyph_cache) {
        wlf_log(WLF_ERROR, "Failed to create glyph cache");
        wlf_font_destroy(font);
        return NULL;
    }

    // Load FreeType face (placeholder)
    font->ft_face = malloc(1);  // Placeholder
    if (!font->ft_face) {
        wlf_log(WLF_ERROR, "Failed to load font face");
        wlf_font_destroy(font);
        return NULL;
    }

    // Set font metrics (placeholder values)
    font->height = font->size + 4;
    font->ascent = (font->size * 3) / 4;
    font->descent = font->size - font->ascent;
    font->max_advance = wlf_point_make(font->size, 0);
    font->underline.position = -2;
    font->underline.thickness = 1;
    font->strikeout.position = font->size / 3;
    font->strikeout.thickness = 1;
    font->is_loaded = true;

    wlf_log(WLF_INFO, "Font loaded: %s", pattern);
    return font;
}

struct wlf_font* wlf_font_load_from_file(const char *path, int size, const struct wlf_font_options *options) {
    if (!g_font_initialized) {
        wlf_log(WLF_ERROR, "Font subsystem not initialized");
        return NULL;
    }

    if (!path || size <= 0) {
        wlf_log(WLF_ERROR, "Invalid font path or size");
        return NULL;
    }

    // For now, create a pattern string and use the main load function
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "%s:size=%d", path, size);

    return wlf_font_load(pattern, options);
}

void wlf_font_destroy(struct wlf_font *font) {
    if (!font) {
        return;
    }

    // Destroy glyph cache
    if (font->glyph_cache) {
        glyph_cache_destroy((struct glyph_cache*)font->glyph_cache);
    }

    // Free FreeType face
    if (font->ft_face) {
        // This would normally be: FT_Done_Face(font->ft_face);
        free(font->ft_face);
    }

    free(font->family);
    free(font->style);
    free(font);
}

struct wlf_glyph* wlf_font_rasterize_glyph(struct wlf_font *font, uint32_t codepoint) {
    if (!font || !font->is_loaded) {
        return NULL;
    }

    // Check cache first
    struct wlf_glyph *cached_glyph = glyph_cache_get((struct glyph_cache*)font->glyph_cache, codepoint);
    if (cached_glyph) {
        // Return a copy of the cached glyph
        struct wlf_glyph *glyph = malloc(sizeof(struct wlf_glyph));
        if (glyph) {
            *glyph = *cached_glyph;
            glyph->cached = false;  // This is a copy, not the cached version

            // Copy bitmap data
            size_t bitmap_size = glyph->size.width * glyph->size.height * 4;  // RGBA
            glyph->bitmap = malloc(bitmap_size);
            if (glyph->bitmap && cached_glyph->bitmap) {
                memcpy(glyph->bitmap, cached_glyph->bitmap, bitmap_size);
            }
        }
        return glyph;
    }

    // Rasterize new glyph (placeholder implementation)
    struct wlf_glyph *glyph = calloc(1, sizeof(struct wlf_glyph));
    if (!glyph) {
        return NULL;
    }

    glyph->codepoint = codepoint;

    // Placeholder glyph metrics (would normally come from FreeType)
    glyph->size = wlf_size_make(font->size, font->size);
    glyph->bearing = wlf_point_make(0, font->ascent);
    glyph->advance = wlf_point_make(font->size, 0);
    glyph->is_color = false;
    glyph->cached = false;

    // Create placeholder bitmap
    size_t bitmap_size = glyph->size.width * glyph->size.height * 4;  // RGBA
    glyph->bitmap = calloc(1, bitmap_size);
    if (!glyph->bitmap) {
        free(glyph);
        return NULL;
    }

    // Fill with simple pattern (placeholder)
    uint8_t *pixels = glyph->bitmap;
    for (int y = 0; y < glyph->size.height; y++) {
        for (int x = 0; x < glyph->size.width; x++) {
            int idx = (y * glyph->size.width + x) * 4;
            pixels[idx + 0] = 255;  // R
            pixels[idx + 1] = 255;  // G
            pixels[idx + 2] = 255;  // B
            pixels[idx + 3] = (x + y) % 2 ? 255 : 128;  // A (checkerboard pattern)
        }
    }

    // Cache the glyph
    struct wlf_glyph *cache_glyph = malloc(sizeof(struct wlf_glyph));
    if (cache_glyph) {
        *cache_glyph = *glyph;
        cache_glyph->bitmap = malloc(bitmap_size);
        if (cache_glyph->bitmap) {
            memcpy(cache_glyph->bitmap, glyph->bitmap, bitmap_size);
            glyph_cache_put((struct glyph_cache*)font->glyph_cache, codepoint, cache_glyph);
        } else {
            free(cache_glyph);
        }
    }

    return glyph;
}

bool wlf_font_get_text_metrics(struct wlf_font *font, const char *text, struct wlf_text_metrics *metrics) {
    if (!font || !font->is_loaded || !text || !metrics) {
        return false;
    }

    // Simple metrics calculation (placeholder)
    size_t text_len = strlen(text);
    metrics->size = wlf_size_make(text_len * font->size, font->height);
    metrics->baseline_y = font->ascent;
    metrics->advance_x = text_len * font->size;

    return true;
}

struct wlf_glyph* wlf_font_rasterize_text(struct wlf_font *font, const char *text, uint32_t color) {
    if (!font || !font->is_loaded || !text) {
        return NULL;
    }

    // Get text metrics
    struct wlf_text_metrics metrics;
    if (!wlf_font_get_text_metrics(font, text, &metrics)) {
        return NULL;
    }

    // Create text glyph
    struct wlf_glyph *text_glyph = calloc(1, sizeof(struct wlf_glyph));
    if (!text_glyph) {
        return NULL;
    }

    text_glyph->codepoint = 0;  // Special value for text runs
    text_glyph->size = metrics.size;
    text_glyph->bearing = wlf_point_make(0, metrics.baseline_y);
    text_glyph->advance = wlf_point_make(metrics.advance_x, 0);
    text_glyph->is_color = true;
    text_glyph->cached = false;

    // Create bitmap
    size_t bitmap_size = text_glyph->size.width * text_glyph->size.height * 4;
    text_glyph->bitmap = calloc(1, bitmap_size);
    if (!text_glyph->bitmap) {
        free(text_glyph);
        return NULL;
    }

    // Render each character (placeholder implementation)
    int x_offset = 0;
    for (const char *c = text; *c; c++) {
        struct wlf_glyph *glyph = wlf_font_rasterize_glyph(font, (uint32_t)*c);
        if (glyph) {
            // Composite glyph onto text bitmap (simplified)
            // In a real implementation, this would properly blend the glyphs
            wlf_glyph_destroy(glyph);
        }
        x_offset += font->size;
    }

    return text_glyph;
}

void wlf_glyph_destroy(struct wlf_glyph *glyph) {
    if (!glyph) {
        return;
    }

    free(glyph->bitmap);
    free(glyph);
}

void wlf_font_clear_cache(struct wlf_font *font) {
    if (!font || !font->glyph_cache) {
        return;
    }

    struct glyph_cache *cache = (struct glyph_cache*)font->glyph_cache;

    for (size_t i = 0; i < cache->bucket_count; i++) {
        struct glyph_cache_entry *entry = cache->buckets[i];
        while (entry) {
            struct glyph_cache_entry *next = entry->next;
            wlf_glyph_destroy(entry->glyph);
            free(entry);
            entry = next;
        }
        cache->buckets[i] = NULL;
    }

    cache->entry_count = 0;
    cache->access_counter = 0;
}

size_t wlf_font_get_cache_size(struct wlf_font *font) {
    if (!font || !font->glyph_cache) {
        return 0;
    }

    struct glyph_cache *cache = (struct glyph_cache*)font->glyph_cache;
    return cache->entry_count;
}

void wlf_font_set_max_cache_size(struct wlf_font *font, size_t max_size) {
    if (!font || !font->glyph_cache) {
        return;
    }

    struct glyph_cache *cache = (struct glyph_cache*)font->glyph_cache;
    cache->max_entries = max_size > 0 ? max_size : 1024;
}

bool wlf_font_has_glyph(struct wlf_font *font, uint32_t codepoint) {
    if (!font || !font->is_loaded) {
        return false;
    }

    // Placeholder implementation - assume all printable ASCII characters are supported
    return codepoint >= 32 && codepoint <= 126;
}

struct wlf_point wlf_font_get_kerning(struct wlf_font *font, uint32_t left_codepoint, uint32_t right_codepoint) {
    if (!font || !font->is_loaded) {
        return wlf_point_make(0, 0);
    }

    // Placeholder implementation - no kerning
    (void)left_codepoint;
    (void)right_codepoint;
    return wlf_point_make(0, 0);
}
