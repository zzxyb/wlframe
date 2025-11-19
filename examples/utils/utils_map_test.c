/**
 * @file        utils_map_test.c
 * @brief       Example and test program for wlf_map.
 * @details     Demonstrates various usage patterns of the wlf_map API,
 *              including integer keys, string keys, custom objects, and
 *              iterator usage.
 * @author      YaoBing Xiao
 * @date        2025-10-21
 */

#include "wlf/utils/wlf_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========================================================================
 * Example 1: Integer keys with string values
 * ======================================================================== */

/**
 * @brief Comparison function for integer keys.
 */
static int int_compare(const void *a, const void *b) {
	int ia = *(const int *)a;
	int ib = *(const int *)b;
	return ia - ib;
}

static void example_int_keys(void) {
	printf("\n=== Example 1: Integer Keys with String Values ===\n");

	// Create map with integer comparison
	struct wlf_map *map = wlf_map_create(int_compare);
	assert(map != NULL);

	// Insert some key-value pairs
	int keys[] = {42, 17, 99, 3, 56, 23};
	const char *values[] = {
		"The Answer",
		"Lucky Number",
		"High Score",
		"Magic Number",
		"Random Value",
		"Jordan"
	};

	printf("Inserting %zu entries...\n", sizeof(keys) / sizeof(keys[0]));
	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		bool ok = wlf_map_insert(map, &keys[i], (void *)values[i]);
		assert(ok);
		printf("  Inserted: %d => %s\n", keys[i], values[i]);
	}

	printf("\nMap size: %zu\n", wlf_map_size(map));

	// Find specific values
	printf("\nLookup operations:\n");
	int search_key = 42;
	char *found = wlf_map_find(map, &search_key);
	printf("  Key %d: %s\n", search_key, found ? found : "Not found");

	search_key = 100;
	found = wlf_map_find(map, &search_key);
	printf("  Key %d: %s\n", search_key, found ? found : "Not found");

	// Iterate over map (in sorted order)
	printf("\nIterating over map (sorted by key):\n");
	struct wlf_map_iterator it;
	wlf_map_foreach_entry(map, it) {
		int *key = wlf_map_iterator_key(&it);
		char *value = wlf_map_iterator_value(&it);
		printf("  %d => %s\n", *key, value);
	}

	// Remove an entry
	printf("\nRemoving key 17...\n");
	int remove_key = 17;
	bool removed = wlf_map_remove(map, &remove_key, NULL, NULL);
	printf("  Removed: %s\n", removed ? "yes" : "no");
	printf("  New size: %zu\n", wlf_map_size(map));

	// Clean up
	wlf_map_destroy(map, NULL, NULL);
	printf("\nMap destroyed.\n");
}

/* ========================================================================
 * Example 2: String keys with integer values
 * ======================================================================== */

/**
 * @brief Comparison function for string keys.
 */
static int string_compare(const void *a, const void *b) {
	return strcmp((const char *)a, (const char *)b);
}

static void example_string_keys(void) {
	printf("\n=== Example 2: String Keys with Integer Values ===\n");

	struct wlf_map *map = wlf_map_create(string_compare);
	assert(map != NULL);

	// Insert fruit counts
	struct {
		const char *name;
		int count;
	} fruits[] = {
		{"apple", 5},
		{"banana", 12},
		{"cherry", 8},
		{"date", 3},
		{"elderberry", 20},
	};

	printf("Inserting fruit inventory:\n");
	for (size_t i = 0; i < sizeof(fruits) / sizeof(fruits[0]); i++) {
		// Allocate memory for the count value
		int *count = malloc(sizeof(int));
		*count = fruits[i].count;

		bool ok = wlf_map_insert(map, (void *)fruits[i].name, count);
		assert(ok);
		printf("  %s: %d\n", fruits[i].name, *count);
	}

	// Check if certain fruits exist
	printf("\nChecking inventory:\n");
	printf("  Has 'banana': %s\n",
	       wlf_map_contains(map, "banana") ? "yes" : "no");
	printf("  Has 'grape': %s\n",
	       wlf_map_contains(map, "grape") ? "yes" : "no");

	// Find and update a value
	printf("\nUpdating cherry count from 8 to 15...\n");
	int *cherry_count = wlf_map_find(map, "cherry");
	if (cherry_count) {
		*cherry_count = 15;
		printf("  New cherry count: %d\n", *cherry_count);
	}

	// Display all fruits in alphabetical order
	printf("\nFinal inventory (alphabetically sorted):\n");
	struct wlf_map_iterator it;
	wlf_map_foreach_entry(map, it) {
		const char *name = wlf_map_iterator_key(&it);
		int *count = wlf_map_iterator_value(&it);
		printf("  %s: %d\n", name, *count);
	}

	// Clean up with custom destructor for values
	wlf_map_destroy(map, NULL, free);
	printf("\nMap destroyed.\n");
}

/* ========================================================================
 * Example 3: Custom struct as key
 * ======================================================================== */

struct point {
	int x;
	int y;
};

/**
 * @brief Comparison function for point structures.
 */
static int point_compare(const void *a, const void *b) {
	const struct point *pa = a;
	const struct point *pb = b;

	// Compare by x first, then by y
	if (pa->x != pb->x) {
		return pa->x - pb->x;
	}
	return pa->y - pb->y;
}

static void destroy_point(void *ptr) {
	free(ptr);
}

static void example_custom_struct(void) {
	printf("\n=== Example 3: Custom Struct as Key ===\n");

	struct wlf_map *map = wlf_map_create(point_compare);
	assert(map != NULL);

	// Create some points and assign labels
	struct {
		int x, y;
		const char *label;
	} locations[] = {
		{0, 0, "Origin"},
		{10, 20, "Point A"},
		{5, 15, "Point B"},
		{-3, 7, "Point C"},
		{10, 5, "Point D"},
	};

	printf("Inserting coordinate labels:\n");
	for (size_t i = 0; i < sizeof(locations) / sizeof(locations[0]); i++) {
		struct point *p = malloc(sizeof(struct point));
		p->x = locations[i].x;
		p->y = locations[i].y;

		bool ok = wlf_map_insert(map, p, (void *)locations[i].label);
		assert(ok);
		printf("  (%d, %d) => %s\n", p->x, p->y, locations[i].label);
	}

	// Find a specific point
	printf("\nLooking up point (10, 20)...\n");
	struct point search = {10, 20};
	char *label = wlf_map_find(map, &search);
	printf("  Found: %s\n", label ? label : "Not found");

	// Iterate in sorted order
	printf("\nAll points (sorted by x, then y):\n");
	struct wlf_map_iterator it;
	wlf_map_foreach_entry(map, it) {
		struct point *p = wlf_map_iterator_key(&it);
		const char *lbl = wlf_map_iterator_value(&it);
		printf("  (%d, %d) => %s\n", p->x, p->y, lbl);
	}

	// Clean up
	wlf_map_destroy(map, destroy_point, NULL);
	printf("\nMap destroyed.\n");
}

/* ========================================================================
 * Example 4: Using foreach callback
 * ======================================================================== */

struct sum_data {
	int total;
	int count;
};

static bool sum_callback(void *key, void *value, void *user_data) {
	(void)key; // Unused

	int val = *(int *)value;
	struct sum_data *data = user_data;

	data->total += val;
	data->count++;

	return true; // Continue iteration
}

static void example_foreach_callback(void) {
	printf("\n=== Example 4: Using Foreach Callback ===\n");

	struct wlf_map *map = wlf_map_create(string_compare);
	assert(map != NULL);

	// Insert some scores
	struct {
		const char *player;
		int score;
	} scores[] = {
		{"Alice", 95},
		{"Bob", 87},
		{"Charlie", 92},
		{"Diana", 88},
		{"Eve", 90},
	};

	printf("Player scores:\n");
	for (size_t i = 0; i < sizeof(scores) / sizeof(scores[0]); i++) {
		int *score = malloc(sizeof(int));
		*score = scores[i].score;

		wlf_map_insert(map, (void *)scores[i].player, score);
		printf("  %s: %d\n", scores[i].player, *score);
	}

	// Calculate average using foreach
	struct sum_data data = {0, 0};
	wlf_map_foreach(map, sum_callback, &data);

	double average = data.count > 0 ? (double)data.total / data.count : 0.0;
	printf("\nTotal: %d, Count: %d, Average: %.2f\n",
	       data.total, data.count, average);

	// Clean up
	wlf_map_destroy(map, NULL, free);
	printf("\nMap destroyed.\n");
}

/* ========================================================================
 * Example 5: Testing clear and empty operations
 * ======================================================================== */

static void example_clear_operations(void) {
	printf("\n=== Example 5: Clear and Empty Operations ===\n");

	struct wlf_map *map = wlf_map_create(int_compare);
	assert(map != NULL);

	printf("Initial state - Empty: %s\n",
	       wlf_map_is_empty(map) ? "yes" : "no");

	// Add some entries
	int keys[] = {1, 2, 3, 4, 5};
	char *values[] = {"one", "two", "three", "four", "five"};

	for (size_t i = 0; i < 5; i++) {
		wlf_map_insert(map, &keys[i], values[i]);
	}

	printf("After insertion - Size: %zu, Empty: %s\n",
	       wlf_map_size(map),
	       wlf_map_is_empty(map) ? "yes" : "no");

	// Clear the map
	printf("Clearing map...\n");
	wlf_map_clear(map, NULL, NULL);

	printf("After clear - Size: %zu, Empty: %s\n",
	       wlf_map_size(map),
	       wlf_map_is_empty(map) ? "yes" : "no");

	// Add entries again
	printf("Re-inserting entries...\n");
	for (size_t i = 0; i < 3; i++) {
		wlf_map_insert(map, &keys[i], values[i]);
	}

	printf("After re-insertion - Size: %zu\n", wlf_map_size(map));

	// Clean up
	wlf_map_destroy(map, NULL, NULL);
	printf("\nMap destroyed.\n");
}

/* ========================================================================
 * Example 6: Update existing values
 * ======================================================================== */

static void example_update_values(void) {
	printf("\n=== Example 6: Updating Existing Values ===\n");

	struct wlf_map *map = wlf_map_create(string_compare);
	assert(map != NULL);

	// Initial configuration
	const char *config_key = "timeout";
	int *timeout1 = malloc(sizeof(int));
	*timeout1 = 30;

	printf("Setting %s = %d\n", config_key, *timeout1);
	wlf_map_insert(map, (void *)config_key, timeout1);

	// Update the value (map will keep the new value)
	int *timeout2 = malloc(sizeof(int));
	*timeout2 = 60;

	printf("Updating %s = %d\n", config_key, *timeout2);

	// Get old value before updating
	int *old_value = wlf_map_find(map, config_key);

	// Insert updates the existing entry
	wlf_map_insert(map, (void *)config_key, timeout2);

	// Free the old value manually
	if (old_value) {
		free(old_value);
	}

	// Verify the update
	int *current = wlf_map_find(map, config_key);
	printf("Current value of %s: %d\n", config_key, *current);

	// Clean up
	wlf_map_destroy(map, NULL, free);
	printf("\nMap destroyed.\n");
}

/* ========================================================================
 * Main function
 * ======================================================================== */

int main(void) {
	printf("╔════════════════════════════════════════════════════════╗\n");
	printf("║        wlf_map Usage Examples and Tests               ║\n");
	printf("╚════════════════════════════════════════════════════════╝\n");

	example_int_keys();
	example_string_keys();
	example_custom_struct();
	example_foreach_callback();
	example_clear_operations();
	example_update_values();

	printf("\n╔════════════════════════════════════════════════════════╗\n");
	printf("║        All examples completed successfully!           ║\n");
	printf("╚════════════════════════════════════════════════════════╝\n");

	return 0;
}
