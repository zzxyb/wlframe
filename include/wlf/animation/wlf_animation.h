/**
 * @file        wlf_animation.h
 * @brief       Base animation abstraction for wlframe.
 * @details     Defines the common state, lifecycle operations, implementation
 *              hooks, and listener callbacks used by animation types. Animations
 *              support easing curves, finite or infinite looping, and forward,
 *              backward, or alternating playback.
 * @author      YaoBing Xiao
 * @date        2026-02-19
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-19, initial version\n
 */

#ifndef ANIMATION_WLF_ANIMATION_H
#define ANIMATION_WLF_ANIMATION_H

#include "wlf/curve/wlf_curve.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_animation;

/**
 * @brief State of an animation.
 */
enum wlf_animation_state {
	WLF_ANIMATION_STATE_STOPPED,  /**< The animation is not running. */
	WLF_ANIMATION_STATE_PAUSED,   /**< The animation is paused. */
	WLF_ANIMATION_STATE_RUNNING,  /**< The animation is running. */
};

/**
 * @brief Special animation loop counts.
 *
 * A positive integer may also be passed to wlf_animation_set_loop_count() to
 * request that exact number of iterations.
 */
enum wlf_animation_loop {
	WLF_ANIMATION_LOOP_ONCE = 1,      /**< Play one iteration. */
	WLF_ANIMATION_LOOP_INFINITE = -1, /**< Repeat indefinitely. */
};

/**
 * @brief Playback direction of an animation.
 */
enum wlf_animation_direction {
	WLF_ANIMATION_DIRECTION_FORWARD,  /**< Play from the start to the end. */
	WLF_ANIMATION_DIRECTION_BACKWARD, /**< Play from the end to the start. */
	WLF_ANIMATION_DIRECTION_ALTERNATE,/**< Play forward first, then alternate each iteration. */
};

/**
 * @brief Implementation hooks for concrete animation types.
 *
 * The start, stop, pause, and destroy hooks are required. All other hooks are
 * optional. The implementation object must remain valid for the lifetime of the
 * animation.
 */
struct wlf_animation_impl {
	/**
	 * @brief Start type-specific animation processing.
	 * @param animation Animation being started.
	 */
	void (*start)(struct wlf_animation *animation);

	/**
	 * @brief Stop type-specific animation processing.
	 * @param animation Animation being stopped.
	 */
	void (*stop)(struct wlf_animation *animation);

	/**
	 * @brief Pause type-specific animation processing.
	 * @param animation Animation being paused.
	 */
	void (*pause)(struct wlf_animation *animation);

	/**
	 * @brief Resume type-specific animation processing.
	 * @param animation Animation being resumed.
	 */
	void (*resume)(struct wlf_animation *animation);

	/**
	 * @brief Update type-specific animation state.
	 * @param animation Animation being updated.
	 * @param dt Elapsed time in milliseconds since the previous update.
	 */
	void (*update)(struct wlf_animation *animation, int64_t dt);

	/**
	 * @brief Apply the current interpolated value to the animation target.
	 * @param animation Animation whose value is being applied.
	 * @return true on success, false on failure.
	 */
	bool (*write_back)(struct wlf_animation *animation);

	/**
	 * @brief Synchronize state before the animation starts.
	 * @param animation Animation being prepared.
	 */
	void (*pre_sync)(struct wlf_animation *animation);

	/**
	 * @brief Synchronize state after the animation stops or finishes.
	 * @param animation Animation whose final state is being synchronized.
	 */
	void (*post_sync)(struct wlf_animation *animation);

	/**
	 * @brief Destroy the concrete animation.
	 * @param animation Animation to destroy.
	 */
	void (*destroy)(struct wlf_animation *animation);
};

/**
 * @brief Listener for animation lifecycle events.
 *
 * The listener is not owned by the animation and must remain valid until it is
 * replaced, removed, or the animation is destroyed. The destroy callback runs
 * before the concrete animation is destroyed.
 */
struct wlf_animation_listener {
	/** Called before the animation is destroyed. */
	void (*destroy)(void *data, struct wlf_animation *animation);
	/** Called after the animation has started. */
	void (*started)(void *data, struct wlf_animation *animation);
	/** Called after the animation has been explicitly stopped. */
	void (*stopped)(void *data, struct wlf_animation *animation);
	/** Called after the final iteration completes. */
	void (*finished)(void *data, struct wlf_animation *animation);
	/** Called after a running animation has been paused. */
	void (*paused)(void *data, struct wlf_animation *animation);
	/** Called after a paused animation has resumed. */
	void (*resumed)(void *data, struct wlf_animation *animation);
};

/**
 * @brief Base object embedded in every animation type.
 *
 * Concrete animation types must embed this structure as their first member.
 */
struct wlf_animation {
	const struct wlf_animation_impl *impl; /**< Concrete animation implementation. */

	struct wlf_curve *curve; /**< Owned easing curve. */
	struct {
		const struct wlf_animation_listener *listener; /**< Registered listener. */
		void *user_data; /**< User data passed to listener callbacks. */
	} WLF_PRIVATE;

	int64_t duration; /**< Duration of one iteration in milliseconds. */
	int64_t current_time; /**< Elapsed time in the current iteration, in milliseconds. */
	enum wlf_animation_state state; /**< Current animation state. */

	int loop_count; /**< Total iterations, or WLF_ANIMATION_LOOP_INFINITE. */
	int current_loop; /**< Number of completed iterations. */
	enum wlf_animation_direction direction; /**< Playback direction. */
	bool alternate_reverse; /**< Whether the current alternating iteration is reversed. */
	bool paused_pending; /**< Whether a pause operation is pending. */
	int64_t pause_time; /**< Elapsed iteration time recorded when paused. */

	struct wlf_linked_list link; /**< Link for membership in an animation list. */
};

/**
 * @brief Initialize a base animation in the stopped state.
 *
 * A linear easing curve is created and owned by the animation.
 *
 * @param animation Animation to initialize.
 * @param impl Implementation hooks that remain valid for the animation lifetime.
 */
void wlf_animation_init(struct wlf_animation *animation,
	const struct wlf_animation_impl *impl);

/**
 * @brief Destroy an animation and its easing curve.
 *
 * @param animation Animation to destroy. NULL is accepted.
 */
void wlf_animation_destroy(struct wlf_animation *animation);

/**
 * @brief Register a listener for animation lifecycle events.
 *
 * The animation does not take ownership of @p listener. Only one listener can
 * be registered at a time; registering another listener replaces the previous
 * one. Passing NULL removes the currently registered listener.
 *
 * @param animation Animation to monitor.
 * @param listener Listener owned by the caller, or NULL to remove it.
 * @param data User data passed to every listener callback.
 */
void wlf_animation_add_listener(struct wlf_animation *animation,
	const struct wlf_animation_listener *listener, void *data);

/**
 * @brief Start or restart an animation from its first iteration.
 *
 * @param animation Animation to start.
 */
void wlf_animation_start(struct wlf_animation *animation);

/**
 * @brief Stop an animation and reset its iteration progress.
 *
 * This function has no effect if the animation is already stopped.
 *
 * @param animation Animation to stop.
 */
void wlf_animation_stop(struct wlf_animation *animation);

/**
 * @brief Pause a running animation.
 *
 * This function has no effect unless the animation is running.
 *
 * @param animation Animation to pause.
 */
void wlf_animation_pause(struct wlf_animation *animation);

/**
 * @brief Resume a paused animation.
 *
 * This function has no effect unless the animation is paused.
 *
 * @param animation Animation to resume.
 */
void wlf_animation_resume(struct wlf_animation *animation);

/**
 * @brief Advance a running animation.
 *
 * This function updates the concrete animation and applies its value. It also
 * advances loop state and finishes the animation after its final iteration. A
 * zero-duration animation completes immediately; a negative duration is
 * ignored.
 *
 * @param animation Animation to update.
 * @param dt Elapsed time in milliseconds since the previous update.
 */
void wlf_animation_update(struct wlf_animation *animation, int64_t dt);

/**
 * @brief Set the duration of one animation iteration.
 *
 * @param animation Animation to configure.
 * @param duration Duration in milliseconds.
 */
void wlf_animation_set_duration(struct wlf_animation *animation, int64_t duration);

/**
 * @brief Replace the animation's easing curve.
 *
 * The current curve is destroyed. The animation takes ownership of @p curve.
 *
 * @param animation Animation to configure.
 * @param curve Easing curve whose ownership is transferred to the animation.
 */
void wlf_animation_set_curve(struct wlf_animation *animation,
	struct wlf_curve *curve);

/**
 * @brief Set the total number of animation iterations.
 *
 * @param animation Animation to configure.
 * @param count Positive iteration count, or WLF_ANIMATION_LOOP_INFINITE.
 */
void wlf_animation_set_loop_count(struct wlf_animation *animation, int count);

/**
 * @brief Set the animation playback direction.
 *
 * @param animation Animation to configure.
 * @param direction Playback direction.
 */
void wlf_animation_set_direction(struct wlf_animation *animation,
	enum wlf_animation_direction direction);

/**
 * @brief Evaluate the easing curve at the current animation position.
 *
 * Playback direction is applied before evaluating the curve. Curves that
 * overshoot, such as back or elastic curves, may return values outside [0, 1].
 *
 * @param animation Animation to query.
 * @return Current eased progress, or 0 if the duration is not positive.
 */
float wlf_animation_get_progress(const struct wlf_animation *animation);

#endif // ANIMATION_WLF_ANIMATION_H
