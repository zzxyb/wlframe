/**
 * @file        wlf_sequential_animation_group.h
 * @brief       Sequential animation group implementation for wlframe.
 * @details     A sequential animation group runs each child animation to
 *              completion before starting the next child in list order.
 * @author      YaoBing Xiao
 * @date        2026-09-06
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-09-06, initial version\n
 */

#ifndef ANIMATION_WLF_SEQUENTIAL_ANIMATION_GROUP_H
#define ANIMATION_WLF_SEQUENTIAL_ANIMATION_GROUP_H

#include "wlf/animation/wlf_animation_group.h"

#include <stdbool.h>

/**
 * @brief Animation group that runs children one after another.
 */
struct wlf_sequential_animation_group {
	struct wlf_animation_group base; /**< Base animation group. */
	struct {
		struct wlf_animation *current; /**< Currently active child. */
	} WLF_PRIVATE;
};

/**
 * @brief Create an empty sequential animation group.
 *
 * @return New sequential animation group, or NULL on allocation failure.
 */
struct wlf_sequential_animation_group *
wlf_sequential_animation_group_create(void);

/**
 * @brief Check whether a group is a sequential animation group.
 *
 * @param group Animation group to test.
 * @return true if the group is sequential, false otherwise.
 */
bool wlf_animation_group_is_sequential(
	const struct wlf_animation_group *group);

/**
 * @brief Convert a base group to a sequential animation group.
 *
 * @param group Animation group to convert.
 * @return Sequential group, or NULL if @p group has a different type.
 */
struct wlf_sequential_animation_group *
wlf_sequential_animation_group_from_group(
	struct wlf_animation_group *group);

#endif // ANIMATION_WLF_SEQUENTIAL_ANIMATION_GROUP_H
