#include "wlf/utils/wlf_standard_paths.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/config.h"

#if WLF_HAS_LINUX_PLATFORM

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

// Linux-specific implementation using XDG Base Directory Specification

// Helper function to get home directory
static char *get_home_dir(void) {
	const char *home = getenv("HOME");
	if (home) {
		return strdup(home);
	}

	// Fallback to passwd entry
	struct passwd *pw = getpwuid(getuid());
	if (pw && pw->pw_dir) {
		return strdup(pw->pw_dir);
	}

	return NULL;
}

// Helper function to build path
static char *build_path(const char *base, const char *suffix) {
	if (!base) {
		return NULL;
	}

	if (!suffix) {
		return strdup(base);
	}

	size_t len = strlen(base) + strlen(suffix) + 2;
	char *path = malloc(len);
	if (!path) {
		return NULL;
	}

	snprintf(path, len, "%s/%s", base, suffix);
	return path;
}

static char *get_xdg_dir(const char *env_var, const char *default_suffix) {
	const char *xdg = getenv(env_var);
	if (xdg && xdg[0] == '/') {
		return strdup(xdg);
	}

	char *home = get_home_dir();
	if (!home) {
		return NULL;
	}

	char *path = build_path(home, default_suffix);
	free(home);
	return path;
}

static char *get_xdg_user_dir(const char *dir_name, const char *default_suffix) {
	// Try to read from ~/.config/user-dirs.dirs
	char *config = get_xdg_dir("XDG_CONFIG_HOME", ".config");
	if (!config) {
		return NULL;
	}

	char *user_dirs_file = build_path(config, "user-dirs.dirs");
	free(config);

	if (!user_dirs_file) {
		return NULL;
	}

	char *result = NULL;
	FILE *f = fopen(user_dirs_file, "r");
	if (f) {
		char line[1024];
		char search[256];
		snprintf(search, sizeof(search), "XDG_%s_DIR=\"", dir_name);

		while (fgets(line, sizeof(line), f)) {
			if (strncmp(line, search, strlen(search)) == 0) {
				char *start = line + strlen(search);
				char *end = strchr(start, '"');
				if (end) {
					*end = '\0';
					// Expand $HOME
					if (strncmp(start, "$HOME", 5) == 0) {
						char *home = get_home_dir();
						if (home) {
							result = build_path(home, start + 5);
							free(home);
						}
					} else {
						result = strdup(start);
					}
					break;
				}
			}
		}
		fclose(f);
	}
	free(user_dirs_file);

	// Fallback to default
	if (!result) {
		char *home = get_home_dir();
		if (home) {
			result = build_path(home, default_suffix);
			free(home);
		}
	}

	return result;
}

static char *linux_get_path(struct wlf_standard_paths *paths __attribute__((unused)),
		enum wlf_standard_location location) {
	switch (location) {
	case WLF_LOCATION_HOME:
		return get_home_dir();

	case WLF_LOCATION_DESKTOP:
		return get_xdg_user_dir("DESKTOP", "Desktop");

	case WLF_LOCATION_DOCUMENTS:
		return get_xdg_user_dir("DOCUMENTS", "Documents");

	case WLF_LOCATION_DOWNLOADS:
		return get_xdg_user_dir("DOWNLOAD", "Downloads");

	case WLF_LOCATION_MUSIC:
		return get_xdg_user_dir("MUSIC", "Music");

	case WLF_LOCATION_PICTURES:
		return get_xdg_user_dir("PICTURES", "Pictures");

	case WLF_LOCATION_VIDEOS:
		return get_xdg_user_dir("VIDEOS", "Videos");

	case WLF_LOCATION_CONFIG:
		return get_xdg_dir("XDG_CONFIG_HOME", ".config");

	case WLF_LOCATION_DATA:
		return get_xdg_dir("XDG_DATA_HOME", ".local/share");

	case WLF_LOCATION_CACHE:
		return get_xdg_dir("XDG_CACHE_HOME", ".cache");

	case WLF_LOCATION_RUNTIME: {
		const char *runtime = getenv("XDG_RUNTIME_DIR");
		if (runtime && runtime[0] == '/') {
			return strdup(runtime);
		}
		// Fallback to /tmp/runtime-$UID
		char path[256];
		snprintf(path, sizeof(path), "/tmp/runtime-%d", getuid());
		return strdup(path);
	}

	case WLF_LOCATION_TEMP: {
		const char *tmpdir = getenv("TMPDIR");
		if (!tmpdir) {
			tmpdir = getenv("TEMP");
		}
		if (!tmpdir) {
			tmpdir = "/tmp";
		}
		return strdup(tmpdir);
	}

	case WLF_LOCATION_APPLICATIONS:
		return get_xdg_dir("XDG_DATA_HOME", ".local/share/applications");

	default:
		wlf_log(WLF_ERROR, "Unknown standard location: %d", location);
		return NULL;
	}
}

static void linux_destroy(struct wlf_standard_paths *paths) {
	free(paths);
}

static const struct wlf_standard_paths_impl linux_impl = {
	.get_path = linux_get_path,
	.destroy = linux_destroy,
};

struct wlf_standard_paths *wlf_standard_paths_linux_create(void) {
	struct wlf_standard_paths *paths = calloc(1, sizeof(*paths));
	if (!paths) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate standard paths");
		return NULL;
	}

	paths->impl = &linux_impl;
	paths->data = NULL;

	return paths;
}

#endif // WLF_HAS_LINUX_PLATFORM
