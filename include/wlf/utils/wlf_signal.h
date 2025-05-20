/**
 * @file        wlf_signal.h
 * @brief       Signal and listener utility for event notification in wlframe.
 * @details     This file provides a lightweight signal/slot (observer) mechanism.
 *              It allows objects to emit events (signals) and other objects to listen for those events (listeners).
 *              Listeners can be registered to signals and will be notified when the signal is emitted.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef UTILS_WLF_SIGNAL_H
#define UTILS_WLF_SIGNAL_H

#include "wlf_linked_list.h"

struct wlf_listener;

/**
 * @brief Function type for signal notification callbacks.
 *
 * This function will be called when a signal is emitted.
 *
 * @param listener The listener that received the event.
 * @param data User data passed during signal emission.
 */
typedef void (*wlf_notify_func_t)(struct wlf_listener *listener, void *data);

/**
 * @brief A single listener for signal notifications.
 *
 * Listeners can be registered to signals using wlf_signal_add() and will receive notifications
 * when the signal is emitted.
 *
 * @note A listener can only listen to one signal at a time.
 *
 * @code
 * struct wlf_listener listener;
 * listener.notify = your_callback;
 * wlf_signal_add(&signal, &listener);
 * @endcode
 */
struct wlf_listener {
	struct wlf_linked_list link;   /**< List node for linking listeners */
	wlf_notify_func_t notify;      /**< Callback function */
};

/**
 * @brief Signal implementation for event notification.
 *
 * Allows objects to emit events that listeners can observe.
 *
 * @code
 * struct wlf_signal sig;
 * struct wlf_listener listener;
 * wlf_signal_init(&sig);
 * listener.notify = callback_function;
 * wlf_signal_add(&sig, &listener);
 * wlf_signal_emit(&sig, data);
 * @endcode
 */
struct wlf_signal {
	struct wlf_linked_list listener_list; /**< List of registered listeners */
};

/**
 * @brief Initializes an empty signal.
 *
 * @param signal Signal to initialize.
 */
void wlf_signal_init(struct wlf_signal *signal);

/**
 * @brief Adds a listener to the signal.
 *
 * @param signal Signal to add listener to.
 * @param listener Listener to add.
 * @note Adding a listener that's already in a signal will corrupt both signals.
 */
void wlf_signal_add(struct wlf_signal *signal, struct wlf_listener *listener);

/**
 * @brief Gets the listener with the specified callback.
 *
 * @param signal Signal to search in.
 * @param notify Callback function to find.
 * @return Matching listener or NULL if not found.
 */
struct wlf_listener *wlf_signal_get(
	struct wlf_signal *signal, wlf_notify_func_t notify);

/**
 * @brief Emits the signal to all listeners.
 *
 * @param signal Signal to emit.
 * @param data Data to pass to listeners.
 */
void wlf_signal_emit(struct wlf_signal *signal, void *data);

/**
 * @brief Emits the signal with mutable data to all listeners.
 *
 * @param signal Signal to emit.
 * @param data Mutable data to pass to listeners.
 */
void wlf_signal_emit_mutable(struct wlf_signal *signal, void *data);

#endif // UTILS_WLF_SIGNAL_H
