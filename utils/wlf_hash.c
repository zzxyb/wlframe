#include "wlf/utils/wlf_hash.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

#define WLF_HASH_INITIAL_CAPACITY 16
#define WLF_HASH_MAX_LOAD_FACTOR 0.75
#define WLF_HASH_MIN_LOAD_FACTOR 0.25


/**
 * @brief Resize and rehash the hash table.
 */
static bool resize_hash(struct wlf_hash *hash, size_t new_capacity) {
	struct wlf_hash_bucket *old_buckets = hash->buckets;
	size_t old_capacity = hash->capacity;

	struct wlf_hash_bucket *new_buckets = calloc(new_capacity,
	                                               sizeof(struct wlf_hash_bucket));
	if (!new_buckets) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate new hash buckets");
		return false;
	}

	for (size_t i = 0; i < new_capacity; i++) {
		new_buckets[i].state = WLF_HASH_EMPTY;
	}

	hash->buckets = new_buckets;
	hash->capacity = new_capacity;
	hash->size = 0;
	hash->deleted = 0;

	for (size_t i = 0; i < old_capacity; i++) {
		if (old_buckets[i].state == WLF_HASH_OCCUPIED) {
			uint32_t hash_value = old_buckets[i].hash;
			size_t index = hash_value % new_capacity;

			while (new_buckets[index].state == WLF_HASH_OCCUPIED) {
				index = (index + 1) % new_capacity;
			}

			new_buckets[index].key = old_buckets[i].key;
			new_buckets[index].value = old_buckets[i].value;
			new_buckets[index].hash = hash_value;
			new_buckets[index].state = WLF_HASH_OCCUPIED;
			hash->size++;
		}
	}

	free(old_buckets);
	return true;
}

/**
 * @brief Check if resize is needed and perform it.
 */
static bool check_and_resize(struct wlf_hash *hash) {
	double load_factor = (double)(hash->size + hash->deleted) / hash->capacity;

	if (load_factor > WLF_HASH_MAX_LOAD_FACTOR) {
		size_t new_capacity = hash->capacity * 2;
		return resize_hash(hash, new_capacity);
	}

	return true;
}

uint32_t wlf_hash_string(const char *str) {
	if (!str) {
		return 0;
	}

	uint32_t hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}

	return hash;
}

uint32_t wlf_hash_int(const void *key) {
	int value = *(const int *)key;

	uint32_t hash = (uint32_t)value;
	hash = (hash ^ 61) ^ (hash >> 16);
	hash = hash + (hash << 3);
	hash = hash ^ (hash >> 4);
	hash = hash * 0x27d4eb2d;
	hash = hash ^ (hash >> 15);

	return hash;
}

uint32_t wlf_hash_ptr(const void *key) {
	uintptr_t value = (uintptr_t)key;

	value = (value ^ (value >> 32)) * 0x9e3779b97f4a7c15ULL;
	value = (value ^ (value >> 32)) * 0x9e3779b97f4a7c15ULL;
	value = value ^ (value >> 32);

	return (uint32_t)value;
}

uint32_t wlf_hash_bytes(const void *data, size_t len) {
	if (!data || len == 0) {
		return 0;
	}

	uint32_t hash = 2166136261u;
	const uint8_t *bytes = data;

	for (size_t i = 0; i < len; i++) {
		hash ^= bytes[i];
		hash *= 16777619u;
	}

	return hash;
}

struct wlf_hash *wlf_hash_create(wlf_hash_func_t hash_func,
                                 wlf_hash_compare_func_t compare) {
	if (!hash_func || !compare) {
		wlf_log(WLF_ERROR, "Hash function and compare function cannot be NULL");
		return NULL;
	}

	struct wlf_hash *hash = calloc(1, sizeof(struct wlf_hash));
	if (!hash) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate hash table");
		return NULL;
	}

	hash->capacity = WLF_HASH_INITIAL_CAPACITY;
	hash->buckets = calloc(hash->capacity, sizeof(struct wlf_hash_bucket));
	if (!hash->buckets) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate hash buckets");
		free(hash);
		return NULL;
	}

	for (size_t i = 0; i < hash->capacity; i++) {
		hash->buckets[i].state = WLF_HASH_EMPTY;
	}

	hash->size = 0;
	hash->deleted = 0;
	hash->hash_func = hash_func;
	hash->compare = compare;

	return hash;
}

void wlf_hash_destroy(struct wlf_hash *hash,
                      wlf_hash_destroy_key_func_t destroy_key,
                      wlf_hash_destroy_value_func_t destroy_value) {
	if (!hash) {
		return;
	}

	if (destroy_key || destroy_value) {
		for (size_t i = 0; i < hash->capacity; i++) {
			if (hash->buckets[i].state == WLF_HASH_OCCUPIED) {
				if (destroy_key) {
					destroy_key(hash->buckets[i].key);
				}
				if (destroy_value) {
					destroy_value(hash->buckets[i].value);
				}
			}
		}
	}

	free(hash->buckets);
	free(hash);
}

bool wlf_hash_insert(struct wlf_hash *hash, void *key, void *value) {
	if (!hash || !key) {
		return false;
	}

	if (!check_and_resize(hash)) {
		return false;
	}

	uint32_t hash_value = hash->hash_func(key);
	size_t index = hash_value % hash->capacity;
	size_t start_index = index;

	while (true) {
		struct wlf_hash_bucket *bucket = &hash->buckets[index];

		if (bucket->state == WLF_HASH_EMPTY || bucket->state == WLF_HASH_DELETED) {
			if (bucket->state == WLF_HASH_DELETED) {
				hash->deleted--;
			}
			bucket->key = key;
			bucket->value = value;
			bucket->hash = hash_value;
			bucket->state = WLF_HASH_OCCUPIED;
			hash->size++;
			return true;
		} else if (bucket->hash == hash_value &&
		           hash->compare(bucket->key, key) == 0) {
			bucket->value = value;
			return true;
		}

		index = (index + 1) % hash->capacity;

		if (index == start_index) {
			wlf_log(WLF_ERROR, "Hash table is full (shouldn't happen)");
			return false;
		}
	}
}

bool wlf_hash_remove(struct wlf_hash *hash, void *key,
                     wlf_hash_destroy_key_func_t destroy_key,
                     wlf_hash_destroy_value_func_t destroy_value) {
	if (!hash || !key) {
		return false;
	}

	uint32_t hash_value = hash->hash_func(key);
	size_t index = hash_value % hash->capacity;
	size_t start_index = index;

	while (true) {
		struct wlf_hash_bucket *bucket = &hash->buckets[index];

		if (bucket->state == WLF_HASH_EMPTY) {
			return false; // Key not found
		} else if (bucket->state == WLF_HASH_OCCUPIED &&
		           bucket->hash == hash_value &&
		           hash->compare(bucket->key, key) == 0) {
			if (destroy_key) {
				destroy_key(bucket->key);
			}
			if (destroy_value) {
				destroy_value(bucket->value);
			}

			bucket->state = WLF_HASH_DELETED;
			bucket->key = NULL;
			bucket->value = NULL;
			hash->size--;
			hash->deleted++;
			return true;
		}

		index = (index + 1) % hash->capacity;

		if (index == start_index) {
			return false; // Wrapped around, key not found
		}
	}
}

void *wlf_hash_find(struct wlf_hash *hash, const void *key) {
	if (!hash || !key) {
		return NULL;
	}

	uint32_t hash_value = hash->hash_func(key);
	size_t index = hash_value % hash->capacity;
	size_t start_index = index;

	while (true) {
		struct wlf_hash_bucket *bucket = &hash->buckets[index];

		if (bucket->state == WLF_HASH_EMPTY) {
			return NULL; // Key not found
		} else if (bucket->state == WLF_HASH_OCCUPIED &&
		           bucket->hash == hash_value &&
		           hash->compare(bucket->key, key) == 0) {
			return bucket->value;
		}

		index = (index + 1) % hash->capacity;

		if (index == start_index) {
			return NULL; // Wrapped around
		}
	}
}

bool wlf_hash_contains(struct wlf_hash *hash, const void *key) {
	return wlf_hash_find(hash, key) != NULL;
}

size_t wlf_hash_size(struct wlf_hash *hash) {
	return hash ? hash->size : 0;
}

bool wlf_hash_is_empty(struct wlf_hash *hash) {
	return !hash || hash->size == 0;
}

void wlf_hash_clear(struct wlf_hash *hash,
                    wlf_hash_destroy_key_func_t destroy_key,
                    wlf_hash_destroy_value_func_t destroy_value) {
	if (!hash) {
		return;
	}

	if (destroy_key || destroy_value) {
		for (size_t i = 0; i < hash->capacity; i++) {
			if (hash->buckets[i].state == WLF_HASH_OCCUPIED) {
				if (destroy_key) {
					destroy_key(hash->buckets[i].key);
				}
				if (destroy_value) {
					destroy_value(hash->buckets[i].value);
				}
			}
		}
	}

	for (size_t i = 0; i < hash->capacity; i++) {
		hash->buckets[i].state = WLF_HASH_EMPTY;
		hash->buckets[i].key = NULL;
		hash->buckets[i].value = NULL;
	}

	hash->size = 0;
	hash->deleted = 0;
}

void wlf_hash_foreach(struct wlf_hash *hash,
                      wlf_hash_foreach_func_t func,
                      void *user_data) {
	if (!hash || !func) {
		return;
	}

	for (size_t i = 0; i < hash->capacity; i++) {
		if (hash->buckets[i].state == WLF_HASH_OCCUPIED) {
			if (!func(hash->buckets[i].key, hash->buckets[i].value, user_data)) {
				break;
			}
		}
	}
}

double wlf_hash_load_factor(struct wlf_hash *hash) {
	if (!hash || hash->capacity == 0) {
		return 0.0;
	}
	return (double)hash->size / hash->capacity;
}

struct wlf_hash_iterator wlf_hash_iterator_create(struct wlf_hash *hash) {
	struct wlf_hash_iterator it = {
		.hash = hash,
		.index = 0
	};

	if (hash) {
		while (it.index < hash->capacity &&
		       hash->buckets[it.index].state != WLF_HASH_OCCUPIED) {
			it.index++;
		}
	}

	return it;
}

bool wlf_hash_iterator_has_next(struct wlf_hash_iterator *it) {
	return it && it->hash && it->index < it->hash->capacity;
}

void wlf_hash_iterator_next(struct wlf_hash_iterator *it) {
	if (!it || !it->hash) {
		return;
	}

	it->index++;

	while (it->index < it->hash->capacity &&
	       it->hash->buckets[it->index].state != WLF_HASH_OCCUPIED) {
		it->index++;
	}
}

void *wlf_hash_iterator_key(struct wlf_hash_iterator *it) {
	if (!it || !it->hash || it->index >= it->hash->capacity) {
		return NULL;
	}

	if (it->hash->buckets[it->index].state == WLF_HASH_OCCUPIED) {
		return it->hash->buckets[it->index].key;
	}

	return NULL;
}

void *wlf_hash_iterator_value(struct wlf_hash_iterator *it) {
	if (!it || !it->hash || it->index >= it->hash->capacity) {
		return NULL;
	}

	if (it->hash->buckets[it->index].state == WLF_HASH_OCCUPIED) {
		return it->hash->buckets[it->index].value;
	}

	return NULL;
}
