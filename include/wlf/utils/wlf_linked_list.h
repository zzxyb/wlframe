/**
 * @file        wlf_linked_list.h
 * @brief       Doubly-linked list utility for wlframe.
 * @details     This file provides a simple and efficient doubly-linked list implementation.
 *              It uses a sentinel head node and supports safe insertion, removal, and iteration.
 *              Macros are provided for convenient iteration and container access.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef UTILS_WLF_LINKED_LIST_H
#define UTILS_WLF_LINKED_LIST_H

#include <stddef.h>

#if __STDC_VERSION__ >= 202311L
#define WLF_TYPEOF(expr) typeof(expr)
#else
#define WLF_TYPEOF(expr) __typeof__(expr)
#endif

/**
 * @brief Doubly-linked list node.
 *
 * This list implementation uses a sentinel head node that must be initialized
 * with wlf_linked_list_init(). The head's next/prev pointers point to itself when empty,
 * or to the first/last elements respectively when the list contains elements.
 *
 * Example usage:
 * @code
 * struct element {
 *     int foo;
 *     struct wlf_linked_list link;
 * };
 *
 * // Initialize and populate list
 * struct wlf_linked_list list;
 * struct element e1, e2;
 *
 * wlf_linked_list_init(&list);
 * wlf_linked_list_insert(&list, &e1.link);
 * wlf_linked_list_insert(&list, &e2.link);
 *
 * // Iterate over list
 * struct element *e;
 * wlf_linked_list_for_each(e, &list, link) {
 *     // Use e
 * }
 * @endcode
 */
struct wlf_linked_list {
	struct wlf_linked_list *prev;  /**< Previous list element */
	struct wlf_linked_list *next;  /**< Next list element */
};

/**
 * @brief Initializes an empty list.
 *
 * @param list List to initialize
 */
void wlf_linked_list_init(struct wlf_linked_list *list);

/**
 * @brief Inserts an element into the list.
 *
 * @param list Element after which to insert
 * @param elm Element to insert
 *
 * @note Inserting an element that's already in a list will corrupt both lists
 */
void wlf_linked_list_insert(struct wlf_linked_list *list, struct wlf_linked_list *elm);

/**
 * @brief Removes an element from its list.
 *
 * @param elm Element to remove
 *
 * @note Leaves elm in an invalid state
 */
void wlf_linked_list_remove(struct wlf_linked_list *elm);

/**
 * @brief Gets the number of elements in the list.
 *
 * @param list List to check
 * @return Number of elements
 *
 * @note O(n) operation
 */
int wlf_linked_list_length(const struct wlf_linked_list *list);

/**
 * @brief Checks if a list is empty.
 *
 * @param list List to check
 * @return 1 if empty, 0 if not empty
 */
int wlf_linked_list_empty(const struct wlf_linked_list *list);

/**
 * @brief Moves all elements from one list to another.
 *
 * @param list Destination list
 * @param other Source list (becomes invalid)
 */
void wlf_linked_list_insert_list(struct wlf_linked_list *list, struct wlf_linked_list *other);

/**
 * @brief Gets the containing structure from a member pointer.
 *
 * Example:
 * @code
 * struct container {
 *     struct wlf_linked_list member;
 * };
 *
 * struct wlf_linked_list *p = ...;
 * struct container *c = wlf_container_of(p, c, member);
 * @endcode
 *
 * @param ptr Pointer to member
 * @param sample Sample pointer to containing type
 * @param member Name of the member within the struct
 * @return Pointer to containing structure
 */
#define wlf_container_of(ptr, sample, member)				\
	(WLF_TYPEOF(sample))((char *)(ptr) -				\
				offsetof(WLF_TYPEOF(*sample), member))

/**
 * @brief Iterates forward through a list.
 *
 * @param pos Iterator variable
 * @param head List head
 * @param member Name of list member in structure
 */
#define wlf_linked_list_for_each(pos, head, member)				\
	for (pos = wlf_container_of((head)->next, pos, member);	\
			&pos->member != (head);					\
			pos = wlf_container_of(pos->member.next, pos, member))

/**
 * @brief Iterates forward through a list safely (allows removal).
 *
 * @param pos Iterator variable
 * @param tmp Temporary storage
 * @param head List head
 * @param member Name of list member in structure
 *
 * @note Only removal of current element is safe
 */
#define wlf_linked_list_for_each_safe(pos, tmp, head, member)			\
	for (pos = wlf_container_of((head)->next, pos, member),		\
			tmp = wlf_container_of((pos)->member.next, tmp, member);	\
			&pos->member != (head);					\
			pos = tmp,							\
			tmp = wlf_container_of(pos->member.next, tmp, member))

/**
 * @brief Iterates backwards through a list.
 *
 * @param pos Iterator variable
 * @param head List head
 * @param member Name of list member in structure
 */
#define wlf_linked_list_for_each_reverse(pos, head, member)			\
	for (pos = wlf_container_of((head)->prev, pos, member);	\
			&pos->member != (head);					\
			pos = wlf_container_of(pos->member.prev, pos, member))

/**
 * @brief Iterates backwards through a list safely (allows removal).
 *
 * @param pos Iterator variable
 * @param tmp Temporary storage
 * @param head List head
 * @param member Name of list member in structure
 *
 * @note Only removal of current element is safe
 */
#define wlf_linked_list_for_each_reverse_safe(pos, tmp, head, member)		\
	for (pos = wlf_container_of((head)->prev, pos, member),	\
			tmp = wlf_container_of((pos)->member.prev, tmp, member);	\
			&pos->member != (head);					\
			pos = tmp,							\
			tmp = wlf_container_of(pos->member.prev, tmp, member))

#endif // UTILS_WLF_LINKED_LIST_H
