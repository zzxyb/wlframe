#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

#define WLF_BACKEND_EXAMPLE 1000

struct wlf_backend_example {
	struct wlf_backend base;     /**< Base backend structure */
	bool started;                /**< Whether backend is started */
	char *custom_data;           /**< Plugin-specific data */
};

static bool example_backend_start(struct wlf_backend *backend);
static void example_backend_stop(struct wlf_backend *backend);
static void example_backend_destroy(struct wlf_backend *backend);

bool wlf_backend_plugin_init(void);
void wlf_backend_plugin_cleanup(void);

static const struct wlf_backend_impl example_impl = {
	.start = example_backend_start,
	.stop = example_backend_stop,
	.destroy = example_backend_destroy,
};

static bool example_backend_start(struct wlf_backend *backend) {
	struct wlf_backend_example *example = (struct wlf_backend_example *)backend;

	if (example->started) {
		return true;
	}

	wlf_log(WLF_INFO, "Starting example plugin backend with data: %s",
		example->custom_data);

	example->started = true;
	return true;
}

static void example_backend_stop(struct wlf_backend *backend) {
	struct wlf_backend_example *example = (struct wlf_backend_example *)backend;

	if (!example->started) {
		return;
	}

	wlf_log(WLF_INFO, "Stopping example plugin backend");
	example->started = false;
}

static void example_backend_destroy(struct wlf_backend *backend) {
	struct wlf_backend_example *example = (struct wlf_backend_example *)backend;

	wlf_log(WLF_INFO, "Destroying example plugin backend");

	example_backend_stop(backend);
	free(example->custom_data);
	free(example);
}

static struct wlf_backend *example_backend_create(void *args) {
	WLF_UNUSED(args);

	struct wlf_backend_example *backend = calloc(1, sizeof(struct wlf_backend_example));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate example backend");
		return NULL;
	}

	// Initialize base backend
	backend->base.impl = &example_impl;
	backend->base.type = WLF_BACKEND_EXAMPLE;
	backend->base.data = backend;

	// Initialize signals
	wlf_signal_init(&backend->base.events.destroy);

	// Plugin-specific initialization
	backend->custom_data = strdup("Hello from plugin!");
	backend->started = false;

	wlf_log(WLF_INFO, "Created example plugin backend");
	return &backend->base;
}

static bool example_backend_is_available(void) {
	// Plugin is always available for demo purposes
	return true;
}

// Plugin registry entry (static to this plugin)
static struct wlf_backend_registry_entry plugin_entry = {
	.type = WLF_BACKEND_EXAMPLE,
	.name = "example-plugin",
	.priority = 50,
	.create = example_backend_create,
	.is_available = example_backend_is_available,
	.handle = NULL,  // Will be set by the plugin loader
};

/**
 * @brief Plugin initialization function
 * This function is called when the plugin is loaded
 * Every plugin must export this function
 */
bool wlf_backend_plugin_init(void) {
	wlf_log(WLF_INFO, "Initializing example backend plugin");

	if (!wlf_backend_register(&plugin_entry)) {
		wlf_log(WLF_ERROR, "Failed to register example backend plugin");
		return false;
	}

	wlf_log(WLF_INFO, "Example backend plugin initialized successfully");
	return true;
}

/**
 * @brief Plugin cleanup function
 * This function is called when the plugin is unloaded
 * Every plugin should export this function
 */
void wlf_backend_plugin_cleanup(void) {
	wlf_log(WLF_INFO, "Cleaning up example backend plugin");
	wlf_backend_unregister(WLF_BACKEND_EXAMPLE);
}
