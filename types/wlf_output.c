#include "wlf/types/wlf_output.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <stdlib.h>

void wlf_output_init(struct wlf_output *output,
		const struct wlf_output_impl *impl) {
	assert(impl->destroy);

	*output = (struct wlf_output){
		.impl = impl,
	};

	wlf_signal_init(&output->events.destroy);
	wlf_signal_init(&output->events.name_changed);
	wlf_signal_init(&output->events.model_changed);
	wlf_signal_init(&output->events.manufacturer_changed);
	wlf_signal_init(&output->events.description_changed);

	wlf_signal_init(&output->events.geometry_changed);
	wlf_signal_init(&output->events.physical_size_changed);
	wlf_signal_init(&output->events.refresh_rate_changed);
	wlf_signal_init(&output->events.scale_changed);
	wlf_signal_init(&output->events.transform_changed);
	wlf_signal_init(&output->events.subpixel_changed);
}

void wlf_output_destroy(struct wlf_output *output) {
	if (!output) {
		return;
	}

	wlf_signal_emit_mutable(&output->events.destroy, output);

	assert(wlf_linked_list_empty(&output->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&output->events.name_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.model_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.manufacturer_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.description_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.geometry_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.physical_size_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.refresh_rate_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.scale_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.transform_changed.listener_list));
	assert(wlf_linked_list_empty(&output->events.subpixel_changed.listener_list));

	if (output->name != NULL) {
		free(output->name);
	}

	if (output->model != NULL) {
		free(output->model);
	}

	if (output->manufacturer != NULL) {
		free(output->manufacturer);
	}

	if (output->description != NULL) {
		free(output->description);
	}

	if (output->geometry != NULL) {
		free(output->geometry);
	}

	if (output->physical_size != NULL) {
		free(output->physical_size);
	}

	if (output->impl && output->impl->destroy) {
		output->impl->destroy(output);
	} else {
		free(output);
	}
}
