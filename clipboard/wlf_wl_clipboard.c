#include "wlf/clipboard/wlf_wl_clipboard.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_interface.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

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

static void wlf_wl_mime_type_list_clear(struct wlf_linked_list *list);
static void wlf_wl_clipboard_data_clear(struct wlf_wl_clipboard_data *data);
static bool wlf_wl_clipboard_data_set(struct wlf_wl_clipboard_data *clipboard_data,
	const char *mime_type, const void *data, size_t size);
static bool wlf_wl_clipboard_data_ensure_source(struct wlf_wl_clipboard *wl_clipboard,
	struct wlf_wl_clipboard_data *clipboard_data);
static void *wlf_wl_offer_receive_data(struct wlf_backend_wayland *backend,
	struct wl_data_offer *offer, const char *mime_type, size_t *size);

static void data_source_target(void *data, struct wl_data_source *source,
	const char *mime_type);
static void data_source_send(void *data, struct wl_data_source *source,
	const char *mime_type, int32_t fd);
static void data_source_cancelled(void *data, struct wl_data_source *source);
static void data_offer_offer(void *data, struct wl_data_offer *offer,
	const char *mime_type);
static void data_device_data_offer(void *data, struct wl_data_device *device,
	struct wl_data_offer *offer);
static void data_device_selection(void *data, struct wl_data_device *device,
	struct wl_data_offer *offer);

static const struct wl_data_source_listener data_source_listener = {
	.target = data_source_target,
	.send = data_source_send,
	.cancelled = data_source_cancelled,
};

static const struct wl_data_offer_listener data_offer_listener = {
	.offer = data_offer_offer,
};

static const struct wl_data_device_listener data_device_listener = {
	.data_offer = data_device_data_offer,
	.selection = data_device_selection,
};

static void wlf_wl_mime_type_list_clear(struct wlf_linked_list *list) {
	struct wlf_wl_mime_type *mt, *tmp;
	wlf_linked_list_for_each_safe(mt, tmp, list, link) {
		wlf_linked_list_remove(&mt->link);
		free(mt->mime_type);
		free(mt);
	}
}

static void wlf_wl_clipboard_data_clear(struct wlf_wl_clipboard_data *data) {
	if (data->source != NULL) {
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

static bool wlf_wl_clipboard_data_set(struct wlf_wl_clipboard_data *clipboard_data,
		const char *mime_type, const void *data, size_t size) {
	struct wlf_clipboard_data *entry;
	wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
		if (strcmp(entry->mime_type, mime_type) == 0) {
			void *new_data = malloc(size);
			if (new_data == NULL && size > 0) {
				wlf_log_errno(WLF_ERROR, "failed to allocate clipboard data");
				return false;
			}

			if (size > 0) {
				memcpy(new_data, data, size);
			}

			free(entry->data);
			entry->data = new_data;
			entry->size = size;
			return true;
		}
	}

	entry = calloc(1, sizeof(*entry));
	if (entry == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate clipboard data entry");
		return false;
	}

	entry->mime_type = strdup(mime_type);
	if (entry->mime_type == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to duplicate clipboard mime type");
		free(entry);
		return false;
	}

	entry->data = malloc(size);
	if (entry->data == NULL && size > 0) {
		wlf_log_errno(WLF_ERROR, "failed to allocate clipboard payload");
		free(entry->mime_type);
		free(entry);
		return false;
	}

	if (size > 0) {
		memcpy(entry->data, data, size);
	}
	entry->size = size;
	wlf_linked_list_insert(&clipboard_data->entries, &entry->link);

	return true;
}

static bool wlf_wl_clipboard_data_ensure_source(struct wlf_wl_clipboard *wl_clipboard,
		struct wlf_wl_clipboard_data *clipboard_data) {
	struct wlf_clipboard_data *entry;

	if (clipboard_data->source != NULL) {
		return true;
	}

	clipboard_data->source = wl_data_device_manager_create_data_source(
		wl_clipboard->data_device_manager);
	if (clipboard_data->source == NULL) {
		wlf_log(WLF_ERROR, "failed to create wl_data_source");
		return false;
	}

	wl_data_source_add_listener(clipboard_data->source,
		&data_source_listener, clipboard_data);

	wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
		wl_data_source_offer(clipboard_data->source, entry->mime_type);
	}

	return true;
}

static void *wlf_wl_offer_receive_data(struct wlf_backend_wayland *backend,
		struct wl_data_offer *offer, const char *mime_type, size_t *size) {
	int fds[2] = {-1, -1};
	char *buffer = NULL;
	size_t capacity = 4096;
	size_t received = 0;

	if (pipe(fds) < 0) {
		wlf_log_errno(WLF_ERROR, "failed to create data receive pipe");
		return NULL;
	}

	wl_data_offer_receive(offer, mime_type, fds[1]);
	close(fds[1]);
	fds[1] = -1;

	if (backend->display == NULL) {
		wlf_log(WLF_ERROR, "wayland display is not available");
		close(fds[0]);
		return NULL;
	}

	wl_display_roundtrip(backend->display);

	buffer = malloc(capacity);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate data receive buffer");
		close(fds[0]);
		return NULL;
	}

	while (true) {
		if (received == capacity) {
			size_t new_capacity = capacity * 2;
			char *new_buffer = realloc(buffer, new_capacity);
			if (new_buffer == NULL) {
				wlf_log_errno(WLF_ERROR, "failed to grow data receive buffer");
				free(buffer);
				close(fds[0]);
				return NULL;
			}

			buffer = new_buffer;
			capacity = new_capacity;
		}

		ssize_t ret = read(fds[0], buffer + received, capacity - received);
		if (ret == 0) {
			break;
		}
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}

			wlf_log_errno(WLF_ERROR, "failed to read received data");
			free(buffer);
			close(fds[0]);
			return NULL;
		}

		received += (size_t)ret;
	}

	close(fds[0]);

	if (size != NULL) {
		*size = received;
	}

	return buffer;
}

static void data_source_target(void *data, struct wl_data_source *source,
		const char *mime_type) {
	(void)data;
	(void)source;
	wlf_log(WLF_DEBUG, "Data source target: %s", mime_type ? mime_type : "none");
}

static void data_source_send(void *data, struct wl_data_source *source,
		const char *mime_type, int32_t fd) {
	(void)source;
	struct wlf_wl_clipboard_data *clipboard_data = data;
	struct wlf_clipboard_data *entry;

	wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
		if (strcmp(entry->mime_type, mime_type) != 0) {
			continue;
		}

		size_t written = 0;
		while (written < entry->size) {
			ssize_t ret = write(fd, (char *)entry->data + written,
				entry->size - written);
			if (ret < 0) {
				if (errno == EINTR) {
					continue;
				}
				wlf_log_errno(WLF_ERROR, "failed to write offered clipboard data");
				break;
			}

			written += (size_t)ret;
		}

		close(fd);
		return;
	}

	close(fd);
}

static void data_source_cancelled(void *data, struct wl_data_source *source) {
	struct wlf_wl_clipboard_data *clipboard_data = data;

	if (clipboard_data->source == source) {
		clipboard_data->source = NULL;
	}

	wlf_log(WLF_DEBUG, "Data source cancelled");
}

static void data_offer_offer(void *data, struct wl_data_offer *offer,
		const char *mime_type) {
	(void)offer;
	struct wlf_linked_list *mime_types = data;
	struct wlf_wl_mime_type *mt;

	if (mime_type == NULL) {
		return;
	}

	mt = calloc(1, sizeof(*mt));
	if (mt == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate mime type entry");
		return;
	}

	mt->mime_type = strdup(mime_type);
	if (mt->mime_type == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to duplicate mime type string");
		free(mt);
		return;
	}

	wlf_linked_list_insert(mime_types, &mt->link);
}

static void data_device_data_offer(void *data, struct wl_data_device *device,
		struct wl_data_offer *offer) {
	(void)data;
	(void)device;
	(void)offer;
}

static void data_device_selection(void *data, struct wl_data_device *device,
		struct wl_data_offer *offer) {
	struct wlf_wl_clipboard *clipboard = data;

	(void)device;

	if (clipboard->clipboard_offer != NULL) {
		wl_data_offer_destroy(clipboard->clipboard_offer);
	}
	wlf_wl_mime_type_list_clear(&clipboard->clipboard_mime_types);

	clipboard->clipboard_offer = offer;

	if (offer != NULL) {
		wl_data_offer_add_listener(offer, &data_offer_listener,
			&clipboard->clipboard_mime_types);
	}

	wlf_signal_emit_mutable(&clipboard->base.events.changed, NULL);
}

static void wlf_wl_clipboard_impl_destroy(struct wlf_clipboard *clipboard) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);

	wlf_wl_clipboard_data_clear(&wl_clipboard->clipboard_data);
	wlf_wl_clipboard_data_clear(&wl_clipboard->selection_data);

	if (wl_clipboard->clipboard_offer != NULL) {
		wl_data_offer_destroy(wl_clipboard->clipboard_offer);
		wl_clipboard->clipboard_offer = NULL;
	}

	if (wl_clipboard->selection_offer != NULL) {
		wl_data_offer_destroy(wl_clipboard->selection_offer);
		wl_clipboard->selection_offer = NULL;
	}

	wlf_wl_mime_type_list_clear(&wl_clipboard->clipboard_mime_types);
	wlf_wl_mime_type_list_clear(&wl_clipboard->selection_mime_types);

	if (wl_clipboard->data_device != NULL) {
		wl_data_device_destroy(wl_clipboard->data_device);
		wl_clipboard->data_device = NULL;
	}

	if (wl_clipboard->data_device_manager != NULL) {
		wl_data_device_manager_destroy(wl_clipboard->data_device_manager);
		wl_clipboard->data_device_manager = NULL;
	}
}

static bool wlf_wl_clipboard_impl_set_data(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type,
		const void *data, size_t size) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);
	struct wlf_wl_clipboard_data *clipboard_data;

	clipboard_data = mode == WLF_CLIPBOARD_MODE_CLIPBOARD ?
		&wl_clipboard->clipboard_data : &wl_clipboard->selection_data;

	if (!wlf_wl_clipboard_data_set(clipboard_data, mime_type, data, size)) {
		return false;
	}

	if (!wlf_wl_clipboard_data_ensure_source(wl_clipboard, clipboard_data)) {
		return false;
	}

	if (mode == WLF_CLIPBOARD_MODE_CLIPBOARD) {
		wl_data_device_set_selection(wl_clipboard->data_device,
			clipboard_data->source, 0);
	}

	return true;
}

static void *wlf_wl_clipboard_impl_get_data(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type, size_t *size) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);
	struct wlf_wl_clipboard_data *clipboard_data;
	struct wl_data_offer *offer;
	struct wlf_clipboard_data *entry;

	offer = mode == WLF_CLIPBOARD_MODE_CLIPBOARD ?
		wl_clipboard->clipboard_offer : wl_clipboard->selection_offer;

	if (offer != NULL) {
		return wlf_wl_offer_receive_data(wl_clipboard->backend, offer, mime_type, size);
	}

	clipboard_data = mode == WLF_CLIPBOARD_MODE_CLIPBOARD ?
		&wl_clipboard->clipboard_data : &wl_clipboard->selection_data;

	wlf_linked_list_for_each(entry, &clipboard_data->entries, link) {
		void *ret;

		if (strcmp(entry->mime_type, mime_type) != 0) {
			continue;
		}

		ret = malloc(entry->size);
		if (ret == NULL && entry->size > 0) {
			wlf_log_errno(WLF_ERROR, "failed to allocate clipboard readback buffer");
			return NULL;
		}

		if (entry->size > 0) {
			memcpy(ret, entry->data, entry->size);
		}
		if (size != NULL) {
			*size = entry->size;
		}

		return ret;
	}

	return NULL;
}

static char **wlf_wl_clipboard_impl_get_mime_types(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, size_t *count) {
	struct wlf_wl_clipboard *wl_clipboard = wlf_container_of(clipboard,
		wl_clipboard, base);
	struct wlf_linked_list *mime_types;
	struct wlf_wl_mime_type *mt;
	char **array;
	size_t i;

	mime_types = mode == WLF_CLIPBOARD_MODE_CLIPBOARD ?
		&wl_clipboard->clipboard_mime_types : &wl_clipboard->selection_mime_types;

	*count = 0;
	wlf_linked_list_for_each(mt, mime_types, link) {
		(*count)++;
	}

	if (*count == 0) {
		return NULL;
	}

	array = calloc(*count, sizeof(*array));
	if (array == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate mime type array");
		*count = 0;
		return NULL;
	}

	i = 0;
	wlf_linked_list_for_each(mt, mime_types, link) {
		array[i] = strdup(mt->mime_type);
		if (array[i] == NULL) {
			wlf_log_errno(WLF_ERROR, "failed to duplicate mime type");
			while (i > 0) {
				i--;
				free(array[i]);
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
	struct wlf_wl_clipboard_data *clipboard_data;

	clipboard_data = mode == WLF_CLIPBOARD_MODE_CLIPBOARD ?
		&wl_clipboard->clipboard_data : &wl_clipboard->selection_data;
	wlf_wl_clipboard_data_clear(clipboard_data);

	if (mode == WLF_CLIPBOARD_MODE_CLIPBOARD) {
		wl_data_device_set_selection(wl_clipboard->data_device, NULL, 0);
	}
}

struct wlf_wl_clipboard *wlf_wl_clipboard_create(
		struct wlf_backend_wayland *backend, struct wl_seat *seat) {
	struct wlf_wl_clipboard *wl_clipboard;
	struct wlf_wl_interface *interface;
	uint32_t bind_version;

	assert(backend != NULL);

	wl_clipboard = calloc(1, sizeof(*wl_clipboard));
	if (wl_clipboard == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_wl_clipboard");
		return NULL;
	}

	wlf_clipboard_init(&wl_clipboard->base, &wl_clipboard_impl);
	wl_clipboard->base.data = wl_clipboard;

	wl_clipboard->backend = backend;
	wl_clipboard->seat = seat != NULL ? seat : backend->seat;

	wlf_linked_list_init(&wl_clipboard->clipboard_data.entries);
	wlf_linked_list_init(&wl_clipboard->selection_data.entries);
	wlf_linked_list_init(&wl_clipboard->clipboard_mime_types);
	wlf_linked_list_init(&wl_clipboard->selection_mime_types);

	interface = wlf_wl_backend_find_interface(backend, "wl_data_device_manager");
	if (interface == NULL) {
		wlf_log(WLF_ERROR, "wl_data_device_manager is unavailable");
		free(wl_clipboard);
		return NULL;
	}

	if (wl_clipboard->seat == NULL) {
		wlf_log(WLF_ERROR, "wl_seat is required for clipboard/dnd");
		free(wl_clipboard);
		return NULL;
	}

	bind_version = interface->version;
	if (bind_version > (uint32_t)wl_data_device_manager_interface.version) {
		bind_version = (uint32_t)wl_data_device_manager_interface.version;
	}

	wl_clipboard->data_device_manager = wl_registry_bind(
		backend->registry,
		interface->name,
		&wl_data_device_manager_interface,
		bind_version);
	if (wl_clipboard->data_device_manager == NULL) {
		wlf_log(WLF_ERROR, "failed to bind wl_data_device_manager");
		free(wl_clipboard);
		return NULL;
	}

	wl_clipboard->data_device = wl_data_device_manager_get_data_device(
		wl_clipboard->data_device_manager,
		wl_clipboard->seat);
	if (wl_clipboard->data_device == NULL) {
		wlf_log(WLF_ERROR, "failed to create wl_data_device");
		wl_data_device_manager_destroy(wl_clipboard->data_device_manager);
		free(wl_clipboard);
		return NULL;
	}

	wl_data_device_add_listener(wl_clipboard->data_device,
		&data_device_listener, wl_clipboard);

	return wl_clipboard;
}

void wlf_wl_clipboard_destroy(struct wlf_wl_clipboard *clipboard) {
	if (clipboard == NULL) {
		return;
	}

	wlf_clipboard_destroy(&clipboard->base);
}
