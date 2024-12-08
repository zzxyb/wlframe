#include "wlf/seat/wlf_input_device.h"
#include "wlf/utils/wlf_signal.h"

#include <stdlib.h>
#include <string.h>

void wlf_input_device_init(struct wlf_input_device *device, enum wlf_input_device_type type,
		const char *name, enum wlf_input_device_capability capabilities) {
	*device = (struct wlf_input_device) {
		.type = type,
		.capabilities = capabilities,
		.name = name ? strdup(name) : NULL,
	};

	wlf_signal_init(&device->events.destroy);
}

void wlf_input_device_fnish(struct wlf_input_device *device) {
	free(device->name);
	wlf_signal_emit_mutable(&device->events.destroy, device);
	wlf_double_list_remove(&device->events.destroy.listener_list);
	free(device);
}
