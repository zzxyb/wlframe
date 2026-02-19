/**
 * @file        wlf_animator.h
 * @brief       Animation system interface in wlframe.
 * @details     This file provides a general abstraction for animators.
 *              It defines the base animator structure along with a virtual interface
 *              for controlling animations (start, stop, pause, resume, update).
 *              The animator system supports easing curves, loop control, direction control,
 *              and event signals. Different animator implementations (such as property animators,
 *              value animators, etc.) can be provided by implementing the `wlf_animator_impl` structure.
 * @author      YaoBing Xiao
 * @date        2026-02-19
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-19, initial version\n
 */

#ifndef ANIMATOR_WLF_ANIMATOR_H
#define ANIMATOR_WLF_ANIMATOR_H

#include "wlf/animator/wlf_curve.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_animator;

/**
 * @brief Animation state enumeration.
 *
 * Specifies the current state of an animation.
 */
enum wlf_animator_state {
	WLF_ANIMATOR_STATE_STOPPED,  /**< Animation is stopped */
	WLF_ANIMATOR_STATE_PAUSED,   /**< Animation is paused */
	WLF_ANIMATOR_STATE_RUNNING,  /**< Animation is running */
};

/**
 * @brief Animation loop count enumeration.
 *
 * Defines special loop count values. The loop_count field can accept
 * any positive integer (1, 2, 3, ...) to specify the number of iterations,
 * or use WLF_ANIMATOR_LOOP_INFINITE (-1) for infinite looping.
 */
enum wlf_animator_loop {
	WLF_ANIMATOR_LOOP_ONCE = 1,      /**< Run once (value=1 allows arbitrary positive integers for loop count) */
	WLF_ANIMATOR_LOOP_INFINITE = -1, /**< Loop infinitely (value=-1 as special sentinel) */
};

/**
 * @brief Animation direction enumeration.
 *
 * Specifies the direction in which the animation plays.
 */
enum wlf_animator_direction {
	WLF_ANIMATOR_DIRECTION_FORWARD,  /**< Animate forward (from start to end) */
	WLF_ANIMATOR_DIRECTION_BACKWARD, /**< Animate backward (from end to start) */
	WLF_ANIMATOR_DIRECTION_ALTERNATE,/**< Alternate direction on each loop iteration */
};

/**
 * @brief Virtual methods for animator operations.
 *
 * This structure defines the interface that animator implementations must provide.
 * Inspired by Qt's QAbstractAnimation virtual interface.
 */
struct wlf_animator_impl {
	/**
	 * @brief Starts the animation
	 * @param animator The animator instance
	 */
	void (*start)(struct wlf_animator *animator);

	/**
	 * @brief Stops the animation
	 * @param animator The animator instance
	 */
	void (*stop)(struct wlf_animator *animator);

	/**
	 * @brief Pauses the animation
	 * @param animator The animator instance
	 */
	void (*pause)(struct wlf_animator *animator);

	/**
	 * @brief Resumes the animation
	 * @param animator The animator instance
	 */
	void (*resume)(struct wlf_animator *animator);

	/**
	 * @brief Updates the animation
	 * @param animator The animator instance
	 * @param dt Delta time in milliseconds since last update
	 */
	void (*update)(struct wlf_animator *animator, int64_t dt);

	/**
	 * @brief Writes back the interpolated value
	 * @param animator The animator instance
	 * @return true if write back succeeded, false otherwise
	 */
	bool (*write_back)(struct wlf_animator *animator);

	/**
	 * @brief Prepares for synchronization before animation starts
	 * @param animator The animator instance
	 */
	void (*pre_sync)(struct wlf_animator *animator);

	/**
	 * @brief Finalizes synchronization after animation ends
	 * @param animator The animator instance
	 */
	void (*post_sync)(struct wlf_animator *animator);

	/**
	 * @brief Destroys the animator
	 * @param animator The animator instance
	 */
	void (*destroy)(struct wlf_animator *animator);
};

/**
 * @brief Base animator structure.
 *
 * All animator types must have this structure as their first member.
 * This design is inspired by Qt's QAbstractAnimation and provides
 * a common base for all animation types in wlframe.
 */
struct wlf_animator {
	const struct wlf_animator_impl *impl;  /**< Virtual method table */

	struct wlf_curve *curve;               /**< Animation easing curve (owned pointer) */
	int64_t duration;                      /**< Duration of the animation in milliseconds */
	int64_t current_time;                  /**< Current elapsed time in milliseconds */
	enum wlf_animator_state state;         /**< Current state of the animation */

	/* Loop control */
	int loop_count;                        /**< Number of times to loop (-1 for infinite, positive integers for specific count) */
	int current_loop;                      /**< Current loop iteration (0-based counter) */
	enum wlf_animator_direction direction; /**< Animation direction */
	bool alternate_reverse;                /**< Internal flag for alternate direction (used when direction is ALTERNATE) */

	/* Events */
	struct {
		struct wlf_signal destroy;     /**< Signal emitted when animator is destroyed */
		struct wlf_signal started;     /**< Signal emitted when animation starts */
		struct wlf_signal stopped;     /**< Signal emitted when animation stops */
		struct wlf_signal finished;    /**< Signal emitted when animation completes all loops */
		struct wlf_signal paused;      /**< Signal emitted when animation pauses */
		struct wlf_signal resumed;     /**< Signal emitted when animation resumes */
		struct wlf_signal updated;     /**< Signal emitted on each update tick */
	} events;

	/* Timing control */
	bool paused_pending;                   /**< Pause operation is pending */
	int64_t pause_time;                    /**< Time when animation was paused (in milliseconds) */
};

/**
 * @brief Initializes a base animator.
 *
 * @param animator The animator to initialize.
 * @param impl Implementation methods for this animator.
 */
void wlf_animator_init(struct wlf_animator *animator,
	const struct wlf_animator_impl *impl);

/**
 * @brief Destroys a base animator.
 *
 * @param animator The animator to destroy.
 */
void wlf_animator_destroy(struct wlf_animator *animator);

/**
 * @brief Starts the animation.
 *
 * @param animator The animator to start.
 */
void wlf_animator_start(struct wlf_animator *animator);

/**
 * @brief Stops the animation.
 *
 * @param animator The animator to stop.
 */
void wlf_animator_stop(struct wlf_animator *animator);

/**
 * @brief Pauses the animation.
 *
 * @param animator The animator to pause.
 */
void wlf_animator_pause(struct wlf_animator *animator);

/**
 * @brief Resumes the animation.
 *
 * @param animator The animator to resume.
 */
void wlf_animator_resume(struct wlf_animator *animator);

/**
 * @brief Updates the animation.
 *
 * @param animator The animator to update.
 * @param dt Delta time in milliseconds since last update.
 */
void wlf_animator_update(struct wlf_animator *animator, int64_t dt);

/**
 * @brief Sets animation duration.
 *
 * @param animator The animator.
 * @param duration Duration in milliseconds.
 */
void wlf_animator_set_duration(struct wlf_animator *animator, int64_t duration);

/**
 * @brief Sets animation easing curve.
 *
 * @param animator The animator.
 * @param curve The easing curve (takes ownership).
 */
void wlf_animator_set_curve(struct wlf_animator *animator,
	struct wlf_curve *curve);

/**
 * @brief Sets loop count.
 *
 * @param animator The animator.
 * @param count Number of loops (use WLF_ANIMATOR_LOOP_INFINITE for infinite, or any positive integer for specific count).
 */
void wlf_animator_set_loop_count(struct wlf_animator *animator, int count);

/**
 * @brief Sets animation direction.
 *
 * @param animator The animator.
 * @param direction The animation direction.
 */
void wlf_animator_set_direction(struct wlf_animator *animator,
	enum wlf_animator_direction direction);

/**
 * @brief Gets current animation progress.
 *
 * @param animator The animator.
 * @return Progress value in range [0.0, 1.0].
 */
float wlf_animator_get_progress(const struct wlf_animator *animator);

#endif // ANIMATOR_WLF_ANIMATOR_H
