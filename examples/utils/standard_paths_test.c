#include "wlf/utils/wlf_standard_paths.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>

static void print_location(struct wlf_standard_paths *paths, 
		enum wlf_standard_location location) {
	const char *name = wlf_standard_path_display_name(location);
	char *path = wlf_standard_path_get(paths, location);
	
	if (path) {
		printf("%-20s: %s\n", name, path);
		free(path);
	} else {
		printf("%-20s: (not available)\n", name);
	}
}

static void test_app_specific_paths(struct wlf_standard_paths *paths) {
	printf("\n--- Application-specific paths (for 'myapp') ---\n");
	
	const char *app_name = "myapp";
	
	char *config = wlf_standard_path_writable(paths, WLF_LOCATION_CONFIG, app_name);
	if (config) {
		printf("App Config          : %s\n", config);
		
		// Test directory creation
		if (wlf_standard_path_ensure_dir(config)) {
			printf("  -> Directory ensured successfully\n");
		} else {
			printf("  -> Failed to ensure directory\n");
		}
		free(config);
	}
	
	char *data = wlf_standard_path_writable(paths, WLF_LOCATION_DATA, app_name);
	if (data) {
		printf("App Data            : %s\n", data);
		free(data);
	}
	
	char *cache = wlf_standard_path_writable(paths, WLF_LOCATION_CACHE, app_name);
	if (cache) {
		printf("App Cache           : %s\n", cache);
		free(cache);
	}
}

int main(void) {
	wlf_log_init(WLF_INFO, NULL);
	
	printf("=== wlframe Standard Paths Test ===\n\n");
	
	// Create standard paths instance
	struct wlf_standard_paths *paths = wlf_standard_paths_auto_create();
	if (!paths) {
		printf("Failed to create standard paths instance\n");
		return 1;
	}
	
	printf("--- Standard locations ---\n");
	print_location(paths, WLF_LOCATION_HOME);
	print_location(paths, WLF_LOCATION_DESKTOP);
	print_location(paths, WLF_LOCATION_DOCUMENTS);
	print_location(paths, WLF_LOCATION_DOWNLOADS);
	print_location(paths, WLF_LOCATION_MUSIC);
	print_location(paths, WLF_LOCATION_PICTURES);
	print_location(paths, WLF_LOCATION_VIDEOS);
	print_location(paths, WLF_LOCATION_CONFIG);
	print_location(paths, WLF_LOCATION_DATA);
	print_location(paths, WLF_LOCATION_CACHE);
	print_location(paths, WLF_LOCATION_RUNTIME);
	print_location(paths, WLF_LOCATION_TEMP);
	print_location(paths, WLF_LOCATION_APPLICATIONS);
	
	test_app_specific_paths(paths);
	
	// Clean up
	wlf_standard_paths_destroy(paths);
	
	printf("\n--- Testing singleton API (NULL paths) ---\n");
	char *home = wlf_standard_path_get(NULL, WLF_LOCATION_HOME);
	if (home) {
		printf("Home (singleton)    : %s\n", home);
		free(home);
	}
	
	return 0;
}
