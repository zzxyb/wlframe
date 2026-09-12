/**
 * @file        wlf_map.h
 * @brief       Ordered map (associative container) utility for wlframe.
 * @details     This file provides an ordered map implementation similar to Qt's QMap,
 *              based on a red-black tree structure. It supports generic key-value pairs
 *              with custom comparison functions, providing O(log n) insertion, deletion,
 *              and lookup operations. The map maintains keys in sorted order.
 *
 *              Features:
 *                  - Generic key-value storage with void pointers
 *                  - Custom key comparison function support
 *                  - Ordered iteration (in-order traversal)
 *                  - O(log n) insert, find, remove operations
 *                  - Red-black tree self-balancing
 *                  - Iterator support for range-based operations
 *
 *              Typical usage:
 *              @code
 *              // Define comparison function for integer keys
 *              int int_compare(const void *a, const void *b) {
 *                  int ia = *(const int*)a;
 *                  int ib = *(const int*)b;
 *                  return ia - ib;
 *              }
 *
 *              // Create and use map
 *              struct wlf_map *map = wlf_map_create(int_compare);
 *              int key = 42;
 *              char *value = "answer";
 *              wlf_map_insert(map, &key, value);
 *
 *              char *found = wlf_map_find(map, &key);
 *              wlf_map_destroy(map, NULL, NULL);
 *              @endcode
 *
 * @author      YaoBing Xiao
 * @date        2025-10-21
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-10-21, initial version\n
 */

#ifndef UTILS_WLF_MAP_H
#define UTILS_WLF_MAP_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Key comparison function type.
 * @param a First key to compare.
 * @param b Second key to compare.
 * @return Negative if a < b, 0 if a == b, positive if a > b.
 */
typedef int (*wlf_map_compare_func_t)(const void *a, const void *b);

/**
 * @brief Callback function for destroying keys.
 * @param key The key to destroy.
 */
typedef void (*wlf_map_destroy_key_func_t)(void *key);

/**
 * @brief Callback function for destroying values.
 * @param value The value to destroy.
 */
typedef void (*wlf_map_destroy_value_func_t)(void *value);

/**
 * @brief Callback function for iterating over map entries.
 * @param key The entry's key.
 * @param value The entry's value.
 * @param user_data User-provided data.
 * @return true to continue iteration, false to stop.
 */
typedef bool (*wlf_map_foreach_func_t)(void *key, void *value, void *user_data);

/**
 * @brief Red-black tree node colors.
 */
enum wlf_map_color {
	WLF_MAP_RED,    /**< Red node */
	WLF_MAP_BLACK   /**< Black node */
};

/**
 * @brief Red-black tree node structure.
 */
struct wlf_map_node {
	void *key;                      /**< Key pointer */
	void *value;                    /**< Value pointer */
	enum wlf_map_color color;       /**< Node color for red-black tree */
	struct wlf_map_node *left;      /**< Left child */
	struct wlf_map_node *right;     /**< Right child */
	struct wlf_map_node *parent;    /**< Parent node */
};

/**
 * @brief Map structure (red-black tree).
 */
struct wlf_map {
	struct wlf_map_node *root;      /**< Root node of the tree */
	wlf_map_compare_func_t compare; /**< Key comparison function */
	size_t size;                    /**< Number of entries in the map */
};

/**
 * @brief Map iterator structure.
 */
struct wlf_map_iterator {
	struct wlf_map *map;            /**< Map being iterated */
	struct wlf_map_node *current;   /**< Current node */
};

/**
 * @brief Create a new map.
 * @param compare Key comparison function.
 * @return Pointer to the newly created map, or NULL on failure.
 */
struct wlf_map *wlf_map_create(wlf_map_compare_func_t compare);

/**
 * @brief Destroy a map and free all resources.
 * @param map The map to destroy.
 * @param destroy_key Optional callback to destroy keys (can be NULL).
 * @param destroy_value Optional callback to destroy values (can be NULL).
 */
void wlf_map_destroy(struct wlf_map *map,
                     wlf_map_destroy_key_func_t destroy_key,
                     wlf_map_destroy_value_func_t destroy_value);

/**
 * @brief Insert or update a key-value pair in the map.
 * @param map The map.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 * @return true on success, false on failure.
 */
bool wlf_map_insert(struct wlf_map *map, void *key, void *value);

/**
 * @brief Remove a key-value pair from the map.
 * @param map The map.
 * @param key The key to remove.
 * @param destroy_key Optional callback to destroy the key (can be NULL).
 * @param destroy_value Optional callback to destroy the value (can be NULL).
 * @return true if the key was found and removed, false otherwise.
 */
bool wlf_map_remove(struct wlf_map *map, void *key,
                    wlf_map_destroy_key_func_t destroy_key,
                    wlf_map_destroy_value_func_t destroy_value);

/**
 * @brief Find a value by key.
 * @param map The map.
 * @param key The key to search for.
 * @return The value associated with the key, or NULL if not found.
 */
void *wlf_map_find(struct wlf_map *map, const void *key);

/**
 * @brief Check if a key exists in the map.
 * @param map The map.
 * @param key The key to check.
 * @return true if the key exists, false otherwise.
 */
bool wlf_map_contains(struct wlf_map *map, const void *key);

/**
 * @brief Get the number of entries in the map.
 * @param map The map.
 * @return The number of key-value pairs.
 */
size_t wlf_map_size(struct wlf_map *map);

/**
 * @brief Check if the map is empty.
 * @param map The map.
 * @return true if the map is empty, false otherwise.
 */
bool wlf_map_is_empty(struct wlf_map *map);

/**
 * @brief Clear all entries from the map.
 * @param map The map.
 * @param destroy_key Optional callback to destroy keys (can be NULL).
 * @param destroy_value Optional callback to destroy values (can be NULL).
 */
void wlf_map_clear(struct wlf_map *map,
                   wlf_map_destroy_key_func_t destroy_key,
                   wlf_map_destroy_value_func_t destroy_value);

/**
 * @brief Iterate over all entries in the map in sorted order.
 * @param map The map.
 * @param func Callback function for each entry.
 * @param user_data User data passed to the callback.
 */
void wlf_map_foreach(struct wlf_map *map,
                     wlf_map_foreach_func_t func,
                     void *user_data);

/**
 * @brief Create an iterator for the map (points to the first element).
 * @param map The map.
 * @return Iterator structure.
 */
struct wlf_map_iterator wlf_map_iterator_create(struct wlf_map *map);

/**
 * @brief Check if the iterator has a next element.
 * @param it The iterator.
 * @return true if there is a next element, false otherwise.
 */
bool wlf_map_iterator_has_next(struct wlf_map_iterator *it);

/**
 * @brief Advance the iterator to the next element.
 * @param it The iterator.
 */
void wlf_map_iterator_next(struct wlf_map_iterator *it);

/**
 * @brief Get the current key from the iterator.
 * @param it The iterator.
 * @return The current key, or NULL if invalid.
 */
void *wlf_map_iterator_key(struct wlf_map_iterator *it);

/**
 * @brief Get the current value from the iterator.
 * @param it The iterator.
 * @return The current value, or NULL if invalid.
 */
void *wlf_map_iterator_value(struct wlf_map_iterator *it);

/**
 * @brief Helper macro for iterating over map entries.
 * @param map The map to iterate over.
 * @param it Iterator variable name.
 *
 * Example:
 * @code
 * struct wlf_map_iterator it;
 * wlf_map_foreach_entry(my_map, it) {
 *     void *key = wlf_map_iterator_key(&it);
 *     void *value = wlf_map_iterator_value(&it);
 *     // Use key and value
 * }
 * @endcode
 */
#define wlf_map_foreach_entry(map, it) \
	for (it = wlf_map_iterator_create(map); \
	     wlf_map_iterator_has_next(&it); \
	     wlf_map_iterator_next(&it))

#endif // UTILS_WLF_MAP_H
