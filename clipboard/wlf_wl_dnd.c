#include "wlf/clipboard/wlf_wl_dnd.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_interface.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct wlf_wl_dnd_mime_type {
	char *mime_type;
	struct wlf_linked_list link;
};

struct wlf_wl_dnd_data_entry {
	char *mime_type;
	void *data;
	size_t size;
	struct wlf_linked_list link;
};

static void dnd_mime_type_list_clear(struct wlf_linked_list *list);
static void dnd_source_data_clear(struct wlf_wl_dnd_data *source_data);
static bool dnd_source_data_set(struct wlf_wl_dnd_data *source_data,
	const char *mime_type, const void *data, size_t size);
static bool dnd_source_data_ensure_source(struct wlf_wl_dnd *dnd,
	struct wlf_wl_dnd_data *source_data);
static void *dnd_offer_receive_data(struct wlf_backend_wayland *backend,
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
static void data_device_enter(void *data, struct wl_data_device *device,
	uint32_t serial, struct wl_surface *surface, wl_fixed_t x,
	wl_fixed_t y, struct wl_data_offer *offer);
static void data_device_leave(void *data, struct wl_data_device *device);
static void data_device_motion(void *data, struct wl_data_device *device,
	uint32_t time, wl_fixed_t x, wl_fixed_t y);
static void data_device_drop(void *data, struct wl_data_device *device);
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
	.enter = data_device_enter,
	.leave = data_device_leave,
	.motion = data_device_motion,
	.drop = data_device_drop,
	.selection = data_device_selection,
};

static void dnd_mime_type_list_clear(struct wlf_linked_list *list) {
	struct wlf_wl_dnd_mime_type *mt, *tmp;
	wlf_linked_list_for_each_safe(mt, tmp, list, link) {
		wlf_linked_list_remove(&mt->link);
		free(mt->mime_type);
		free(mt);
	}
}

static void dnd_source_data_clear(struct wlf_wl_dnd_data *source_data) {
	struct wlf_wl_dnd_data_entry *entry, *tmp;

	if (source_data->source != NULL) {
		wl_data_source_destroy(source_data->source);
		source_data->source = NULL;
	}

	wlf_linked_list_for_each_safe(entry, tmp, &source_data->entries, link) {
		wlf_linked_list_remove(&entry->link);
		free(entry->mime_type);
		free(entry->data);
		free(entry);
	}
}

static bool dnd_source_data_set(struct wlf_wl_dnd_data *source_data,
		const char *mime_type, const void *data, size_t size) {
	struct wlf_wl_dnd_data_entry *entry;

	wlf_linked_list_for_each(entry, &source_data->entries, link) {
		if (strcmp(entry->mime_type, mime_type) == 0) {
			void *new_data = malloc(size);
			if (new_data == NULL && size > 0) {
				wlf_log_errno(WLF_ERROR, "failed to allocate dnd payload");
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
		wlf_log_errno(WLF_ERROR, "failed to allocate dnd data entry");
		return false;
	}

	entry->mime_type = strdup(mime_type);
	if (entry->mime_type == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to duplicate dnd mime type");
		free(entry);
		return false;
	}

	entry->data = malloc(size);
	if (entry->data == NULL && size > 0) {
		wlf_log_errno(WLF_ERROR, "failed to allocate dnd payload buffer");
		free(entry->mime_type);
		free(entry);
		return false;
	}

	if (size > 0) {
		memcpy(entry->data, data, size);
	}
	entry->size = size;

	wlf_linked_list_insert(&source_data->entries, &entry->link);
	return true;
}

static bool dnd_source_data_ensure_source(struct wlf_wl_dnd *dnd,
		struct wlf_wl_dnd_data *source_data) {
	struct wlf_wl_dnd_data_entry *entry;

	if (source_data->source != NULL) {
		return true;
	}

	source_data->source = wl_data_device_manager_create_data_source(
		dnd->data_device_manager);
	if (source_data->source == NULL) {
		wlf_log(WLF_ERROR, "failed to create dnd wl_data_source");
		return false;
	}

	wl_data_source_add_listener(source_data->source,
		&data_source_listener, source_data);

	wlf_linked_list_for_each(entry, &source_data->entries, link) {
		wl_data_source_offer(source_data->source, entry->mime_type);
	}

	return true;
}

static void *dnd_offer_receive_data(struct wlf_backend_wayland *backend,
		struct wl_data_offer *offer, const char *mime_type, size_t *size) {
	int fds[2] = {-1, -1};
	char *buffer = NULL;
	size_t capacity = 4096;
	size_t received = 0;

	if (pipe(fds) < 0) {
		wlf_log_errno(WLF_ERROR, "failed to create dnd receive pipe");
		return NULL;
	}

	wl_data_offer_receive(offer, mime_type, fds[1]);
	close(fds[1]);

	if (backend->display == NULL) {
		wlf_log(WLF_ERROR, "wayland display is not available");
		close(fds[0]);
		return NULL;
	}

	wl_display_roundtrip(backend->display);

	buffer = malloc(capacity);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate dnd receive buffer");
		close(fds[0]);
		return NULL;
	}

	while (true) {
		if (received == capacity) {
			size_t new_capacity = capacity * 2;
			char *new_buffer = realloc(buffer, new_capacity);
			if (new_buffer == NULL) {
				wlf_log_errno(WLF_ERROR, "failed to grow dnd receive buffer");
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

			wlf_log_errno(WLF_ERROR, "failed to read dnd data");
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
	(void)mime_type;
}

static void data_source_send(void *data, struct wl_data_source *source,
		const char *mime_type, int32_t fd) {
	struct wlf_wl_dnd_data *source_data = data;
	struct wlf_wl_dnd_data_entry *entry;

	(void)source;

	wlf_linked_list_for_each(entry, &source_data->entries, link) {
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
				wlf_log_errno(WLF_ERROR, "failed to write dnd source data");
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
	struct wlf_wl_dnd_data *source_data = data;

	if (source_data->source == source) {
		source_data->source = NULL;
	}
}

static void data_offer_offer(void *data, struct wl_data_offer *offer,
		const char *mime_type) {
	struct wlf_linked_list *mime_types = data;
	struct wlf_wl_dnd_mime_type *mt;

	(void)offer;

	if (mime_type == NULL) {
		return;
	}

	mt = calloc(1, sizeof(*mt));
	if (mt == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate dnd mime type entry");
		return;
	}

	mt->mime_type = strdup(mime_type);
	if (mt->mime_type == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to duplicate dnd mime type");
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

static void data_device_enter(void *data, struct wl_data_device *device,
		uint32_t serial, struct wl_surface *surface, wl_fixed_t x,
		wl_fixed_t y, struct wl_data_offer *offer) {
	struct wlf_wl_dnd *dnd = data;

	(void)device;
	(void)surface;

	if (dnd->offer != NULL) {
		wl_data_offer_destroy(dnd->offer);
	}
	dnd_mime_type_list_clear(&dnd->offer_mime_types);

	dnd->offer = offer;
	dnd->serial = serial;
	dnd->x = x;
	dnd->y = y;

	if (offer != NULL) {
		wl_data_offer_add_listener(offer, &data_offer_listener,
			&dnd->offer_mime_types);
	}

	wlf_signal_emit_mutable(&dnd->events.enter, NULL);
}

static void data_device_leave(void *data, struct wl_data_device *device) {
	struct wlf_wl_dnd *dnd = data;

	(void)device;

	if (dnd->offer != NULL) {
		wl_data_offer_destroy(dnd->offer);
		dnd->offer = NULL;
	}
	dnd_mime_type_list_clear(&dnd->offer_mime_types);
	wlf_signal_emit_mutable(&dnd->events.leave, NULL);
}

static void data_device_motion(void *data, struct wl_data_device *device,
		uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	struct wlf_wl_dnd *dnd = data;

	(void)device;
	(void)time;

	dnd->x = x;
	dnd->y = y;
	wlf_signal_emit_mutable(&dnd->events.motion, NULL);
}

static void data_device_drop(void *data, struct wl_data_device *device) {
	struct wlf_wl_dnd *dnd = data;

	(void)device;
	wlf_signal_emit_mutable(&dnd->events.drop, NULL);
}

static void data_device_selection(void *data, struct wl_data_device *device,
		struct wl_data_offer *offer) {
	(void)data;
	(void)device;
	(void)offer;
}

void wlf_wl_dnd_init(struct wlf_wl_dnd *dnd,
		struct wlf_backend_wayland *backend, struct wl_seat *seat) {
	assert(dnd != NULL);
	assert(backend != NULL);

	dnd->backend = backend;
	dnd->seat = seat != NULL ? seat : backend->seat;

	wlf_linked_list_init(&dnd->offer_mime_types);
	wlf_linked_list_init(&dnd->source_data.entries);
	wlf_signal_init(&dnd->events.enter);
	wlf_signal_init(&dnd->events.leave);
	wlf_signal_init(&dnd->events.motion);
	wlf_signal_init(&dnd->events.drop);
}

void wlf_wl_dnd_finish(struct wlf_wl_dnd *dnd) {
	if (dnd == NULL) {
		return;
	}

	dnd_source_data_clear(&dnd->source_data);

	if (dnd->offer != NULL) {
		wl_data_offer_destroy(dnd->offer);
		dnd->offer = NULL;
	}
	dnd_mime_type_list_clear(&dnd->offer_mime_types);

	if (dnd->data_device != NULL) {
		wl_data_device_destroy(dnd->data_device);
		dnd->data_device = NULL;
	}

	if (dnd->data_device_manager != NULL) {
		wl_data_device_manager_destroy(dnd->data_device_manager);
		dnd->data_device_manager = NULL;
	}
}

struct wlf_wl_dnd *wlf_wl_dnd_create(struct wlf_backend_wayland *backend,
		struct wl_seat *seat) {
	struct wlf_wl_dnd *dnd;
	struct wlf_wl_interface *interface;
	uint32_t bind_version;

	assert(backend != NULL);

	dnd = calloc(1, sizeof(*dnd));
	if (dnd == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_wl_dnd");
		return NULL;
	}

	wlf_wl_dnd_init(dnd, backend, seat);

	interface = wlf_wl_backend_find_interface(backend, "wl_data_device_manager");
	if (interface == NULL) {
		wlf_log(WLF_ERROR, "wl_data_device_manager is unavailable");
		wlf_wl_dnd_destroy(dnd);
		return NULL;
	}

	if (dnd->seat == NULL) {
		wlf_log(WLF_ERROR, "wl_seat is required for dnd");
		wlf_wl_dnd_destroy(dnd);
		return NULL;
	}

	bind_version = interface->version;
	if (bind_version > (uint32_t)wl_data_device_manager_interface.version) {
		bind_version = (uint32_t)wl_data_device_manager_interface.version;
	}

	dnd->data_device_manager = wl_registry_bind(
		backend->registry,
		interface->name,
		&wl_data_device_manager_interface,
		bind_version);
	if (dnd->data_device_manager == NULL) {
		wlf_log(WLF_ERROR, "failed to bind wl_data_device_manager for dnd");
		wlf_wl_dnd_destroy(dnd);
		return NULL;
	}

	dnd->data_device = wl_data_device_manager_get_data_device(
		dnd->data_device_manager,
		dnd->seat);
	if (dnd->data_device == NULL) {
		wlf_log(WLF_ERROR, "failed to create wl_data_device for dnd");
		wlf_wl_dnd_destroy(dnd);
		return NULL;
	}

	wl_data_device_add_listener(dnd->data_device, &data_device_listener, dnd);
	return dnd;
}

void wlf_wl_dnd_destroy(struct wlf_wl_dnd *dnd) {
	if (dnd == NULL) {
		return;
	}

	wlf_wl_dnd_finish(dnd);
	free(dnd);
}

char **wlf_wl_dnd_get_mime_types(struct wlf_wl_dnd *dnd, size_t *count) {
	struct wlf_wl_dnd_mime_type *mt;
	char **array;
	size_t i;

	assert(dnd != NULL);
	assert(count != NULL);

	*count = 0;
	wlf_linked_list_for_each(mt, &dnd->offer_mime_types, link) {
		(*count)++;
	}

	if (*count == 0) {
		return NULL;
	}

	array = calloc(*count, sizeof(*array));
	if (array == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate dnd mime type array");
		*count = 0;
		return NULL;
	}

	i = 0;
	wlf_linked_list_for_each(mt, &dnd->offer_mime_types, link) {
		array[i] = strdup(mt->mime_type);
		if (array[i] == NULL) {
			wlf_log_errno(WLF_ERROR, "failed to duplicate dnd mime type");
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

void *wlf_wl_dnd_get_data(struct wlf_wl_dnd *dnd,
		const char *mime_type, size_t *size) {
	assert(dnd != NULL);
	assert(mime_type != NULL);

	if (dnd->offer == NULL) {
		return NULL;
	}

	return dnd_offer_receive_data(dnd->backend, dnd->offer, mime_type, size);
}

bool wlf_wl_dnd_set_data(struct wlf_wl_dnd *dnd,
		const char *mime_type, const void *data, size_t size) {
	assert(dnd != NULL);
	assert(mime_type != NULL);
	assert(data != NULL || size == 0);

	if (!dnd_source_data_set(&dnd->source_data, mime_type, data, size)) {
		return false;
	}

	if (!dnd_source_data_ensure_source(dnd, &dnd->source_data)) {
		return false;
	}

	return true;
}

bool wlf_wl_dnd_start_drag(struct wlf_wl_dnd *dnd,
		struct wl_surface *origin, struct wl_surface *icon, uint32_t serial) {
	assert(dnd != NULL);
	assert(origin != NULL);

	if (!dnd_source_data_ensure_source(dnd, &dnd->source_data)) {
		return false;
	}

	if (dnd->source_data.source == NULL) {
		wlf_log(WLF_ERROR, "dnd source is unavailable");
		return false;
	}

	wl_data_device_start_drag(
		dnd->data_device,
		dnd->source_data.source,
		origin,
		icon,
		serial);

	return true;
}

void wlf_wl_dnd_clear_data(struct wlf_wl_dnd *dnd) {
	assert(dnd != NULL);
	dnd_source_data_clear(&dnd->source_data);
}
