/**
 * @file        font_backend_example.c
 * @brief       Example demonstrating font backend system usage.
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 */

#include "wlf/font/wlf_font.h"
#include "wlf/font/wlf_font_backend.h"
#include "wlf/utils/wlf_log.h"
#include <stdio.h>
#include <stdlib.h>

// Callback function for font enumeration
static bool font_info_callback(const struct wlf_font_info *info, void *user_data) {
    int *count = (int*)user_data;
    (*count)++;

    printf("Font #%d:\n", *count);
    printf("  Family: %s\n", info->family_name ? info->family_name : "Unknown");
    printf("  Style: %s\n", info->style_name ? info->style_name : "Unknown");
    printf("  PostScript: %s\n", info->postscript_name ? info->postscript_name : "Unknown");
    printf("  File: %s\n", info->file_path ? info->file_path : "Unknown");
    printf("  Weight: %d\n", info->weight);
    printf("  Style: %d\n", info->style);
    printf("  Monospace: %s\n", info->is_monospace ? "Yes" : "No");
    printf("  Scalable: %s\n", info->is_scalable ? "Yes" : "No");

    if (info->languages) {
        printf("  Languages: ");
        for (char **lang = info->languages; *lang; lang++) {
            printf("%s ", *lang);
        }
        printf("\n");
    }

    printf("\n");

    // Stop after 10 fonts to avoid too much output
    return *count < 10;
}

int main(void) {
    printf("Font Backend System Example\n");
    printf("===========================\n\n");

    // Initialize font system
    if (!wlf_font_init()) {
        fprintf(stderr, "Failed to initialize font system\n");
        return 1;
    }

    // Get active backend info
    const struct wlf_font_backend *backend = wlf_font_backend_get_active();
    if (backend) {
        printf("Active backend: %s (%s)\n\n", backend->name, backend->description);
    } else {
        printf("No active backend available\n\n");
    }

    // List all available backends
    size_t backend_count;
    const struct wlf_font_backend **backends = wlf_font_backend_get_all(&backend_count);
    printf("Available backends (%zu):\n", backend_count);
    for (size_t i = 0; i < backend_count; i++) {
        printf("  %zu. %s - %s\n", i + 1, backends[i]->name, backends[i]->description);
    }
    printf("\n");

    if (!backend) {
        printf("Cannot demonstrate font operations without an active backend\n");
        wlf_font_cleanup();
        return 1;
    }

    // Enumerate first 10 system fonts
    printf("System Fonts (first 10):\n");
    printf("========================\n");
    int font_count = 0;
    if (!wlf_font_enumerate_system_fonts(font_info_callback, &font_count)) {
        printf("Failed to enumerate system fonts\n");
    }
    printf("Total fonts enumerated: %d\n\n", font_count);

    // Try to find specific fonts
    printf("Searching for monospace fonts:\n");
    printf("==============================\n");
    font_count = 0;
    if (!wlf_font_find_system_fonts("monospace", font_info_callback, &font_count)) {
        printf("Failed to search for monospace fonts\n");
    }
    printf("Monospace fonts found: %d\n\n", font_count);

    // Get system font paths
    printf("System Font Paths:\n");
    printf("==================\n");

    char *default_font = wlf_font_get_system_default_font(NULL);
    printf("Default font: %s\n", default_font ? default_font : "Not found");
    free(default_font);

    char *mono_font = wlf_font_get_system_monospace_font();
    printf("Monospace font: %s\n", mono_font ? mono_font : "Not found");
    free(mono_font);

    char *serif_font = wlf_font_get_system_font_path("serif", WLF_FONT_STYLE_NORMAL, WLF_FONT_WEIGHT_NORMAL);
    printf("Serif font: %s\n", serif_font ? serif_font : "Not found");
    free(serif_font);

    char *sans_font = wlf_font_get_system_font_path("sans-serif", WLF_FONT_STYLE_NORMAL, WLF_FONT_WEIGHT_NORMAL);
    printf("Sans-serif font: %s\n", sans_font ? sans_font : "Not found");
    free(sans_font);

    printf("\n");

    // Try to load system fonts
    printf("Loading System Fonts:\n");
    printf("=====================\n");

    struct wlf_font *default_font_obj = wlf_font_load_system_default(NULL, 14, NULL);
    if (default_font_obj) {
        printf("Successfully loaded default font: %s\n", default_font_obj->family ? default_font_obj->family : "Unknown");
        wlf_font_destroy(default_font_obj);
    } else {
        printf("Failed to load default font\n");
    }

    struct wlf_font *mono_font_obj = wlf_font_load_system_monospace(12, NULL);
    if (mono_font_obj) {
        printf("Successfully loaded monospace font: %s\n", mono_font_obj->family ? mono_font_obj->family : "Unknown");
        wlf_font_destroy(mono_font_obj);
    } else {
        printf("Failed to load monospace font\n");
    }

    struct wlf_font *serif_font_obj = wlf_font_load_system_font("serif",
                                                               WLF_FONT_STYLE_NORMAL,
                                                               WLF_FONT_WEIGHT_NORMAL,
                                                               16, NULL);
    if (serif_font_obj) {
        printf("Successfully loaded serif font: %s\n", serif_font_obj->family ? serif_font_obj->family : "Unknown");
        wlf_font_destroy(serif_font_obj);
    } else {
        printf("Failed to load serif font\n");
    }

    printf("\n");

    // Cleanup
    wlf_font_cleanup();
    printf("Font system cleaned up\n");

    return 0;
}
