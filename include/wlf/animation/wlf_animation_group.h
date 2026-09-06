/**
 * @file        wlf_animation_group.h
 * @brief       Independent animation group abstraction for wlframe.
 * @details     Animation groups own and coordinate child animations without
 *              being animations themselves. Concrete groups define whether
 *              children run in parallel or sequentially.
 * @author      YaoBing Xiao
 * @date        2026-09-06
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-09-06, initial version\n
 */

#ifndef ANIMATION_WLF_ANIMATION_GROUP_H
#define ANIMATION_WLF_ANIMATION_GROUP_H

#include "wlf/animation/wlf_animation.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_animation_group;

/**
 * @brief Implementation hooks for concrete animation group types.
 */
struct wlf_animation_group_impl {
	/**
	 * @brief Start the child animations managed by the group.
	 * @param group Animation group being started.
	 */
	void (*start)(struct wlf_animation_group *group);

	/**
	 * @brief Stop the child animations managed by the group.
	 * @param group Animation group being stopped.
	 */
	void (*stop)(struct wlf_animation_group *group);

	/**
	 * @brief Pause the active child animations in the group.
	 * @param group Animation group being paused.
	 */
	void (*pause)(struct wlf_animation_group *group);

	/**
	 * @brief Resume the paused child animations in the group.
	 * @param group Animation group being resumed.
	 */
	void (*resume)(struct wlf_animation_group *group);

	/**
	 * @brief Advance the active child animations in the group.
	 * @param group Animation group being updated.
	 * @param dt Elapsed time in milliseconds since the previous update.
	 */
	void (*update)(struct wlf_animation_group *group, int64_t dt);

	/**
	 * @brief Release resources owned by the concrete group and free it.
	 * @param group Animation group to destroy.
	 */
	void (*destroy)(struct wlf_animation_group *group);
};

/**
 * @brief Listener for animation group lifecycle events.
 *
 * The listener is not owned by the group and must remain valid until replaced,
 * removed, or the group is destroyed. All callbacks are optional.
 */
struct wlf_animation_group_listener {
	/** Called after child destruction and before the concrete group is destroyed. */
	void (*destroy)(void *data, struct wlf_animation_group *group);
	/** Called after the group's start implementation completes. */
	void (*started)(void *data, struct wlf_animation_group *group);
	/** Called after the group has been explicitly stopped, not on completion. */
	void (*stopped)(void *data, struct wlf_animation_group *group);
	/** Called after a running group has been paused. */
	void (*paused)(void *data, struct wlf_animation_group *group);
	/** Called after a paused group has resumed. */
	void (*resumed)(void *data, struct wlf_animation_group *group);
};

/**
 * @brief Base object embedded in every animation group type.
 *
 * The group owns every animation in @ref children.
 */
struct wlf_animation_group {
	const struct wlf_animation_group_impl *impl; /**< Concrete implementation. */
	struct {
		const struct wlf_animation_group_listener *listener; /**< Registered listener. */
		void *user_data; /**< User data passed to listener callbacks. */
	} WLF_PRIVATE;
	struct wlf_linked_list childrens; /**< Owned child animations. */
	enum wlf_animation_state state; /**< Current group state. */
};

/**
 * @brief Initialize an empty animation group.
 *
 * This function is intended for concrete animation group implementations.
 *
 * @param group Animation group to initialize.
 * @param impl Concrete group implementation hooks.
 */
void wlf_animation_group_init(struct wlf_animation_group *group,
	const struct wlf_animation_group_impl *impl);

/**
 * @brief Destroy an animation group and all of its child animations.
 *
 * @param group Animation group to destroy. NULL is accepted.
 */
void wlf_animation_group_destroy(struct wlf_animation_group *group);

/**
 * @brief Register or replace the listener for an animation group.
 *
 * @param group Animation group to monitor.
 * @param listener Listener owned by the caller, or NULL to remove it.
 * @param data User data passed to every listener callback.
 */
void wlf_animation_group_add_listener(struct wlf_animation_group *group,
	const struct wlf_animation_group_listener *listener, void *data);

/**
 * @brief Start or restart every animation managed by a group.
 *
 * @param group Animation group to start.
 */
void wlf_animation_group_start(struct wlf_animation_group *group);

/**
 * @brief Stop a group and reset its child animations.
 *
 * @param group Animation group to stop.
 */
void wlf_animation_group_stop(struct wlf_animation_group *group);

/**
 * @brief Pause a running animation group.
 *
 * @param group Animation group to pause.
 */
void wlf_animation_group_pause(struct wlf_animation_group *group);

/**
 * @brief Resume a paused animation group.
 *
 * @param group Animation group to resume.
 */
void wlf_animation_group_resume(struct wlf_animation_group *group);

/**
 * @brief Advance the child animations managed by a group.
 *
 * @param group Animation group to update.
 * @param dt Elapsed time in milliseconds since the previous update.
 */
void wlf_animation_group_update(struct wlf_animation_group *group, int64_t dt);

/**
 * @brief Append an animation to a stopped group.
 *
 * The group takes ownership of @p animation on success. The caller must not
 * destroy it unless it is first removed from the group. An animation can only
 * belong to one group at a time and must be stopped before it is added.
 *
 * @param group Destination animation group.
 * @param animation Child animation whose ownership is transferred.
 * @return true on success, false if the animation cannot be added.
 */
bool wlf_animation_group_add_animation(struct wlf_animation_group *group,
	struct wlf_animation *animation);

/**
 * @brief Remove an animation from a stopped group.
 *
 * Ownership of @p animation is returned to the caller on success.
 *
 * @param group Animation group containing the child.
 * @param animation Child animation to remove.
 * @return true on success, false if the child is not removable from the group.
 */
bool wlf_animation_group_remove_animation(struct wlf_animation_group *group,
	struct wlf_animation *animation);

/**
 * @brief Get the number of child animations in a group.
 *
 * @param group Animation group to query.
 * @return Number of child animations.
 */
int wlf_animation_group_animation_count(
	const struct wlf_animation_group *group);

/**
 * @brief Advance a child animation across loop boundaries.
 *
 * This function is intended for concrete animation group implementations.
 *
 * @param animation Child animation to advance.
 * @param dt Elapsed time in milliseconds, which must be non-negative.
 * @return Unconsumed time in milliseconds.
 */
int64_t wlf_animation_group_advance_animation(
	struct wlf_animation *animation, int64_t dt);

#endif // ANIMATION_WLF_ANIMATION_GROUP_H
