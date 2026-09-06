/**
 * @file        wlf_parallel_animation_group.h
 * @brief       Parallel animation group implementation for wlframe.
 * @details     A parallel animation group starts and advances all child
 *              animations together.
 * @author      YaoBing Xiao
 * @date        2026-09-06
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-09-06, initial version\n
 */

#ifndef ANIMATION_WLF_PARALLEL_ANIMATION_GROUP_H
#define ANIMATION_WLF_PARALLEL_ANIMATION_GROUP_H

#include "wlf/animation/wlf_animation_group.h"

#include <stdbool.h>

/**
 * @brief Animation group that runs all children concurrently.
 */
struct wlf_parallel_animation_group {
	struct wlf_animation_group base; /**< Base animation group. */
};

/**
 * @brief Create an empty parallel animation group.
 *
 * @return New parallel animation group, or NULL on allocation failure.
 */
struct wlf_parallel_animation_group *
wlf_parallel_animation_group_create(void);

/**
 * @brief Check whether a group is a parallel animation group.
 *
 * @param group Animation group to test.
 * @return true if the group is parallel, false otherwise.
 */
bool wlf_animation_group_is_parallel(
	const struct wlf_animation_group *group);

/**
 * @brief Convert a base group to a parallel animation group.
 *
 * @param group Animation group to convert.
 * @return Parallel group, or NULL if @p group has a different type.
 */
struct wlf_parallel_animation_group *
wlf_parallel_animation_group_from_group(
	struct wlf_animation_group *group);

#endif // ANIMATION_WLF_PARALLEL_ANIMATION_GROUP_H
