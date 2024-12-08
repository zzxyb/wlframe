#ifndef WLF_SIGNAL_H
#define WLF_SIGNAL_H

#include "wlf_double_list.h"

struct wlf_listener;

/**
 * Function type for signal notification callbacks.
 *
 * This is the function signature for callbacks that will be triggered when
 * a signal is emitted. The callback receives the listener that caught the
 * signal and the data passed during emission.
 *
 * \param listener The listener that received the event
 * \param data User data passed during signal emission
 */
typedef void (*wlf_notify_func_t)(struct wlf_listener *listener, void *data);

/**
 * A single listener for signal notifications.
 *
 * Provides the means to listen for signal notifications. Listeners can be
 * registered to signals using wlf_signal_add() and will receive notifications
 * of significant events.
 *
 * Example usage:
 *
 * \code
 * struct wlf_listener listener;
 *
 * // Setup listener
 * listener.notify = your_callback_method;
 *
 * // Add to signal
 * wlf_signal_add(&some_signal, &listener);
 * \endcode
 *
 * If the listener is part of a larger struct:
 *
 * \code
 * void your_callback(struct wlf_listener *listener, void *data)
 * {
 *     struct your_data *data;
 *     data = wlf_container_of(listener, data, listener_member);
 * }
 * \endcode
 *
 * \note A listener can only listen to one signal at a time
 */
struct wlf_listener {
	struct wlf_double_list link;
	wlf_notify_func_t notify;
};

/**
 * Signal implementation for event notification.
 *
 * This signal implementation allows objects to emit events that listeners can
 * observe. Listeners can be added using wlf_signal_add() and the signal can
 * be emitted using wlf_signal_emit(), which will notify all registered listeners.
 *
 * Example usage:
 *
 * \code
 * struct wlf_signal sig;
 * struct wlf_listener listener;
 *
 * // Initialize signal
 * wlf_signal_init(&sig);
 *
 * // Setup listener
 * listener.notify = callback_function;
 * wlf_signal_add(&sig, &listener);
 *
 * // Emit signal
 * wlf_signal_emit(&sig, data);
 * \endcode
 */
struct wlf_signal {
	struct wlf_double_list listener_list;
};

/**
 * Initializes an empty signal.
 *
 * \param signal Signal to initialize
 */
void wlf_signal_init(struct wlf_signal *signal);

/**
 * Adds a listener to the signal.
 *
 * \param signal Signal to add listener to
 * \param listener Listener to add
 *
 * \note Adding a listener that's already in a signal will corrupt both signals
 */
void wlf_signal_add(struct wlf_signal *signal, struct wlf_listener *listener);

/**
 * Gets the listener with the specified callback.
 *
 * \param signal Signal to search in
 * \param notify Callback function to find
 * \return Matching listener or NULL if not found
 */
struct wlf_listener *wlf_signal_get(
	struct wlf_signal *signal, wlf_notify_func_t notify);

/**
 * Emits the signal to all listeners.
 *
 * \param signal Signal to emit
 * \param data Data to pass to listeners
 */
void wlf_signal_emit(struct wlf_signal *signal, void *data);

/**
 * Emits the signal with mutable data to all listeners.
 *
 * \param signal Signal to emit
 * \param data Mutable data to pass to listeners
 */
void wlf_signal_emit_mutable(struct wlf_signal *signal, void *data);

#endif
