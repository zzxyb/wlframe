# Backend Plugin Development Guide

This document provides detailed instructions for developing custom backend plugins for wlframe and compiling them as dynamic libraries (.so files).

## Overview

wlframe supports a plugin system that allows developers to create custom backend implementations. Plugins are loaded as dynamic libraries and must implement specific interfaces and export necessary functions.

## Plugin Interface Requirements

### 1. Required Exported Functions

Each backend plugin must export the following two functions:

```c
/**
 * @brief Plugin initialization entry point
 * @return true on successful initialization, false on failure
 */
bool wlf_backend_plugin_init(void);

/**
 * @brief Plugin cleanup function
 */
void wlf_backend_plugin_cleanup(void);
```

### 2. Backend Implementation Interface

Plugins must implement the `wlf_backend_impl` interface:

```c
struct wlf_backend_impl {
    bool (*start)(struct wlf_backend *backend);
    void (*stop)(struct wlf_backend *backend);
    void (*destroy)(struct wlf_backend *backend);
};
```

### 3. Registration Functions

Plugins need to provide the following functions to register the backend:

```c
/**
 * @brief Create backend instance
 * @param args Creation arguments (optional)
 * @return Pointer to created backend, or NULL on failure
 */
struct wlf_backend *your_backend_create(void *args);

/**
 * @brief Check if backend is available
 * @return true if available, false otherwise
 */
bool your_backend_is_available(void);

/**
 * @brief Register backend to the system
 * @return true on successful registration, false on failure
 */
bool your_backend_register(void);
```

## Development Steps

### Step 1: Create Plugin Source File

Create a new C file, for example `my_custom_backend.c`:

```c
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

// Custom backend type ID (use values greater than 1000 to avoid conflicts)
#define WLF_BACKEND_CUSTOM 1001

// Custom backend data structure
struct wlf_backend_custom {
    struct wlf_backend base;     // Base backend structure
    bool started;                // Whether backend is started
    char *custom_data;           // Plugin-specific data
};

// Forward declarations
static bool custom_backend_start(struct wlf_backend *backend);
static void custom_backend_stop(struct wlf_backend *backend);
static void custom_backend_destroy(struct wlf_backend *backend);

// Backend implementation
static const struct wlf_backend_impl custom_impl = {
    .start = custom_backend_start,
    .stop = custom_backend_stop,
    .destroy = custom_backend_destroy,
};

// Implement backend operation functions
static bool custom_backend_start(struct wlf_backend *backend) {
    struct wlf_backend_custom *custom = (struct wlf_backend_custom *)backend;

    if (custom->started) {
        return true;
    }

    // Implement backend startup logic here
    wlf_log(WLF_INFO, "Starting custom backend");

    custom->started = true;
    return true;
}

static void custom_backend_stop(struct wlf_backend *backend) {
    struct wlf_backend_custom *custom = (struct wlf_backend_custom *)backend;

    if (!custom->started) {
        return;
    }

    // Implement backend stop logic here
    wlf_log(WLF_INFO, "Stopping custom backend");

    custom->started = false;
}

static void custom_backend_destroy(struct wlf_backend *backend) {
    struct wlf_backend_custom *custom = (struct wlf_backend_custom *)backend;

    custom_backend_stop(backend);

    // Clean up resources
    if (custom->custom_data) {
        free(custom->custom_data);
    }

    wlf_signal_emit_mutable(&backend->events.destroy, backend);
    free(custom);

    wlf_log(WLF_INFO, "Custom backend destroyed");
}

// Create backend instance
struct wlf_backend *wlf_backend_custom_create(void *args) {
    struct wlf_backend_custom *backend = calloc(1, sizeof(*backend));
    if (!backend) {
        return NULL;
    }

    // Initialize base structure
    backend->base.impl = &custom_impl;
    backend->base.type = WLF_BACKEND_CUSTOM;
    backend->base.data = backend;

    // Initialize signals
    wlf_signal_init(&backend->base.events.destroy);

    // Plugin-specific initialization
    backend->custom_data = strdup("Custom backend data");
    backend->started = false;

    wlf_log(WLF_INFO, "Created custom backend");
    return &backend->base;
}

// Check backend availability
bool wlf_backend_custom_is_available(void) {
    // Implement availability check logic here
    return true;
}

// Register backend
bool wlf_backend_custom_register(void) {
    static struct wlf_backend_registry_entry entry = {
        .type = WLF_BACKEND_CUSTOM,
        .name = "custom",
        .priority = 50,  // Priority (higher value = higher priority)
        .create = wlf_backend_custom_create,
        .is_available = wlf_backend_custom_is_available,
    };

    return wlf_backend_register(&entry);
}

// Plugin entry point functions
bool wlf_backend_plugin_init(void) {
    wlf_log(WLF_INFO, "Initializing custom backend plugin");

    if (!wlf_backend_custom_register()) {
        wlf_log(WLF_ERROR, "Failed to register custom backend");
        return false;
    }

    wlf_log(WLF_INFO, "Custom backend plugin initialized successfully");
    return true;
}

void wlf_backend_plugin_cleanup(void) {
    wlf_log(WLF_INFO, "Cleaning up custom backend plugin");
    wlf_backend_unregister(WLF_BACKEND_CUSTOM);
}
```

### Step 2: Create Build Configuration

Create a `meson.build` file to compile the plugin:

```meson
project('wlframe-custom-backend', 'c',
    version: '1.0.0',
    default_options: ['c_std=c99']
)

# Find wlframe library
wlframe_dep = dependency('wlframe', required: true)

# Compile plugin as shared library
custom_backend_plugin = shared_library('wlf_backend_custom',
    'my_custom_backend.c',
    dependencies: [wlframe_dep],
    install: true,
    install_dir: get_option('libdir') / 'wlframe' / 'plugins'
)
```

Or use a simple Makefile:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -fPIC -std=c99
LDFLAGS = -shared
TARGET = libwlf_backend_custom.so
SOURCE = my_custom_backend.c

# Assume wlframe is installed in standard paths
INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib -lwlframe

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -D $(TARGET) /usr/local/lib/wlframe/plugins/$(TARGET)

.PHONY: clean install
```

### Step 3: Compile Plugin

Using meson build:

```bash
meson setup builddir
meson compile -C builddir
```

Or using make:

```bash
make
```

### Step 4: Install and Use Plugin

1. **Install plugin**:
   ```bash
   # Copy to plugin directory
   sudo cp libwlf_backend_custom.so /usr/local/lib/wlframe/plugins/
   ```

2. **Load plugin in application**:
   ```c
   #include "wlf/platform/wlf_backend.h"

   int main() {
       // Initialize backend system
       wlf_backend_init();

       // Load plugin
       if (!wlf_backend_load_plugin("/usr/local/lib/wlframe/plugins/libwlf_backend_custom.so")) {
           fprintf(stderr, "Failed to load custom backend plugin\n");
           return 1;
       }

       // Create backend instance
       struct wlf_backend_create_args args = { .type = WLF_BACKEND_CUSTOM };
       struct wlf_backend *backend = wlf_backend_create(&args);

       if (backend) {
           wlf_backend_start(backend);
           // Use backend...
           wlf_backend_destroy(backend);
       }

       // Cleanup
       wlf_backend_finish();
       return 0;
   }
   ```

## Best Practices

### 1. Error Handling
- Always check function return values
- Provide meaningful error messages
- Properly clean up allocated resources

### 2. Logging
```c
#include "wlf/utils/wlf_log.h"

// Use appropriate log levels
wlf_log(WLF_DEBUG, "Debug information");
wlf_log(WLF_INFO, "General information");
wlf_log(WLF_WARNING, "Warning message");
wlf_log(WLF_ERROR, "Error occurred");
```

### 3. Memory Management
- Use `calloc()` to initialize structures to zero
- Free all allocated memory in the destroy function
- Avoid memory leaks

### 4. Thread Safety
- If the backend needs to support multithreading, ensure proper synchronization mechanisms
- Consider using atomic operations or mutexes

## Example Plugin

The project includes a complete example plugin implementation:
- `examples/platform/wlf_backend_plugin_example.c`

This example demonstrates how to implement a simple but complete backend plugin.

## Debugging Tips

### 1. Compile-time Debugging
```bash
# Add debug information
gcc -g -DDEBUG -fPIC -shared my_custom_backend.c -o libwlf_backend_custom.so
```

### 2. Runtime Debugging
```bash
# Use gdb for debugging
gdb your_application
(gdb) set environment LD_LIBRARY_PATH=/path/to/plugin
(gdb) run
```

### 3. Log Debugging
```c
// Add logs at critical points
wlf_log(WLF_DEBUG, "Backend function called with args: %p", args);
```

## Troubleshooting

### Common Issues

1. **Plugin loading failure**
   - Check if .so file path is correct
   - Verify symbol exports are correct (`nm -D libplugin.so`)
   - Check if dependent libraries are available

2. **Symbol not found errors**
   - Ensure required functions are exported
   - Check if function signatures match interface definitions

3. **Segmentation faults**
   - Check memory allocation and deallocation
   - Verify pointer validity
   - Use valgrind to detect memory errors

### Debug Commands

```bash
# Check exported symbols
nm -D libwlf_backend_custom.so | grep wlf_backend_plugin

# Check dependencies
ldd libwlf_backend_custom.so

# Memory check
valgrind --tool=memcheck your_application
```

## Summary

Developing wlframe backend plugins requires:

1. Implementing the standard backend interface
2. Exporting required plugin functions
3. Properly managing resources and memory
4. Providing appropriate error handling and logging
5. Following best practices to ensure plugin stability

By following this guide, you can create robust and stable backend plugins to extend wlframe's functionality.
