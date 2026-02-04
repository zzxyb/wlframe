#include "wlf/utils/wlf_standard_paths.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

// Platform-specific implementations
#if WLF_HAS_LINUX_PLATFORM
extern struct wlf_standard_paths *wlf_standard_paths_linux_create(void);
#elif WLF_HAS_MACOS_PLATFORM
extern struct wlf_standard_paths *wlf_standard_paths_macos_create(void);
#endif

// Display names for locations
static const char *location_names[] = {
	[WLF_LOCATION_HOME] = "Home",
	[WLF_LOCATION_DESKTOP] = "Desktop",
	[WLF_LOCATION_DOCUMENTS] = "Documents",
	[WLF_LOCATION_DOWNLOADS] = "Downloads",
	[WLF_LOCATION_MUSIC] = "Music",
	[WLF_LOCATION_PICTURES] = "Pictures",
	[WLF_LOCATION_VIDEOS] = "Videos",
	[WLF_LOCATION_CONFIG] = "Config",
	[WLF_LOCATION_DATA] = "Data",
	[WLF_LOCATION_CACHE] = "Cache",
	[WLF_LOCATION_RUNTIME] = "Runtime",
	[WLF_LOCATION_TEMP] = "Temp",
	[WLF_LOCATION_APPLICATIONS] = "Applications",
};

// Singleton instance
static struct wlf_standard_paths *global_paths = NULL;

const char *wlf_standard_path_display_name(enum wlf_standard_location location) {
	if (location >= 0 && location < sizeof(location_names) / sizeof(location_names[0])) {
		return location_names[location];
	}
	return "Unknown";
}

// Helper function to build path
static char *build_path(const char *base, const char *suffix) {
	if (!base) {
		return NULL;
	}

	if (!suffix) {
		return strdup(base);
	}

	size_t len = strlen(base) + strlen(suffix) + 2; // +2 for '/' and '\0'
	char *path = malloc(len);
	if (!path) {
		return NULL;
	}

	snprintf(path, len, "%s/%s", base, suffix);
	return path;
}

bool wlf_standard_path_ensure_dir(const char *path) {
	if (!path) {
		return false;
	}

	// Check if directory already exists
	struct stat st;
	if (stat(path, &st) == 0) {
		return S_ISDIR(st.st_mode);
	}

	// Create parent directories recursively
	char *path_copy = strdup(path);
	if (!path_copy) {
		return false;
	}

	char *p = path_copy;
	if (*p == '/') {
		p++;
	}

	while (*p) {
		if (*p == '/') {
			*p = '\0';
			if (mkdir(path_copy, 0755) != 0 && errno != EEXIST) {
				free(path_copy);
				return false;
			}
			*p = '/';
		}
		p++;
	}

	// Create the final directory
	bool result = (mkdir(path_copy, 0755) == 0 || errno == EEXIST);
	free(path_copy);
	return result;
}

struct wlf_standard_paths *wlf_standard_paths_auto_create(void) {
#if WLF_HAS_LINUX_PLATFORM
	return wlf_standard_paths_linux_create();
#elif WLF_HAS_MACOS_PLATFORM
	return wlf_standard_paths_macos_create();
#else
	wlf_log(WLF_ERROR, "Unsupported platform for standard paths");
	return NULL;
#endif
}

void wlf_standard_paths_destroy(struct wlf_standard_paths *paths) {
	if (!paths) {
		return;
	}

	if (paths->impl && paths->impl->destroy) {
		paths->impl->destroy(paths);
	} else {
		free(paths);
	}
}

static struct wlf_standard_paths *get_singleton(void) {
	if (!global_paths) {
		global_paths = wlf_standard_paths_auto_create();
	}
	return global_paths;
}

char *wlf_standard_path_get(struct wlf_standard_paths *paths, 
		enum wlf_standard_location location) {
	if (!paths) {
		paths = get_singleton();
	}

	if (!paths || !paths->impl || !paths->impl->get_path) {
		wlf_log(WLF_ERROR, "Invalid standard paths instance");
		return NULL;
	}

	return paths->impl->get_path(paths, location);
}

char *wlf_standard_path_writable(struct wlf_standard_paths *paths,
		enum wlf_standard_location location, const char *app_name) {
	char *base = wlf_standard_path_get(paths, location);
	if (!base) {
		return NULL;
	}

	if (!app_name) {
		return base;
	}

	char *path = build_path(base, app_name);
	free(base);
	return path;
}
