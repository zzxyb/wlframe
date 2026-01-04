#ifndef MODEL_WLF_MODEL_H
#define MODEL_WLF_MODEL_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_list.h"

#include <stdint.h>
#include <stdbool.h>

struct wsm_model;
struct wsm_model_index;

struct wsm_model_impl {
	struct wsm_model_index *(*index)(struct wsm_model *model,
			int row, int column, const struct wsm_model_index *parent);
	struct wsm_model_index *(*parent)(struct wsm_model *model, const struct wsm_model_index *child);

	int (*row_count)(struct wsm_model *model, const struct wsm_model_index *parent);
	int (*column_count)(struct wsm_model *model, const struct wsm_model_index *parent);

	void *(*data)(struct wsm_model *model, const struct wsm_model_index *index, uint32_t role);
	void *(*set_data)(struct wsm_model *model, const struct wsm_model_index *index, void *value, uint32_t role);
	void (*destroy)(struct wsm_model *model);
};

struct wsm_model_roles {
	const int *data;
	size_t size;
};

struct wsm_model_range_event {
	struct wsm_model *model;
	struct wsm_model_index *parent;
	int first, last;
};

struct wsm_model_move_event {
	struct wsm_model *model;
	struct wsm_model_index *src_parent;
	struct wsm_model_index *dst_parent;
	int src_first, src_last, dst_pos;
};

struct wsm_model_data_changed_event {
	struct wsm_model *model;
	struct wsm_model_index *top_left;
	struct wsm_model_index *bottom_right;
	struct wsm_model_roles;
};

struct wsm_model {
	const struct wsm_model_impl *impl;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal columns_about_to_be_inserted;
		struct wlf_signal columns_about_to_be_moved;
		struct wlf_signal columns_about_to_be_removed;
		struct wlf_signal columns_inserted;
		struct wlf_signal columns_moved;
		struct wlf_signal columns_removed;
		struct wlf_signal data_changed;
		struct wlf_signal about_to_be_reset;
		struct wlf_signal reset;
		struct wlf_signal rows_about_to_be_inserted;
		struct wlf_signal rows_about_to_be_moved;
		struct wlf_signal rows_about_to_be_removed;
		struct wlf_signal rows_inserted;
		struct wlf_signal rows_moved;
		struct wlf_signal rows_removed;
	} events;
};

void wsm_model_init(struct wsm_model *model, const struct wsm_model_impl *impl);
void wsm_model_destory(struct wsm_model *model);
void wsm_modele_begin_insert_columns(const struct wsm_model_index *parent,
	int first,
	int last);
void wsm_model_begin_insert_rows(const struct wsm_model_index *parent,
	int first,
	int last);
bool wsm_model_begin_move_columns(const struct wsm_model_index *src_parent,
	int src_first,
	int src_last,
	const struct wsm_model_index *dst_parent,
	int dst_child);
bool wsm_model_begin_move_rows(const struct wsm_model_index *src_parent,
	int src_first,
	int src_last,
	const struct wsm_model_index *dst_parent,
	int dst_child);
void wsm_model_begin_remove_columns(const struct wsm_model_index *parent, int first, int last);
void wsm_model_begin_remove_rows(const struct wsm_model_index *parent, int first, int last);
void wsm_model_begin_reset_model();
void wsm_model_change_persistent_index(const struct wsm_model_index *from, const struct wsm_model_index *to);
void wsm_model_change_persistent_index_list(const struct wlf_list *from, const struct wlf_list *to);
const struct wsm_model_index *wsm_model_create_index_from_ptr(int row, int column, const void *ptr);
const struct wsm_model_index *wsm_model_create_index_from_id(int row, int column, uintptr_t id);
void wsm_model_end_insert_columns();
void wsm_model_end_insert_rows();
void wsm_model_end_move_columns();
void wsm_model_end_move_rows();
void wsm_model_end_remove_columns();
void wsm_model_end_remove_rows();
void wsm_model_end_reset_model();
const struct wlf_list *persistentIndexList();

struct wsm_model_index {
	int row;
	int column;

	union {
		void *ptr;
		uintptr_t id;
	} internal;

	struct wsm_model_index *parent;
	const struct wsm_model *model;
};

bool wsm_model_index_is_valid(struct wsm_model_index *index);
struct wsm_model_index *wsm_model_index_sibling(struct wsm_model_index *self, int row, int column);
struct wsm_model_index *wsm_model_index_sibling_at_column(struct wsm_model_index *self, int column);
struct wsm_model_index *wsm_model_index_sibling_at_row(struct wsm_model_index *self, int row);
bool wsm_model_index_equal(const struct wsm_model_index *a,
	const struct wsm_model_index *b);
bool wsm_model_index_less(const struct wsm_model_index *a,
	const struct wsm_model_index *b);

#endif // MODEL_WLF_MODEL_H
