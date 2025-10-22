#ifndef VA_WLF_QUEUE_H
#define VA_WLF_QUEUE_H

#include <stdint.h>
#include <pthread.h>

struct wlf_queue;

struct wlf_queue_listener {
	void (*destroy)(struct wlf_queue *queue, void *data);
};

struct wlf_queue_element {
	void *data;
	struct wlf_queue_element *next;
};

struct wlf_queue {
	const struct wlf_queue_listener *listener;

	struct wlf_queue_element *first;
	struct wlf_queue_element *last;
	pthread_cond_t cond_not_empty;
	pthread_cond_t cond_not_full;
	pthread_mutex_t mutex;
	uint32_t count;
	uint32_t max;

	void *user_data;

	struct {
		void *listener_data;
	} WLF_PRIVATE;
};

struct wlf_queue *wlf_queue_create(uint32_t max);
void wlf_queue_destroy(struct wlf_queue *queue);

void wlf_queue_add_listener(struct wlf_queue *queue, const struct wlf_queue_listener *impl, void *data);
void *wlf_queue_get_listener_data(struct wlf_queue *queue);

void wlf_queue_set_user_data(struct wlf_queue *queue, void *data);
void *wlf_queue_get_user_data(struct wlf_queue *queue);

void wlf_queue_flush(struct wlf_queue *queue);
void wlf_queue_push(struct wlf_queue *queue, void *data);
void *wlf_queue_pop(struct wlf_queue *queue);

#endif // VA_WLF_QUEUE_H
