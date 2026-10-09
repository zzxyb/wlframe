/**
 * @file        wlf_hash.h
 * @brief       Hash table (unordered associative container) utility for wlframe.
 * @details     This file provides a hash table implementation similar to Qt's QHash,
 *              based on open addressing with linear probing. It supports generic
 *              key-value pairs with custom hash and comparison functions, providing
 *              O(1) average-case insertion, deletion, and lookup operations.
 *
 *              Features:
 *                  - Generic key-value storage with void pointers
 *                  - Custom hash function support
 *                  - Custom key comparison function support
 *                  - O(1) average insert, find, remove operations
 *                  - Automatic resizing and rehashing
 *                  - Iterator support for traversal
 *                  - Load factor management
 *
 *              Typical usage:
 *              @code
 *              // Define hash and comparison functions for string keys
 *              uint32_t string_hash(const void *key) {
 *                  return wlf_hash_string((const char *)key);
 *              }
 *
 *              int string_compare(const void *a, const void *b) {
 *                  return strcmp((const char *)a, (const char *)b);
 *              }
 *
 *              // Create and use hash table
 *              struct wlf_hash *hash = wlf_hash_create(string_hash, string_compare);
 *              wlf_hash_insert(hash, "key", "value");
 *
 *              char *found = wlf_hash_find(hash, "key");
 *              wlf_hash_destroy(hash, NULL, NULL);
 *              @endcode
 *
 * @author      YaoBing Xiao
 * @date        2025-10-21
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-10-21, initial version\n
 */

#ifndef UTILS_WLF_HASH_H
#define UTILS_WLF_HASH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Hash function type.
 * @param key The key to hash.
 * @return Hash value as uint32_t.
 */
typedef uint32_t (*wlf_hash_func_t)(const void *key);

/**
 * @brief Key comparison function type.
 * @param a First key to compare.
 * @param b Second key to compare.
 * @return 0 if equal, non-zero otherwise.
 */
typedef int (*wlf_hash_compare_func_t)(const void *a, const void *b);

/**
 * @brief Callback function for destroying keys.
 * @param key The key to destroy.
 */
typedef void (*wlf_hash_destroy_key_func_t)(void *key);

/**
 * @brief Callback function for destroying values.
 * @param value The value to destroy.
 */
typedef void (*wlf_hash_destroy_value_func_t)(void *value);

/**
 * @brief Callback function for iterating over hash entries.
 * @param key The entry's key.
 * @param value The entry's value.
 * @param user_data User-provided data.
 * @return true to continue iteration, false to stop.
 */
typedef bool (*wlf_hash_foreach_func_t)(void *key, void *value, void *user_data);

/**
 * @brief Hash table bucket state.
 */
enum wlf_hash_bucket_state {
	WLF_HASH_EMPTY,     /**< Bucket is empty */
	WLF_HASH_OCCUPIED,  /**< Bucket contains data */
	WLF_HASH_DELETED    /**< Bucket was deleted (tombstone) */
};

/**
 * @brief Hash table bucket structure.
 */
struct wlf_hash_bucket {
	void *key;                          /**< Key pointer */
	void *value;                        /**< Value pointer */
	enum wlf_hash_bucket_state state;   /**< Bucket state */
	uint32_t hash;                      /**< Cached hash value */
};

/**
 * @brief Hash table structure.
 */
struct wlf_hash {
	struct wlf_hash_bucket *buckets;    /**< Array of buckets */
	size_t capacity;                    /**< Total bucket capacity */
	size_t size;                        /**< Number of entries */
	size_t deleted;                     /**< Number of deleted entries (tombstones) */
	wlf_hash_func_t hash_func;          /**< Hash function */
	wlf_hash_compare_func_t compare;    /**< Key comparison function */
};

/**
 * @brief Hash table iterator structure.
 */
struct wlf_hash_iterator {
	struct wlf_hash *hash;              /**< Hash table being iterated */
	size_t index;                       /**< Current bucket index */
};

/**
 * @brief Create a new hash table.
 * @param hash_func Hash function for keys.
 * @param compare Key comparison function.
 * @return Pointer to the newly created hash table, or NULL on failure.
 */
struct wlf_hash *wlf_hash_create(wlf_hash_func_t hash_func,
                                 wlf_hash_compare_func_t compare);

/**
 * @brief Destroy a hash table and free all resources.
 * @param hash The hash table to destroy.
 * @param destroy_key Optional callback to destroy keys (can be NULL).
 * @param destroy_value Optional callback to destroy values (can be NULL).
 */
void wlf_hash_destroy(struct wlf_hash *hash,
                      wlf_hash_destroy_key_func_t destroy_key,
                      wlf_hash_destroy_value_func_t destroy_value);

/**
 * @brief Insert or update a key-value pair in the hash table.
 * @param hash The hash table.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 * @return true on success, false on failure.
 */
bool wlf_hash_insert(struct wlf_hash *hash, void *key, void *value);

/**
 * @brief Remove a key-value pair from the hash table.
 * @param hash The hash table.
 * @param key The key to remove.
 * @param destroy_key Optional callback to destroy the key (can be NULL).
 * @param destroy_value Optional callback to destroy the value (can be NULL).
 * @return true if the key was found and removed, false otherwise.
 */
bool wlf_hash_remove(struct wlf_hash *hash, void *key,
                     wlf_hash_destroy_key_func_t destroy_key,
                     wlf_hash_destroy_value_func_t destroy_value);

/**
 * @brief Find a value by key.
 * @param hash The hash table.
 * @param key The key to search for.
 * @return The value associated with the key, or NULL if not found.
 */
void *wlf_hash_find(struct wlf_hash *hash, const void *key);

/**
 * @brief Check if a key exists in the hash table.
 * @param hash The hash table.
 * @param key The key to check.
 * @return true if the key exists, false otherwise.
 */
bool wlf_hash_contains(struct wlf_hash *hash, const void *key);

/**
 * @brief Get the number of entries in the hash table.
 * @param hash The hash table.
 * @return The number of key-value pairs.
 */
size_t wlf_hash_size(struct wlf_hash *hash);

/**
 * @brief Check if the hash table is empty.
 * @param hash The hash table.
 * @return true if the hash table is empty, false otherwise.
 */
bool wlf_hash_is_empty(struct wlf_hash *hash);

/**
 * @brief Clear all entries from the hash table.
 * @param hash The hash table.
 * @param destroy_key Optional callback to destroy keys (can be NULL).
 * @param destroy_value Optional callback to destroy values (can be NULL).
 */
void wlf_hash_clear(struct wlf_hash *hash,
                    wlf_hash_destroy_key_func_t destroy_key,
                    wlf_hash_destroy_value_func_t destroy_value);

/**
 * @brief Iterate over all entries in the hash table.
 * @param hash The hash table.
 * @param func Callback function for each entry.
 * @param user_data User data passed to the callback.
 */
void wlf_hash_foreach(struct wlf_hash *hash,
                      wlf_hash_foreach_func_t func,
                      void *user_data);

/**
 * @brief Get the current load factor of the hash table.
 * @param hash The hash table.
 * @return Load factor (size / capacity).
 */
double wlf_hash_load_factor(struct wlf_hash *hash);

/**
 * @brief Create an iterator for the hash table (points to the first element).
 * @param hash The hash table.
 * @return Iterator structure.
 */
struct wlf_hash_iterator wlf_hash_iterator_create(struct wlf_hash *hash);

/**
 * @brief Check if the iterator has a next element.
 * @param it The iterator.
 * @return true if there is a next element, false otherwise.
 */
bool wlf_hash_iterator_has_next(struct wlf_hash_iterator *it);

/**
 * @brief Advance the iterator to the next element.
 * @param it The iterator.
 */
void wlf_hash_iterator_next(struct wlf_hash_iterator *it);

/**
 * @brief Get the current key from the iterator.
 * @param it The iterator.
 * @return The current key, or NULL if invalid.
 */
void *wlf_hash_iterator_key(struct wlf_hash_iterator *it);

/**
 * @brief Get the current value from the iterator.
 * @param it The iterator.
 * @return The current value, or NULL if invalid.
 */
void *wlf_hash_iterator_value(struct wlf_hash_iterator *it);

/**
 * @brief Helper macro for iterating over hash table entries.
 * @param hash The hash table to iterate over.
 * @param it Iterator variable name.
 *
 * Example:
 * @code
 * struct wlf_hash_iterator it;
 * wlf_hash_foreach_entry(my_hash, it) {
 *     void *key = wlf_hash_iterator_key(&it);
 *     void *value = wlf_hash_iterator_value(&it);
 *     // Use key and value
 * }
 * @endcode
 */
#define wlf_hash_foreach_entry(hash, it) \
	for (it = wlf_hash_iterator_create(hash); \
	     wlf_hash_iterator_has_next(&it); \
	     wlf_hash_iterator_next(&it))

/**
 * @brief Hash function for strings (djb2 algorithm).
 * @param str The string to hash.
 * @return Hash value.
 */
uint32_t wlf_hash_string(const char *str);

/**
 * @brief Hash function for integers.
 * @param key Pointer to integer.
 * @return Hash value.
 */
uint32_t wlf_hash_int(const void *key);

/**
 * @brief Hash function for pointers.
 * @param key The pointer to hash.
 * @return Hash value.
 */
uint32_t wlf_hash_ptr(const void *key);

/**
 * @brief Hash function for byte arrays.
 * @param data The byte array.
 * @param len Length of the array.
 * @return Hash value.
 */
uint32_t wlf_hash_bytes(const void *data, size_t len);

#endif // UTILS_WLF_HASH_H
