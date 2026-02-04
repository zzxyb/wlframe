#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>

/**
 * @brief Global backend registry
 */
static struct {
	struct wlf_linked_list backends;  /**< List of registered backends */
	bool initialized;                 /**< Whether backend system is initialized */
} backend_registry = {0};

/**
 * @brief Backend name strings
 */
static const char *backend_type_names[] = {
	[WLF_BACKEND_AUTOCREATE] = "autocreate",
	[WLF_BACKEND_WAYLAND] = "wayland",
	[WLF_BACKEND_MACOS] = "macos",
};

/**
 * @brief Find a registered backend by type
 * @param type Backend type to find
 * @return Backend registry entry, or NULL if not found
 */
static struct wlf_backend_registry_entry *find_backend_entry(enum wlf_backend_type type) {
	struct wlf_backend_registry_entry *entry;
	wlf_linked_list_for_each(entry, &backend_registry.backends, link) {
		if (entry->type == type) {
			return entry;
		}
	}
	return NULL;
}

/**
 * @brief Find the best available backend
 * @return Backend registry entry for the best backend, or NULL if none available
 */
static struct wlf_backend_registry_entry *find_best_backend(void) {
	struct wlf_backend_registry_entry *best = NULL;
	int best_priority = -1;

	struct wlf_backend_registry_entry *entry;
	wlf_linked_list_for_each(entry, &backend_registry.backends, link) {
		if (entry->is_available && entry->is_available() && entry->priority > best_priority) {
			best = entry;
			best_priority = entry->priority;
		}
	}

	return best;
}

void wlf_backend_init(void) {
	if (backend_registry.initialized) {
		return;
	}

	wlf_linked_list_init(&backend_registry.backends);
	backend_registry.initialized = true;

	wlf_log(WLF_INFO, "Backend subsystem initialized");
}

void wlf_backend_finish(void) {
	if (!backend_registry.initialized) {
		return;
	}

	// Unregister all backends and unload plugins
	struct wlf_backend_registry_entry *entry, *tmp;
	wlf_linked_list_for_each_safe(entry, tmp, &backend_registry.backends, link) {
		wlf_backend_unregister(entry->type);
	}

	backend_registry.initialized = false;
	wlf_log(WLF_INFO, "Backend subsystem finalized");
}

bool wlf_backend_register(struct wlf_backend_registry_entry *entry) {
	if (!backend_registry.initialized) {
		wlf_log(WLF_ERROR, "Backend subsystem not initialized");
		return false;
	}

	if (entry == NULL || !entry->create || !entry->name) {
		wlf_log(WLF_ERROR, "Invalid backend registry entry");
		return false;
	}

	// Check if backend is already registered
	if (find_backend_entry(entry->type)) {
		wlf_log(WLF_INFO, "Backend type %d already registered", entry->type);
		return false;
	}

	wlf_linked_list_init(&entry->link);
	wlf_linked_list_insert(&backend_registry.backends, &entry->link);

	wlf_log(WLF_INFO, "Registered backend: %s (type=%d, priority=%d)",
		entry->name, entry->type, entry->priority);

	return true;
}

void wlf_backend_unregister(enum wlf_backend_type type) {
	struct wlf_backend_registry_entry *entry = find_backend_entry(type);
	if (entry == NULL) {
		return;
	}

	wlf_log(WLF_INFO, "Unregistering backend: %s", entry->name);

	// Unload plugin if it was dynamically loaded
	if (entry->handle) {
		wlf_backend_plugin_cleanup_func_t cleanup_func =
			dlsym(entry->handle, "wlf_backend_plugin_cleanup");
		if (cleanup_func) {
			cleanup_func();
		}
		dlclose(entry->handle);
	}

	wlf_linked_list_remove(&entry->link);
}

bool wlf_backend_load_plugin(const char *plugin_path) {
	if (plugin_path == NULL) {
		wlf_log(WLF_ERROR, "Plugin path is NULL");
		return false;
	}

	void *handle = dlopen(plugin_path, RTLD_LAZY);
	if (handle == NULL) {
		wlf_log(WLF_ERROR, "Failed to load plugin %s: %s", plugin_path, dlerror());
		return false;
	}

	wlf_backend_plugin_init_func_t init_func = dlsym(handle, "wlf_backend_plugin_init");
	if (!init_func) {
		wlf_log(WLF_ERROR, "Plugin %s missing wlf_backend_plugin_init function", plugin_path);
		dlclose(handle);
		return false;
	}

	if (!init_func()) {
		wlf_log(WLF_ERROR, "Plugin %s initialization failed", plugin_path);
		dlclose(handle);
		return false;
	}

	wlf_log(WLF_INFO, "Loaded backend plugin: %s", plugin_path);
	return true;
}

void wlf_backend_unload_plugin(const char *plugin_path) {
	// Find the plugin by path (this is a simplified approach)
	// In a real implementation, you'd want to track loaded plugins
	wlf_log(WLF_INFO, "Unloading plugin: %s", plugin_path);
}

struct wlf_backend *wlf_backend_autocreate(void) {
	if (!backend_registry.initialized) {
		wlf_log(WLF_ERROR, "Backend subsystem not initialized");
		return NULL;
	}

	// Check environment variable for backend preference
	const char *backend_env = wlf_get_env("WLF_BACKEND");
	if (backend_env) {
		// Try to find backend by name
		struct wlf_backend_registry_entry *entry;
		wlf_linked_list_for_each(entry, &backend_registry.backends, link) {
			if (strcmp(entry->name, backend_env) == 0) {
				if (!entry->is_available || entry->is_available()) {
					wlf_log(WLF_INFO, "Using backend from environment: %s", backend_env);
					return entry->create(NULL);
				} else {
					wlf_log(WLF_INFO, "Requested backend %s not available", backend_env);
					break;
				}
			}
		}
	}

	// Auto-detect best backend
	struct wlf_backend_registry_entry *best = find_best_backend();
	if (best == NULL) {
		wlf_log(WLF_ERROR, "No available backend found");
		return NULL;
	}

	wlf_log(WLF_INFO, "Auto-creating backend: %s", best->name);
	return best->create(NULL);
}

struct wlf_backend *wlf_backend_create(const struct wlf_backend_create_args *args) {
	if (args == NULL) {
		wlf_log(WLF_ERROR, "Backend creation arguments are NULL");
		return NULL;
	}

	struct wlf_backend_registry_entry *entry = find_backend_entry(args->type);
	if (entry == NULL) {
		wlf_log(WLF_ERROR, "Backend type %d not registered", args->type);
		return NULL;
	}

	if (entry->is_available && !entry->is_available()) {
		wlf_log(WLF_ERROR, "Backend %s not available", entry->name);
		return NULL;
	}

	wlf_log(WLF_INFO, "Creating backend: %s", entry->name);
	return entry->create((void *)args);
}

bool wlf_backend_start(struct wlf_backend *backend) {
	if (backend == NULL || !backend->impl || !backend->impl->start) {
		wlf_log(WLF_ERROR, "Invalid backend or missing start implementation");
		return false;
	}

	wlf_log(WLF_DEBUG, "Starting backend type %d", backend->type);
	return backend->impl->start(backend);
}

void wlf_backend_stop(struct wlf_backend *backend) {
	if (backend == NULL || !backend->impl) {
		return;
	}

	wlf_log(WLF_DEBUG, "Stopping backend type %d", backend->type);
	if (backend->impl->stop) {
		backend->impl->stop(backend);
	}
}

void wlf_backend_destroy(struct wlf_backend *backend) {
	if (backend == NULL) {
		return;
	}

	wlf_log(WLF_DEBUG, "Destroying backend type %d", backend->type);

	// Emit destroy signal
	wlf_signal_emit_mutable(&backend->events.destroy, backend);

	if (backend->impl && backend->impl->destroy) {
		backend->impl->destroy(backend);
	} else {
		// Default cleanup
		free(backend);
	}
}

enum wlf_backend_type wlf_backend_get_type(struct wlf_backend *backend) {
	return backend ? backend->type : WLF_BACKEND_AUTOCREATE;
}

bool wlf_backend_is_active(struct wlf_backend *backend) {
	// This would typically check some internal state
	// For now, just check if backend exists
	return backend != NULL;
}

const char *wlf_backend_type_name(enum wlf_backend_type type) {
	if (type >= 0 && type < (int)(sizeof(backend_type_names) / sizeof(backend_type_names[0]))) {
		return backend_type_names[type];
	}
	return "unknown";
}
