#ifndef MODEL_WLF_MODEL_H
#define MODEL_WLF_MODEL_H

#include <stdint.h>
#include <stdbool.h>

struct wsm_model;
struct wsm_model_index;

struct wsm_modele_impl {
	struct wsm_model_index *(*index)(struct wsm_model *model,
			int row, int column, const struct wsm_model_index *parent);
	struct wsm_model_index *(*parent)(struct wsm_model *model, const struct wsm_model_index *child);

	int (*row_count)(struct wsm_model *model, const struct wsm_model_index *parent);
	int (*column_count)(struct wsm_model *model, const struct wsm_model_index *parent);

	void *(*data)(struct wsm_model *model, const struct wsm_model_index *index, uint32_t role);
	void *(*set_data)(struct wsm_model *model, const struct wsm_model_index *index, void *value, uint32_t role);
	void (*destroy)(struct wsm_model *model);
};

struct wsm_model {
	const struct wsm_mode_impl *impl;
};

struct wsm_model_index {
	int row;
	int colum;

	union {
		void *ptr;
		uintptr_t id;
	} internal;

	const struct wsm_model *model;
};

#endif // MODEL_WLF_MODEL_H
