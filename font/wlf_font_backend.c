/**
 * @file        wlf_font_backend.c
 * @brief       Font backend plugin manager implementation.
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 */

#include "wlf/font/wlf_font_backend.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_BACKENDS 16

// Forward declarations for platform backends
extern const struct wlf_font_backend wlf_font_backend_macos;
extern const struct wlf_font_backend wlf_font_backend_fontconfig;

// Global state
static bool g_backend_initialized = false;
static const struct wlf_font_backend *g_registered_backends[MAX_BACKENDS];
static size_t g_backend_count = 0;
static const struct wlf_font_backend *g_active_backend = NULL;

bool wlf_font_backend_init(void) {
	if (g_backend_initialized) {
		return true;
	}

	// Clear registered backends
	memset(g_registered_backends, 0, sizeof(g_registered_backends));
	g_backend_count = 0;
	g_active_backend = NULL;

	// Register platform-specific backends
#ifdef __APPLE__
	if (wlf_font_backend_register(&wlf_font_backend_macos)) {
		wlf_log(WLF_INFO, "Registered macOS font backend");
	}
#endif

#ifdef __linux__
	if (wlf_font_backend_register(&wlf_font_backend_fontconfig)) {
		wlf_log(WLF_INFO, "Registered FontConfig font backend");
	}
#endif

	// Find and activate the first available backend
	for (size_t i = 0; i < g_backend_count; i++) {
		const struct wlf_font_backend *backend = g_registered_backends[i];
		if (backend->is_available && backend->is_available()) {
			if (backend->init && backend->init()) {
				g_active_backend = backend;
				wlf_log(WLF_INFO, "Activated font backend: %s", backend->name);
				break;
			}
		}
	}

	if (!g_active_backend) {
		wlf_log(WLF_INFO, "No font backend available");
	}

	g_backend_initialized = true;
	return true;
}

void wlf_font_backend_cleanup(void) {
	if (!g_backend_initialized) {
		return;
	}

	// Cleanup active backend
	if (g_active_backend && g_active_backend->cleanup) {
		g_active_backend->cleanup();
	}

	g_active_backend = NULL;
	g_backend_count = 0;
	g_backend_initialized = false;

	wlf_log(WLF_INFO, "Font backend system cleaned up");
}

bool wlf_font_backend_register(const struct wlf_font_backend *backend) {
	if (!backend) {
		wlf_log(WLF_ERROR, "Cannot register NULL backend");
		return false;
	}

	if (g_backend_count >= MAX_BACKENDS) {
		wlf_log(WLF_ERROR, "Maximum number of backends reached");
		return false;
	}

	// Check for duplicate registration
	for (size_t i = 0; i < g_backend_count; i++) {
		if (g_registered_backends[i] == backend ||
		    (backend->name && g_registered_backends[i]->name &&
		     strcmp(backend->name, g_registered_backends[i]->name) == 0)) {
			wlf_log(WLF_INFO, "Backend '%s' already registered", backend->name);
			return false;
		}
	}

	g_registered_backends[g_backend_count++] = backend;
	return true;
}

const struct wlf_font_backend* wlf_font_backend_get_active(void) {
	return g_active_backend;
}

const struct wlf_font_backend** wlf_font_backend_get_all(size_t *count) {
	if (count) {
		*count = g_backend_count;
	}
	return g_registered_backends;
}

void wlf_font_info_free(struct wlf_font_info *info) {
	if (!info) {
		return;
	}

	free(info->family_name);
	free(info->style_name);
	free(info->postscript_name);
	free(info->file_path);

	if (info->languages) {
		for (char **lang = info->languages; *lang; lang++) {
			free(*lang);
		}
		free(info->languages);
	}

	if (info->character_sets) {
		for (char **charset = info->character_sets; *charset; charset++) {
			free(*charset);
		}
		free(info->character_sets);
	}

	memset(info, 0, sizeof(*info));
}

bool wlf_font_enumerate_system_fonts(wlf_font_enum_callback callback, void *user_data) {
	if (!g_active_backend) {
		wlf_log(WLF_ERROR, "No active font backend");
		return false;
	}

	if (!g_active_backend->enumerate_fonts) {
		wlf_log(WLF_ERROR, "Backend does not support font enumeration");
		return false;
	}

	return g_active_backend->enumerate_fonts(callback, user_data);
}

bool wlf_font_find_system_fonts(const char *pattern, wlf_font_enum_callback callback, void *user_data) {
	if (!g_active_backend) {
		wlf_log(WLF_ERROR, "No active font backend");
		return false;
	}

	if (!g_active_backend->find_fonts) {
		wlf_log(WLF_ERROR, "Backend does not support font search");
		return false;
	}

	return g_active_backend->find_fonts(pattern, callback, user_data);
}

char* wlf_font_get_system_font_path(const char *family_name, enum wlf_font_style style, enum wlf_font_weight weight) {
	if (!g_active_backend) {
		wlf_log(WLF_ERROR, "No active font backend");
		return NULL;
	}

	if (!g_active_backend->get_font_path) {
		wlf_log(WLF_ERROR, "Backend does not support font path lookup");
		return NULL;
	}

	return g_active_backend->get_font_path(family_name, style, weight);
}

char* wlf_font_get_system_default_font(const char *language) {
	if (!g_active_backend) {
		wlf_log(WLF_ERROR, "No active font backend");
		return NULL;
	}

	if (!g_active_backend->get_default_font) {
		wlf_log(WLF_ERROR, "Backend does not support default font lookup");
		return NULL;
	}

	return g_active_backend->get_default_font(language);
}

char* wlf_font_get_system_monospace_font(void) {
	if (!g_active_backend) {
		wlf_log(WLF_ERROR, "No active font backend");
		return NULL;
	}

	if (!g_active_backend->get_monospace_font) {
		wlf_log(WLF_ERROR, "Backend does not support monospace font lookup");
		return NULL;
	}

	return g_active_backend->get_monospace_font();
}
