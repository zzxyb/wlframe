#ifndef WLF_LIST_H
#define WLF_LIST_H

/**
 * @brief Structure representing a dynamic list.
 */
struct wlf_list {
	void **items; /**< Pointer to the array of items in the list */
	int capacity; /**< Maximum number of items the list can hold */
	int length; /**< Current number of items in the list */
};

/**
 * @brief Creates a new dynamic list.
 * @return Pointer to the newly created wlf_list instance.
 */
struct wlf_list *wlf_list_create(void);

/**
 * @brief Frees the memory allocated for the specified list.
 * @param list Pointer to the wlf_list instance to free.
 */
void wlf_list_destroy(struct wlf_list *list);

/**
 * @brief Adds an item to the end of the specified list.
 * @param list Pointer to the wlf_list instance.
 * @param item Pointer to the item to add.
 */
void wlf_list_add(struct wlf_list *list, void *item);

/**
 * @brief Inserts an item at the specified index in the list.
 * @param list Pointer to the wlf_list instance.
 * @param index Index at which to insert the item.
 * @param item Pointer to the item to insert.
 */
void wlf_list_insert(struct wlf_list *list, int index, void *item);

/**
 * @brief Deletes an item at the specified index from the list.
 * @param list Pointer to the wlf_list instance.
 * @param index Index of the item to delete.
 */
void wlf_list_delete(struct wlf_list *list, int index);

/**
 * @brief Concatenates another list to the end of the specified list.
 * @param list Pointer to the wlf_list instance.
 * @param source Pointer to the source wlf_list to concatenate.
 */
void wlf_list_cat(struct wlf_list *list, struct wlf_list *source);

/**
 * @brief Sorts the items in the list using the specified comparison function.
 * @param list Pointer to the wlf_list instance.
 * @param compare Comparison function to determine the order of items.
 */
void wlf_list_qsort(struct wlf_list *list,
	int compare(const void *left, const void *right));

/**
 * @brief Sequentially finds an item in the list using the specified comparison function.
 * @param list Pointer to the wlf_list instance.
 * @param compare Comparison function to compare items.
 * @param cmp_to Pointer to the item to compare against.
 * @return Index of the found item, or -1 if not found.
 */
int wlf_list_seq_find(struct wlf_list *list,
	int compare(const void *item, const void *cmp_to), const void *cmp_to);

/**
 * @brief Finds the index of the specified item in the list.
 * @param list Pointer to the wlf_list instance.
 * @param item Pointer to the item to find.
 * @return Index of the found item, or -1 if not found.
 */
int wlf_list_find(struct wlf_list *list, const void *item);

/**
 * @brief Performs a stable sort on the items in the list using the specified comparison function.
 * @param list Pointer to the wlf_list instance.
 * @param compare Comparison function to determine the order of items.
 */
void wlf_list_stable_sort(struct wlf_list *list,
	int compare(const void *a, const void *b));

/**
 * @brief Swaps two items in the list at the specified indices.
 * @param list Pointer to the wlf_list instance.
 * @param src Index of the first item to swap.
 * @param dest Index of the second item to swap.
 */
void wlf_list_swap(struct wlf_list *list, int src, int dest);

/**
 * @brief Moves the specified item to the end of the list.
 * @param list Pointer to the wlf_list instance.
 * @param item Pointer to the item to move.
 */
void wlf_list_move_to_end(struct wlf_list *list, void *item);

/**
 * @brief Frees the items in the list and destroys the list itself.
 * @param list Pointer to the wlf_list instance to free.
 */
void wlf_list_free_items_and_destroy(struct wlf_list *list);

#endif
