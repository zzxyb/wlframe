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
	wlf_signal_init(&output->events.name_change);
	wlf_signal_init(&output->events.model_change);
	wlf_signal_init(&output->events.manufacturer_change);
	wlf_signal_init(&output->events.description_change);

	wlf_signal_init(&output->events.geometry_change);
	wlf_signal_init(&output->events.physical_size_change);
	wlf_signal_init(&output->events.refreshRate_change);
	wlf_signal_init(&output->events.scale_change);
	wlf_signal_init(&output->events.transform_change);
	wlf_signal_init(&output->events.subpixel_change);
}

void wlf_output_destroy(struct wlf_output *output) {
	if (!output) {
		return;
	}

	wlf_signal_emit_mutable(&output->events.destroy, output);

	if (output->impl && output->impl->destroy) {
		output->impl->destroy(output);
	} else {
		free(output);
	}
}

void wlf_output_manager_init(struct wlf_output_manager *manager,
		const struct wlf_output_manager_impl *impl) {
	assert(impl->destroy);

	*manager = (struct wlf_output_manager){
		.impl = impl,
	};

	wlf_linked_list_init(&manager->outputs);

	wlf_signal_init(&manager->events.destroy);
	wlf_signal_init(&manager->events.output_added);
	wlf_signal_init(&manager->events.output_removed);
}

void wlf_output_manager_destroy(struct wlf_output_manager *manager) {
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);

	if (manager->impl && manager->impl->destroy) {
		manager->impl->destroy(manager);
	} else {
		free(manager);
	}
}
