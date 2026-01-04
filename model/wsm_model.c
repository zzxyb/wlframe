#include "wlf/model/wsm_model.h"

#include <stdlib.h>

bool wsm_model_index_is_valid(struct wsm_model_index *index) {
	if (index == NULL) {
		return false;
	}

	if (index->model == NULL) {
		return false;
	}

	if (index->row < 0 || index->column < 0) {
		return false;
	}

	if (index->internal.ptr == NULL && index->internal.id == 0) {
		return false;
	}

	return true;
}

struct wsm_model_index *wsm_model_index_sibling(struct wsm_model_index *self, int row, int column) {
	if (!wsm_model_index_is_valid(self)) {
		return NULL;
	}

	if (row < 0 || column < 0) {
		return NULL;
	}

	struct wsm_model_index *idx = malloc(sizeof(*idx));
	if (!idx) {
		return NULL;
	}

	*idx = *self;
	idx->row = row;
	idx->column = column;

	return idx;
}

struct wsm_model_index *wsm_model_index_sibling_at_column(struct wsm_model_index *self, int column) {
	if (!wsm_model_index_is_valid(self)) {
		return NULL;
	}

	return wsm_model_index_sibling(self, self->row, column);
}

struct wsm_model_index *wsm_model_index_sibling_at_row(struct wsm_model_index *self, int row) {
	if (!wsm_model_index_is_valid(self)) {
		return NULL;
	}

	return wsm_model_index_sibling(self, row, self->column);
}

bool wsm_model_index_equal(const struct wsm_model_index *a,
		const struct wsm_model_index *b) {
	if (a == b) {
		return true;
	}

	if (a == NULL || b == NULL) {
		return false;
	}

	return a->row == b->row &&
		a->column == b->column &&
		a->parent == b->parent &&
		a->model == b->model &&
		a->internal.ptr == b->internal.ptr;
}

bool wsm_model_index_less(const struct wsm_model_index *a,
		const struct wsm_model_index *b) {
	if (a == b) {
		return false;
	}

	if (a == NULL) {
		return true;
	}
	if (b == NULL) {
		return false;
	}

	if (a->model != b->model) {
		return a->model < b->model;
	}

	if (a->parent != b->parent) {
		return a->parent < b->parent;
	}

	if (a->row != b->row) {
		return a->row < b->row;
	}

	if (a->column != b->column) {
		return a->column < b->column;
	}

	return a->internal.ptr < b->internal.ptr;
}
