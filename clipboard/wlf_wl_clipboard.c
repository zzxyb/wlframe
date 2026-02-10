#include "wlf/clipboard/wlf_wl_clipboard.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

// Forward declarations
static void wlf_wl_clipboard_impl_destroy(struct wlf_clipboard *clipboard);
static bool wlf_wl_clipboard_impl_set_data(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, const char *mime_type,
	const void *data, size_t size);
static void *wlf_wl_clipboard_impl_get_data(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, const char *mime_type, size_t *size);
static char **wlf_wl_clipboard_impl_get_mime_types(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, size_t *count);
static void wlf_wl_clipboard_impl_clear(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode);

static const struct wlf_clipboard_impl wl_clipboard_impl = {
	.destroy = wlf_wl_clipboard_impl_destroy,
	.set_data = wlf_wl_clipboard_impl_set_data,
	.get_data = wlf_wl_clipboard_impl_get_data,
	.get_mime_types = wlf_wl_clipboard_impl_get_mime_types,
	.clear = wlf_wl_clipboard_impl_clear,
};

// Helper functions

static void wlf_wl_mime_type_list_clear(struct wlf_linked_list *list) {
	struct wlf_wl_mime_type *mt, *tmp;
	wlf_linked_list_for_each_safe(mt, tmp, list, link) {
		wlf_linked_list_remove(&mt->link);
		free(mt->mime_type);
		free(mt);
	}
}

static void wlf_wl_clipboard_data_clear(struct wlf_wl_clipboard_data *data) {
	if (data->source) {
		wl_data_source_destroy(data->source);
		data->source = NULL;
	}

	struct wlf_clipboard_data *entry, *tmp;
	wlf_linked_list_for_each_safe(entry, tmp, &data->entries, link) {
		wlf_linked_list_remove(&entry->link);
		free(entry->mime_type);
		free(entry->data);
		free(entry);
	}
}

// Data source callbacks

static void data_source_target(void *data, struct wl_data_source *source,
		const char *mime_type) {
	wlf_log(WLF_DEBUG, "Data source target: %s", mime_type ? mime_type : "none");
}

static void data_source_send(void *data, struct wl_data_source *source,
		const char *mime_type, int32_t fd) {
	struct wlf_wl_clipboard_data *clipboard_data = data;

	wlf_log(WLF_DEBUG, "Data source send request for: %s", mime_type);

	// Find the requested data
	struct wlf_clipboard_data *entry;
	wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
		if (strcmp(entry->mime_type, mime_type) == 0) {
			// Write data to fd
			ssize_t written = 0;
			while (written < (ssize_t)entry->size) {
				ssize_t ret = write(fd, (char *)entry->data + written,
					entry->size - written);
				if (ret < 0) {
					if (errno == EINTR) {
						continue;
					}
					wlf_log(WLF_ERROR, "Failed to write clipboard data: %s",
						strerror(errno));
					break;
				}
				written += ret;
			}
			close(fd);
			return;
		}
	}

	wlf_log(WLF_INFO, "Requested MIME type not found: %s", mime_type);
	close(fd);
}

static void data_source_cancelled(void *data, struct wl_data_source *source) {
	wlf_log(WLF_DEBUG, "Data source cancelled");
}

static const struct wl_data_source_listener data_source_listener = {
	.target = data_source_target,
	.send = data_source_send,
	.cancelled = data_source_cancelled,
};

// Data offer callbacks

static void data_offer_offer(void *data, struct wl_data_offer *offer,
		const char *mime_type) {
	struct wlf_linked_list *mime_types = data;

	struct wlf_wl_mime_type *mt = malloc(sizeof(struct wlf_wl_mime_type));
	if (mt == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate MIME type entry");
		return;
	}

	mt->mime_type = strdup(mime_type);
	if (mt->mime_type == NULL) {
		wlf_log(WLF_ERROR, "Failed to duplicate MIME type string");
		free(mt);
		return;
	}

	wlf_linked_list_insert(mime_types, &mt->link);
	wlf_log(WLF_DEBUG, "Data offer: %s", mime_type);
}

static const struct wl_data_offer_listener data_offer_listener = {
	.offer = data_offer_offer,
};

// Data device callbacks

static void data_device_data_offer(void *data, struct wl_data_device *device,
		struct wl_data_offer *offer) {
	wlf_log(WLF_DEBUG, "New data offer received");
}

static void data_device_selection(void *data, struct wl_data_device *device,
		struct wl_data_offer *offer) {
	struct wlf_wl_clipboard *clipboard = data;

	wlf_log(WLF_DEBUG, "Selection changed");

	// Clear previous clipboard offer
	if (clipboard->clipboard_offer) {
		wl_data_offer_destroy(clipboard->clipboard_offer);
	}
	wlf_wl_mime_type_list_clear(&clipboard->clipboard_mime_types);

	clipboard->clipboard_offer = offer;

	if (offer) {
		wl_data_offer_add_listener(offer, &data_offer_listener,
			&clipboard->clipboard_mime_types);
	}

	// Emit changed signal
	wlf_signal_emit_mutable(&clipboard->base.events.changed, NULL);
}

static const struct wl_data_device_listener data_device_listener = {
	.data_offer = data_device_data_offer,
	.selection = data_device_selection,
};

// Implementation functions

static void wlf_wl_clipboard_impl_destroy(struct wlf_clipboard *clipboard) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);

	wlf_log(WLF_DEBUG, "Destroying Wayland clipboard");

	// Clear clipboard data
	wlf_wl_clipboard_data_clear(&wl_clipboard->clipboard_data);
	wlf_wl_clipboard_data_clear(&wl_clipboard->selection_data);

	// Destroy offers
	if (wl_clipboard->clipboard_offer) {
		wl_data_offer_destroy(wl_clipboard->clipboard_offer);
	}
	if (wl_clipboard->selection_offer) {
		wl_data_offer_destroy(wl_clipboard->selection_offer);
	}

	// Clear MIME type lists
	wlf_wl_mime_type_list_clear(&wl_clipboard->clipboard_mime_types);
	wlf_wl_mime_type_list_clear(&wl_clipboard->selection_mime_types);

	// Destroy data device
	if (wl_clipboard->data_device) {
		wl_data_device_destroy(wl_clipboard->data_device);
	}
}

static bool wlf_wl_clipboard_impl_set_data(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type,
		const void *data, size_t size) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);

	struct wlf_wl_clipboard_data *clipboard_data =
		(mode == WLF_CLIPBOARD_MODE_CLIPBOARD) ?
		&wl_clipboard->clipboard_data : &wl_clipboard->selection_data;

	// Check if we already have an entry for this MIME type
	struct wlf_clipboard_data *entry;
	bool found = false;
	wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
		if (strcmp(entry->mime_type, mime_type) == 0) {
			// Update existing entry
			free(entry->data);
			entry->data = malloc(size);
			if (entry->data == NULL) {
				wlf_log(WLF_ERROR, "Failed to allocate clipboard data");
				return false;
			}
			memcpy(entry->data, data, size);
			entry->size = size;
			found = true;
			break;
		}
	}

	if (!found) {
		// Create new entry
		entry = malloc(sizeof(struct wlf_clipboard_data));
		if (entry == NULL) {
			wlf_log(WLF_ERROR, "Failed to allocate clipboard data entry");
			return false;
		}

		entry->mime_type = strdup(mime_type);
		if (entry->mime_type == NULL) {
			wlf_log(WLF_ERROR, "Failed to duplicate MIME type");
			free(entry);
			return false;
		}

		entry->data = malloc(size);
		if (entry->data == NULL) {
			wlf_log(WLF_ERROR, "Failed to allocate clipboard data");
			free(entry->mime_type);
			free(entry);
			return false;
		}

		memcpy(entry->data, data, size);
		entry->size = size;

		wlf_linked_list_insert(&clipboard_data->entries, &entry->link);
	}

	// Create or update data source
	if (!clipboard_data->source) {
		clipboard_data->source = wl_data_device_manager_create_data_source(
			wl_clipboard->data_device_manager);
		if (clipboard_data->source == NULL) {
			wlf_log(WLF_ERROR, "Failed to create data source");
			return false;
		}

		wl_data_source_add_listener(clipboard_data->source,
			&data_source_listener, clipboard_data);

		// Offer all MIME types
		wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
			wl_data_source_offer(clipboard_data->source, entry->mime_type);
		}
	} else {
		// Offer the new MIME type if it's a new entry
		if (!found) {
			wl_data_source_offer(clipboard_data->source, mime_type);
		}
	}

	// Set the selection
	if (mode == WLF_CLIPBOARD_MODE_CLIPBOARD) {
		wl_data_device_set_selection(wl_clipboard->data_device,
			clipboard_data->source, 0); // serial 0 for now
	}

	return true;
}

static void *wlf_wl_clipboard_impl_get_data(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type, size_t *size) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);

	struct wl_data_offer *offer =
		(mode == WLF_CLIPBOARD_MODE_CLIPBOARD) ?
		wl_clipboard->clipboard_offer : wl_clipboard->selection_offer;

	if (offer == NULL) {
		// No offer available, try to get from our own data
		struct wlf_wl_clipboard_data *clipboard_data =
			(mode == WLF_CLIPBOARD_MODE_CLIPBOARD) ?
			&wl_clipboard->clipboard_data : &wl_clipboard->selection_data;

		struct wlf_clipboard_data *entry;
		wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
			if (strcmp(entry->mime_type, mime_type) == 0) {
				void *data = malloc(entry->size);
				if (data == NULL) {
					wlf_log(WLF_ERROR, "Failed to allocate data");
					return NULL;
				}
				memcpy(data, entry->data, entry->size);
				if (size) {
					*size = entry->size;
				}
				return data;
			}
		}
		return NULL;
	}

	// Create a pipe to receive data
	int fds[2];
	if (pipe(fds) == -1) {
		wlf_log(WLF_ERROR, "Failed to create pipe: %s", strerror(errno));
		return NULL;
	}

	// Request data
	wl_data_offer_receive(offer, mime_type, fds[1]);
	close(fds[1]);

	// Flush the display
	wl_display_roundtrip(wl_clipboard->display->base);

	// Read data from pipe
	size_t capacity = 4096;
	size_t received = 0;
	char *buffer = malloc(capacity);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate buffer");
		close(fds[0]);
		return NULL;
	}

	while (true) {
		if (received + 1024 > capacity) {
			capacity *= 2;
			char *new_buffer = realloc(buffer, capacity);
			if (new_buffer == NULL) {
				wlf_log(WLF_ERROR, "Failed to reallocate buffer");
				free(buffer);
				close(fds[0]);
				return NULL;
			}
			buffer = new_buffer;
		}

		ssize_t ret = read(fds[0], buffer + received, capacity - received);
		if (ret == 0) {
			// EOF
			break;
		}
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}
			wlf_log(WLF_ERROR, "Failed to read clipboard data: %s",
				strerror(errno));
			free(buffer);
			close(fds[0]);
			return NULL;
		}
		received += ret;
	}

	close(fds[0]);

	if (size) {
		*size = received;
	}

	return buffer;
}

static char **wlf_wl_clipboard_impl_get_mime_types(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, size_t *count) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);

	struct wlf_linked_list *mime_types =
		(mode == WLF_CLIPBOARD_MODE_CLIPBOARD) ?
		&wl_clipboard->clipboard_mime_types : &wl_clipboard->selection_mime_types;

	// Count MIME types
	*count = 0;
	struct wlf_wl_mime_type *mt;
	wlf_linked_list_for_each(mt, mime_types, link) {
		(*count)++;
	}

	if (*count == 0) {
		return NULL;
	}

	// Allocate array
	char **array = malloc(*count * sizeof(char *));
	if (array == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate MIME types array");
		*count = 0;
		return NULL;
	}

	// Fill array
	size_t i = 0;
	wlf_linked_list_for_each(mt, mime_types, link) {
		array[i] = strdup(mt->mime_type);
		if (array[i] == NULL) {
			wlf_log(WLF_ERROR, "Failed to duplicate MIME type");
			// Free already allocated strings
			for (size_t j = 0; j < i; j++) {
				free(array[j]);
			}
			free(array);
			*count = 0;
			return NULL;
		}
		i++;
	}

	return array;
}

static void wlf_wl_clipboard_impl_clear(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);

	struct wlf_wl_clipboard_data *clipboard_data =
		(mode == WLF_CLIPBOARD_MODE_CLIPBOARD) ?
		&wl_clipboard->clipboard_data : &wl_clipboard->selection_data;

	wlf_wl_clipboard_data_clear(clipboard_data);

	// Clear the selection
	if (mode == WLF_CLIPBOARD_MODE_CLIPBOARD) {
		wl_data_device_set_selection(wl_clipboard->data_device, NULL, 0);
	}
}

// Public functions

struct wlf_wl_clipboard *wlf_wl_clipboard_create(
		struct wlf_wl_display *display, struct wl_seat *seat) {
	assert(display);

	struct wlf_wl_clipboard *wl_clipboard = malloc(sizeof(struct wlf_wl_clipboard));
	if (wl_clipboard == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate wlf_wl_clipboard");
		return NULL;
	}

	// Initialize base clipboard
	memset(wl_clipboard, 0, sizeof(struct wlf_wl_clipboard));
	wl_clipboard->base.impl = &wl_clipboard_impl;
	wl_clipboard->base.data = wl_clipboard;

	wlf_signal_init(&wl_clipboard->base.events.destroy);
	wlf_signal_init(&wl_clipboard->base.events.changed);
	wlf_signal_init(&wl_clipboard->base.events.selection_changed);

	wl_clipboard->display = display;
	wl_clipboard->seat = seat;

	// Initialize linked lists
	wlf_linked_list_init(&wl_clipboard->clipboard_data.entries);
	wlf_linked_list_init(&wl_clipboard->selection_data.entries);
	wlf_linked_list_init(&wl_clipboard->clipboard_mime_types);
	wlf_linked_list_init(&wl_clipboard->selection_mime_types);

	// Find data_device_manager
	struct wlf_wl_interface *interface = wlf_wl_display_find_interface(
		display, "wl_data_device_manager");
	if (interface == NULL) {
		wlf_log(WLF_ERROR, "wl_data_device_manager not available");
		free(wl_clipboard);
		return NULL;
	}

	wl_clipboard->data_device_manager = wl_registry_bind(
		display->registry, interface->name,
		&wl_data_device_manager_interface, 3);
	if (wl_clipboard->data_device_manager == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_data_device_manager");
		free(wl_clipboard);
		return NULL;
	}

	// TODO: If seat is NULL, find the first available seat
	if (seat == NULL) {
		wlf_log(WLF_ERROR, "Seat is required for clipboard");
		wl_data_device_manager_destroy(wl_clipboard->data_device_manager);
		free(wl_clipboard);
		return NULL;
	}

	// Create data device
	wl_clipboard->data_device = wl_data_device_manager_get_data_device(
		wl_clipboard->data_device_manager, seat);
	if (wl_clipboard->data_device == NULL) {
		wlf_log(WLF_ERROR, "Failed to get data device");
		wl_data_device_manager_destroy(wl_clipboard->data_device_manager);
		free(wl_clipboard);
		return NULL;
	}

	wl_data_device_add_listener(wl_clipboard->data_device,
		&data_device_listener, wl_clipboard);

	wlf_log(WLF_INFO, "Wayland clipboard created");

	return wl_clipboard;
}

void wlf_wl_clipboard_destroy(struct wlf_wl_clipboard *clipboard) {
	if (clipboard == NULL) {
		return;
	}

	wlf_clipboard_destroy(&clipboard->base);
	free(clipboard);
}
