#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>

void wlf_linked_list_init(struct wlf_linked_list *list) {
	list->prev = list;
	list->next = list;
}

void wlf_linked_list_insert(struct wlf_linked_list *list, struct wlf_linked_list *elm) {
	elm->prev = list;
	elm->next = list->next;
	list->next = elm;
	elm->next->prev = elm;
}

void wlf_linked_list_remove(struct wlf_linked_list *elm) {
	elm->prev->next = elm->next;
	elm->next->prev = elm->prev;
	elm->next = NULL;
	elm->prev = NULL;
}

int wlf_linked_list_length(const struct wlf_linked_list *list) {
	const struct wlf_linked_list *e;
	int count;

	count = 0;
	e = list->next;
	while (e != list) {
		e = e->next;
		count++;
	}

	return count;
}

int wlf_linked_list_empty(const struct wlf_linked_list *list) {
	return list->next == list;
}

void wlf_linked_list_insert_list(struct wlf_linked_list *list, struct wlf_linked_list *other) {
	if (wlf_linked_list_empty(other)) {
		return;
	}

	other->next->prev = list;
	other->prev->next = list->next;
	list->next->prev = other->prev;
	list->next = other->next;
}
