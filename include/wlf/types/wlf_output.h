#ifndef TYPES_WLF_OUTPUT_H
#define TYPES_WLF_OUTPUT_H

#include "wlf/math/wlf_size.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_output;

enum wlf_output_transform {
	WLF_OUTPUT_TRANSFORM_NORMAL = 0,
	WLF_OUTPUT_TRANSFORM_90,
	WLF_OUTPUT_TRANSFORM_180,
	WLF_OUTPUT_TRANSFORM_270,
	WLF_OUTPUT_TRANSFORM_FLIPPED,
	WLF_OUTPUT_TRANSFORM_FLIPPED_90,
	WLF_OUTPUT_TRANSFORM_FLIPPED_180,
	WLF_OUTPUT_TRANSFORM_FLIPPED_270,
};

enum wlf_output_subpixel {
	WLF_OUTPUT_SUBPIXEL_UNKNOWN = 0,
	WLF_OUTPUT_SUBPIXEL_NONE,
	WLF_OUTPUT_SUBPIXEL_HORIZONTAL_RGB,
	WLF_OUTPUT_SUBPIXEL_HORIZONTAL_BGR,
	WLF_OUTPUT_SUBPIXEL_VERTICAL_RGB,
	WLF_OUTPUT_SUBPIXEL_VERTICAL_BGR,
};
enum wlf_output_type {
	WLF_OUTPUT = 0,
	WLF_OUTPUT_VIRTUAL,
};

struct wlf_output_impl {
	const char *name;
	enum wlf_output_type type;
	void (*destroy)(struct wlf_output *output);
};

struct wlf_output {
	const struct wlf_output_impl *impl;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal name_change;
		struct wlf_signal model_change;
		struct wlf_signal manufacturer_change;
		struct wlf_signal description_change;


		struct wlf_signal geometry_change;
		struct wlf_signal physical_size_change;
		struct wlf_signal refreshRate_change;
		struct wlf_signal scale_change;
		struct wlf_signal transform_change;
		struct wlf_signal subpixel_change;
	} events;

	char *name;
	char *model;
	char *manufacturer;
	char *description;

	struct wlf_rect geometry;
	struct wlf_size physical_size;

	int refreshRate;
	int scale;
	enum wlf_output_transform transform;
	enum wlf_output_subpixel subpixel;
};

void wlf_output_init(struct wlf_output *output,
	const struct wlf_output_impl *impl);
void wlf_output_destroy(struct wlf_output *output);

struct wlf_output_manager_impl {
	const char *name;
	void (*destroy)(struct wlf_output *output);
};

struct wlf_output_manager {
	const struct wlf_output_manager_impl *impl;
	struct {
		struct wlf_signal destroy;
		struct wlf_signal output_added;
		struct wlf_signal output_removed;
	} events;
};

void wlf_output_manager_init(struct wlf_output_manager *manager,
	const struct wlf_output_manager_impl *impl);
void wlf_output_manager_destroy(struct wlf_output_manager *manager);

#endif // TYPES_WLF_OUTPUT_H
