#include "wlf/utils/wlf_standard_paths.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/config.h"

#if WLF_HAS_MACOS_PLATFORM

#import <Foundation/Foundation.h>

#include <stdlib.h>
#include <string.h>

// macOS-specific implementation using Foundation framework

static char *macos_get_path(struct wlf_standard_paths *paths __attribute__((unused)),
		enum wlf_standard_location location) {
	@autoreleasepool {
		NSArray *paths_array = nil;
		NSSearchPathDirectory dir;
		bool use_search_path = true;
		NSString *result_path = nil;

		switch (location) {
		case WLF_LOCATION_HOME:
			result_path = NSHomeDirectory();
			use_search_path = false;
			break;

		case WLF_LOCATION_DESKTOP:
			dir = NSDesktopDirectory;
			break;

		case WLF_LOCATION_DOCUMENTS:
			dir = NSDocumentDirectory;
			break;

		case WLF_LOCATION_DOWNLOADS:
			dir = NSDownloadsDirectory;
			break;

		case WLF_LOCATION_MUSIC:
			dir = NSMusicDirectory;
			break;

		case WLF_LOCATION_PICTURES:
			dir = NSPicturesDirectory;
			break;

		case WLF_LOCATION_VIDEOS:
			dir = NSMoviesDirectory;
			break;

		case WLF_LOCATION_CONFIG:
		case WLF_LOCATION_DATA:
			dir = NSApplicationSupportDirectory;
			break;

		case WLF_LOCATION_CACHE:
			dir = NSCachesDirectory;
			break;

		case WLF_LOCATION_RUNTIME:
		case WLF_LOCATION_TEMP:
			result_path = NSTemporaryDirectory();
			use_search_path = false;
			break;

		case WLF_LOCATION_APPLICATIONS:
			dir = NSApplicationDirectory;
			break;

		default:
			wlf_log(WLF_ERROR, "Unknown standard location: %d", location);
			return NULL;
		}

		if (use_search_path) {
			paths_array = NSSearchPathForDirectoriesInDomains(dir, NSUserDomainMask, YES);
			if (paths_array && [paths_array count] > 0) {
				result_path = [paths_array objectAtIndex:0];
			}
		}

		if (result_path) {
			const char *cstr = [result_path UTF8String];
			return cstr ? strdup(cstr) : NULL;
		}

		return NULL;
	}
}

static void macos_destroy(struct wlf_standard_paths *paths) {
	free(paths);
}

static const struct wlf_standard_paths_impl macos_impl = {
	.get_path = macos_get_path,
	.destroy = macos_destroy,
};

struct wlf_standard_paths *wlf_standard_paths_macos_create(void) {
	struct wlf_standard_paths *paths = calloc(1, sizeof(*paths));
	if (!paths) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate standard paths");
		return NULL;
	}

	paths->impl = &macos_impl;
	paths->data = NULL;

	return paths;
}

#endif // WLF_HAS_MACOS_PLATFORM
