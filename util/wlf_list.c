#include "wlf/util/wlf_list.h"
#include "wlf/util/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct wlf_list *wlf_list_create(void) {
	struct wlf_list *list = malloc(sizeof(struct wlf_list));
	if (!list) {
		return NULL;
	}
	list->capacity = 10;
	list->length = 0;
	list->items = malloc(sizeof(void*) * list->capacity);

	return list;
}

static void list_resize(struct wlf_list *list) {
	if (list->length == list->capacity) {
		list->capacity *= 2;
		list->items = realloc(list->items, sizeof(void*) * list->capacity);
	}
}

void wlf_list_destroy(struct wlf_list *list) {
	if (list == NULL) {
		return;
	}
	free(list->items);
	free(list);
}

void wlf_list_add(struct wlf_list *list, void *item) {
	list_resize(list);
	list->items[list->length++] = item;
}

void wlf_list_insert(struct wlf_list *list, int index, void *item) {
	list_resize(list);
	memmove(&list->items[index + 1], &list->items[index],
		sizeof(void*) * (list->length - index));
	list->length++;
	list->items[index] = item;
}

void wlf_list_delete(struct wlf_list *list, int index) {
	list->length--;
	memmove(&list->items[index], &list->items[index + 1],
		sizeof(void*) * (list->length - index));
}

void wlf_list_cat(struct wlf_list *list, struct wlf_list *source) {
	for (int i = 0; i < source->length; ++i) {
		wlf_list_add(list, source->items[i]);
	}
}

void wlf_list_qsort(struct wlf_list *list, int compare(const void *left, const void *right)) {
	qsort(list->items, list->length, sizeof(void *), compare);
}

int wlf_list_seq_find(struct wlf_list *list,
		int compare(const void *item, const void *data), const void *data) {
	for (int i = 0; i < list->length; i++) {
		void *item = list->items[i];
		if (compare(item, data) == 0) {
			return i;
		}
	}

	return -1;
}

int wlf_list_find(struct wlf_list *list, const void *item) {
	for (int i = 0; i < list->length; i++) {
		if (list->items[i] == item) {
			return i;
		}
	}

	return -1;
}

void wlf_list_swap(struct wlf_list *list, int src, int dest) {
	void *tmp = list->items[src];
	list->items[src] = list->items[dest];
	list->items[dest] = tmp;
}

void wlf_list_move_to_end(struct wlf_list *list, void *item) {
	int i;
	for (i = 0; i < list->length; ++i) {
		if (list->items[i] == item) {
			break;
		}
	}
	if (!wlf_assert(i < list->length, "Item not found in list")) {
		return;
	}
	wlf_list_delete(list, i);
	wlf_list_add(list, item);
}

static void list_rotate(struct wlf_list *list, int from, int to) {
	void *tmp = list->items[to];

	while (to > from) {
		list->items[to] = list->items[to - 1];
		to--;
	}

	list->items[from] = tmp;
}

static void list_inplace_merge(struct wlf_list *list, int left,
		int last, int mid, int compare(const void *a, const void *b)) {
	int right = mid + 1;

	if (compare(&list->items[mid], &list->items[right]) <= 0) {
		return;
	}

	while (left <= mid && right <= last) {
		if (compare(&list->items[left], &list->items[right]) <= 0) {
			left++;
		} else {
			list_rotate(list, left, right);
			left++;
			mid++;
			right++;
		}
	}
}

static void list_inplace_sort(struct wlf_list *list, int first,
		int last, int compare(const void *a, const void *b)) {
	if (first >= last) {
		return;
	} else if ((last - first) == 1) {
		if (compare(&list->items[first],
					&list->items[last]) > 0) {
			wlf_list_swap(list, first, last);
		}
	} else {
		int mid = (int)((last + first) / 2);
		list_inplace_sort(list, first, mid, compare);
		list_inplace_sort(list, mid + 1, last, compare);
		list_inplace_merge(list, first, last, mid, compare);
	}
}

void wlf_list_stable_sort(struct wlf_list *list,
		int compare(const void *a, const void *b)) {
	if (list->length > 1) {
		list_inplace_sort(list, 0, list->length - 1, compare);
	}
}

void wlf_list_free_items_and_destroy(struct wlf_list *list) {
	if (!list) {
		wlf_log(WLF_ERROR, "wlf_list is NULL!");
		return;
	}

	for (int i = 0; i < list->length; ++i) {
		free(list->items[i]);
	}
	wlf_list_destroy(list);
}
