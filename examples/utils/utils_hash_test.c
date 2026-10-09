/**
 * @file        utils_hash_test.c
 * @brief       Example and test program for wlf_hash.
 * @details     Demonstrates various usage patterns of the wlf_hash API,
 *              including string keys, integer keys, custom hash functions,
 *              and performance comparisons with wlf_map.
 * @author      YaoBing Xiao
 * @date        2025-10-21
 */

#include "wlf/utils/wlf_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

/* ========================================================================
 * Example 1: String keys with integer values
 * ======================================================================== */

static uint32_t string_hash_func(const void *key) {
	return wlf_hash_string((const char *)key);
}

static int string_compare(const void *a, const void *b) {
	return strcmp((const char *)a, (const char *)b);
}

static void example_string_keys(void) {
	printf("\n=== Example 1: String Keys with Integer Values ===\n");

	// Create hash table with string keys
	struct wlf_hash *hash = wlf_hash_create(string_hash_func, string_compare);
	assert(hash != NULL);

	// Insert word frequencies
	struct {
		const char *word;
		int count;
	} words[] = {
		{"hello", 5},
		{"world", 3},
		{"hash", 10},
		{"table", 7},
		{"example", 2},
	};

	printf("Inserting word frequencies:\n");
	for (size_t i = 0; i < sizeof(words) / sizeof(words[0]); i++) {
		int *count = malloc(sizeof(int));
		*count = words[i].count;

		bool ok = wlf_hash_insert(hash, (void *)words[i].word, count);
		assert(ok);
		printf("  '%s': %d\n", words[i].word, *count);
	}

	printf("\nHash table size: %zu\n", wlf_hash_size(hash));
	printf("Load factor: %.2f\n", wlf_hash_load_factor(hash));

	// Lookup operations
	printf("\nLookup operations:\n");
	int *found = wlf_hash_find(hash, "hash");
	printf("  'hash': %s\n", found ? "found" : "not found");
	if (found) {
		printf("    count = %d\n", *found);
	}

	found = wlf_hash_find(hash, "missing");
	printf("  'missing': %s\n", found ? "found" : "not found");

	// Update a value
	printf("\nUpdating 'hello' count to 15...\n");
	found = wlf_hash_find(hash, "hello");
	if (found) {
		*found = 15;
		printf("  New count: %d\n", *found);
	}

	// Iterate over hash table
	printf("\nAll entries (unordered):\n");
	struct wlf_hash_iterator it;
	wlf_hash_foreach_entry(hash, it) {
		const char *word = wlf_hash_iterator_key(&it);
		int *count = wlf_hash_iterator_value(&it);
		printf("  '%s': %d\n", word, *count);
	}

	// Clean up
	wlf_hash_destroy(hash, NULL, free);
	printf("\nHash table destroyed.\n");
}

/* ========================================================================
 * Example 2: Integer keys with string values
 * ======================================================================== */

static uint32_t int_hash_func(const void *key) {
	return wlf_hash_int(key);
}

static int int_compare(const void *a, const void *b) {
	return *(const int *)a - *(const int *)b;
}

static void example_int_keys(void) {
	printf("\n=== Example 2: Integer Keys with String Values ===\n");

	struct wlf_hash *hash = wlf_hash_create(int_hash_func, int_compare);
	assert(hash != NULL);

	// Insert employee IDs and names
	struct {
		int id;
		const char *name;
	} employees[] = {
		{1001, "Alice"},
		{1002, "Bob"},
		{1003, "Charlie"},
		{1004, "Diana"},
		{1005, "Eve"},
	};

	printf("Inserting employee records:\n");
	for (size_t i = 0; i < sizeof(employees) / sizeof(employees[0]); i++) {
		int *id = malloc(sizeof(int));
		*id = employees[i].id;

		bool ok = wlf_hash_insert(hash, id, (void *)employees[i].name);
		assert(ok);
		printf("  ID %d: %s\n", *id, employees[i].name);
	}

	// Lookup by ID
	printf("\nLookup operations:\n");
	int search_id = 1003;
	char *name = wlf_hash_find(hash, &search_id);
	printf("  Employee ID %d: %s\n", search_id, name ? name : "Not found");

	search_id = 9999;
	name = wlf_hash_find(hash, &search_id);
	printf("  Employee ID %d: %s\n", search_id, name ? name : "Not found");

	// Check contains
	printf("\nContains check:\n");
	search_id = 1002;
	printf("  Has ID %d: %s\n", search_id,
	       wlf_hash_contains(hash, &search_id) ? "yes" : "no");

	// Remove an entry
	printf("\nRemoving ID 1003...\n");
	int remove_id = 1003;
	bool removed = wlf_hash_remove(hash, &remove_id, free, NULL);
	printf("  Removed: %s\n", removed ? "yes" : "no");
	printf("  New size: %zu\n", wlf_hash_size(hash));

	// Clean up
	wlf_hash_destroy(hash, free, NULL);
	printf("\nHash table destroyed.\n");
}

/* ========================================================================
 * Example 3: Using foreach callback
 * ======================================================================== */

struct stats_data {
	int total_length;
	int count;
};

static bool count_lengths_callback(void *key, void *value, void *user_data) {
	(void)value; // Unused

	const char *str = key;
	struct stats_data *stats = user_data;

	stats->total_length += strlen(str);
	stats->count++;

	return true;
}

static void example_foreach_callback(void) {
	printf("\n=== Example 3: Using Foreach Callback ===\n");

	struct wlf_hash *hash = wlf_hash_create(string_hash_func, string_compare);
	assert(hash != NULL);

	// Insert some strings
	const char *strings[] = {
		"apple", "banana", "cherry", "date", "elderberry",
		"fig", "grape", "honeydew"
	};

	printf("Inserting strings:\n");
	for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
		int *index = malloc(sizeof(int));
		*index = (int)i;
		wlf_hash_insert(hash, (void *)strings[i], index);
		printf("  %s\n", strings[i]);
	}

	// Calculate statistics using foreach
	struct stats_data stats = {0, 0};
	wlf_hash_foreach(hash, count_lengths_callback, &stats);

	double avg_length = stats.count > 0 ?
	                    (double)stats.total_length / stats.count : 0.0;

	printf("\nStatistics:\n");
	printf("  Count: %d\n", stats.count);
	printf("  Total length: %d\n", stats.total_length);
	printf("  Average length: %.2f\n", avg_length);

	// Clean up
	wlf_hash_destroy(hash, NULL, free);
	printf("\nHash table destroyed.\n");
}

/* ========================================================================
 * Example 4: Pointer keys
 * ======================================================================== */

struct object {
	int id;
	char name[32];
};

static uint32_t ptr_hash_func(const void *key) {
	return wlf_hash_ptr(key);
}

static int ptr_compare(const void *a, const void *b) {
	return (a == b) ? 0 : 1;
}

static void example_ptr_keys(void) {
	printf("\n=== Example 4: Pointer Keys (Object Cache) ===\n");

	struct wlf_hash *hash = wlf_hash_create(ptr_hash_func, ptr_compare);
	assert(hash != NULL);

	// Create some objects
	struct object *obj1 = malloc(sizeof(struct object));
	obj1->id = 1;
	strcpy(obj1->name, "Object One");

	struct object *obj2 = malloc(sizeof(struct object));
	obj2->id = 2;
	strcpy(obj2->name, "Object Two");

	struct object *obj3 = malloc(sizeof(struct object));
	obj3->id = 3;
	strcpy(obj3->name, "Object Three");

	// Use object pointers as keys, store metadata as values
	printf("Caching object metadata:\n");

	int *meta1 = malloc(sizeof(int)); *meta1 = 100;
	int *meta2 = malloc(sizeof(int)); *meta2 = 200;
	int *meta3 = malloc(sizeof(int)); *meta3 = 300;

	wlf_hash_insert(hash, obj1, meta1);
	wlf_hash_insert(hash, obj2, meta2);
	wlf_hash_insert(hash, obj3, meta3);

	printf("  %p (%s): metadata = %d\n", (void *)obj1, obj1->name, *meta1);
	printf("  %p (%s): metadata = %d\n", (void *)obj2, obj2->name, *meta2);
	printf("  %p (%s): metadata = %d\n", (void *)obj3, obj3->name, *meta3);

	// Lookup by pointer
	printf("\nLookup by pointer:\n");
	int *found_meta = wlf_hash_find(hash, obj2);
	if (found_meta) {
		printf("  Found metadata for %s: %d\n", obj2->name, *found_meta);
	}

	// Clean up
	wlf_hash_destroy(hash, NULL, free);
	free(obj1);
	free(obj2);
	free(obj3);
	printf("\nHash table destroyed.\n");
}

/* ========================================================================
 * Example 5: Clear and resize operations
 * ======================================================================== */

static void example_clear_and_resize(void) {
	printf("\n=== Example 5: Clear and Resize Operations ===\n");

	struct wlf_hash *hash = wlf_hash_create(int_hash_func, int_compare);
	assert(hash != NULL);

	printf("Initial state:\n");
	printf("  Size: %zu, Load factor: %.2f\n",
	       wlf_hash_size(hash), wlf_hash_load_factor(hash));

	// Insert many entries to trigger resize
	printf("\nInserting 100 entries to trigger auto-resize...\n");
	for (int i = 0; i < 100; i++) {
		int *key = malloc(sizeof(int));
		int *value = malloc(sizeof(int));
		*key = i;
		*value = i * 10;
		wlf_hash_insert(hash, key, value);
	}

	printf("After insertion:\n");
	printf("  Size: %zu, Load factor: %.2f\n",
	       wlf_hash_size(hash), wlf_hash_load_factor(hash));

	// Clear the hash table
	printf("\nClearing hash table...\n");
	wlf_hash_clear(hash, free, free);

	printf("After clear:\n");
	printf("  Size: %zu, Load factor: %.2f\n",
	       wlf_hash_size(hash), wlf_hash_load_factor(hash));
	printf("  Empty: %s\n", wlf_hash_is_empty(hash) ? "yes" : "no");

	// Clean up
	wlf_hash_destroy(hash, NULL, NULL);
	printf("\nHash table destroyed.\n");
}

/* ========================================================================
 * Example 6: Performance comparison
 * ======================================================================== */

static void example_performance(void) {
	printf("\n=== Example 6: Performance Test ===\n");

	const int NUM_ENTRIES = 10000;
	struct wlf_hash *hash = wlf_hash_create(int_hash_func, int_compare);
	assert(hash != NULL);

	// Measure insertion time
	clock_t start = clock();
	for (int i = 0; i < NUM_ENTRIES; i++) {
		int *key = malloc(sizeof(int));
		int *value = malloc(sizeof(int));
		*key = i;
		*value = i * 2;
		wlf_hash_insert(hash, key, value);
	}
	clock_t end = clock();
	double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

	printf("Inserted %d entries in %.2f ms\n", NUM_ENTRIES, insert_time);
	printf("Final load factor: %.2f\n", wlf_hash_load_factor(hash));

	// Measure lookup time
	start = clock();
	int found_count = 0;
	for (int i = 0; i < NUM_ENTRIES; i++) {
		int key = i;
		if (wlf_hash_find(hash, &key)) {
			found_count++;
		}
	}
	end = clock();
	double lookup_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

	printf("\nLooked up %d entries in %.2f ms\n", NUM_ENTRIES, lookup_time);
	printf("Found: %d/%d\n", found_count, NUM_ENTRIES);

	// Clean up
	wlf_hash_destroy(hash, free, free);
	printf("\nHash table destroyed.\n");
}

/* ========================================================================
 * Example 7: Custom hash function for structs
 * ======================================================================== */

struct coord {
	int x;
	int y;
};

static uint32_t coord_hash(const void *key) {
	const struct coord *c = key;
	// Combine x and y using a simple hash
	return wlf_hash_int(&c->x) ^ (wlf_hash_int(&c->y) << 16);
}

static int coord_compare(const void *a, const void *b) {
	const struct coord *ca = a;
	const struct coord *cb = b;
	if (ca->x != cb->x) return ca->x - cb->x;
	return ca->y - cb->y;
}

static void destroy_coord(void *ptr) {
	free(ptr);
}

static void example_custom_hash(void) {
	printf("\n=== Example 7: Custom Hash Function for Structs ===\n");

	struct wlf_hash *hash = wlf_hash_create(coord_hash, coord_compare);
	assert(hash != NULL);

	// Insert grid coordinates with labels
	struct {
		int x, y;
		const char *label;
	} points[] = {
		{0, 0, "Origin"},
		{10, 20, "Point A"},
		{-5, 15, "Point B"},
		{7, -3, "Point C"},
	};

	printf("Inserting coordinate labels:\n");
	for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
		struct coord *c = malloc(sizeof(struct coord));
		c->x = points[i].x;
		c->y = points[i].y;

		wlf_hash_insert(hash, c, (void *)points[i].label);
		printf("  (%d, %d) => %s\n", c->x, c->y, points[i].label);
	}

	// Lookup a point
	printf("\nLookup (%d, %d):\n", 10, 20);
	struct coord search = {10, 20};
	char *label = wlf_hash_find(hash, &search);
	printf("  Found: %s\n", label ? label : "Not found");

	// Iterate
	printf("\nAll coordinates:\n");
	struct wlf_hash_iterator it;
	wlf_hash_foreach_entry(hash, it) {
		struct coord *c = wlf_hash_iterator_key(&it);
		const char *lbl = wlf_hash_iterator_value(&it);
		printf("  (%d, %d) => %s\n", c->x, c->y, lbl);
	}

	// Clean up
	wlf_hash_destroy(hash, destroy_coord, NULL);
	printf("\nHash table destroyed.\n");
}

/* ========================================================================
 * Main function
 * ======================================================================== */

int main(void) {
	printf("╔════════════════════════════════════════════════════════╗\n");
	printf("║       wlf_hash Usage Examples and Tests               ║\n");
	printf("╚════════════════════════════════════════════════════════╝\n");

	example_string_keys();
	example_int_keys();
	example_foreach_callback();
	example_ptr_keys();
	example_clear_and_resize();
	example_performance();
	example_custom_hash();

	printf("\n╔════════════════════════════════════════════════════════╗\n");
	printf("║       All examples completed successfully!            ║\n");
	printf("╚════════════════════════════════════════════════════════╝\n");

	return 0;
}
