#include "wlf/utils/wlf_array.h"

#include <stdlib.h>
#include <string.h>

#define WLF_ARRAY_POISON_PTR (void *) 4

void wlf_array_init(struct wlf_array *array) {
	memset(array, 0, sizeof *array);
}

void wlf_array_release(struct wlf_array *array)
{
	free(array->data);
	array->data = WLF_ARRAY_POISON_PTR;
}

void *wlf_array_add(struct wlf_array *array, size_t size) {
	size_t alloc;
	void *data, *p;

	if (array->alloc > 0)
		alloc = array->alloc;
	else
		alloc = 16;

	while (alloc < array->size + size)
		alloc *= 2;

	if (array->alloc < alloc) {
		if (array->alloc > 0)
			data = realloc(array->data, alloc);
		else
			data = malloc(alloc);

		if (data == NULL)
			return NULL;
		array->data = data;
		array->alloc = alloc;
	}

	p = (char *)array->data + array->size;
	array->size += size;

	return p;
}

int wlf_array_copy(struct wlf_array *array, struct wlf_array *source) {
	if (array->size < source->size) {
		if (!wlf_array_add(array, source->size - array->size))
			return -1;
	} else {
		array->size = source->size;
	}

	if (source->size > 0)
		memcpy(array->data, source->data, source->size);

	return 0;
}
