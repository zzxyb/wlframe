#include "wlf/va/wlf_queue.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <time.h>

struct wlf_queue *wlf_queue_create(uint32_t max) {
	struct wlf_queue *queue = calloc(1, sizeof(struct wlf_queue));
	if (!queue) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_queue failed!");
		return NULL;
	}

	if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
		wlf_log(WLF_ERROR, "pthread_mutex_init failed!");
		goto failed;
	}
	if (pthread_cond_init(&queue->cond_not_empty, NULL) != 0) {
		wlf_log(WLF_ERROR, "pthread_cond_init for cond_not_empty failed!");
		goto failed;
	}
	if (pthread_cond_init(&queue->cond_not_full, NULL) != 0) {
		wlf_log(WLF_ERROR, "pthread_cond_init for cond_not_full failed!");
		goto failed;
	}

    queue->count = 0;
	queue->max = max;

	return queue;

failed:
	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->cond_not_full);
	pthread_cond_destroy(&queue->cond_not_empty);
	free(queue);

	return NULL;
}

void wlf_queue_destroy(struct wlf_queue *queue) {
	if (!queue) {
		return;
	}

	wlf_queue_flush(queue);

	if (queue->listener != NULL && queue->listener->destroy != NULL) {
		queue->listener->destroy(queue, queue->listener_data);
	}

	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->cond_not_full);
	pthread_cond_destroy(&queue->cond_not_empty);

	free(queue);
}

void wlf_queue_add_listener(struct wlf_queue *queue, const struct wlf_queue_listener *impl, void *data) {
	queue->listener = impl;
	queue->listener_data = data;
}

void *wlf_queue_get_listener_data(struct wlf_queue *queue) {
	return queue->listener_data;
}

void wlf_queue_set_user_data(struct wlf_queue *queue, void *data) {
	queue->user_data = data;
}

void *wlf_queue_get_user_data(struct wlf_queue *queue) {
	return queue->user_data;
}

void wlf_queue_flush(struct wlf_queue *queue) {
	struct wlf_queue_element *e = NULL;
	struct wlf_queue_element *n = NULL;
	pthread_mutex_lock(&queue->mutex);
	e = queue->first;
	while (e) {
		if (queue->listener != NULL && queue->listener->destroy != NULL) {
			queue->listener->destroy(queue, queue->listener_data);
		}

		n = e->next;
		free(e);
		e = n;
    }
	queue->first = NULL;
	queue->last = NULL;
	queue->count = 0;

	pthread_cond_signal(&queue->cond_not_full);
	pthread_mutex_unlock(&queue->mutex);
}

void wlf_queue_push(struct wlf_queue *queue, void *data) {
    struct wlf_queue_element *e = (struct wlf_queue_element *)malloc(sizeof(struct wlf_queue_element));
    if (e == NULL) {
		wlf_assert(e, "Allocation struct wlf_queue_element failed!");
		return;
	}

    e->data = data;
    e->next = NULL;
    pthread_mutex_lock(&queue->mutex);
    if (queue->max) {
        while (queue->count == (queue->max - 1))
            pthread_cond_wait(&queue->cond_not_full, &queue->mutex);
    }
    if (queue->first == NULL)
        queue->first = e;
    else
        queue->last->next = e;
    queue->last = e;
    ++queue->count;
    pthread_cond_signal(&queue->cond_not_empty);
    pthread_mutex_unlock(&queue->mutex);
}

void *wlf_queue_pop(struct wlf_queue *queue) {
	void* data;
    struct wlf_queue_element* p;
    pthread_mutex_lock(&queue->mutex);
    while (queue->count == 0) {
        pthread_cond_wait(&queue->cond_not_empty, &queue->mutex);
	}

    p = queue->first;
    data = queue->first->data;
    queue->first = queue->first->next;
    if (queue->first == NULL) {
        queue->last = NULL;
	}

    --queue->count;
    pthread_cond_signal(&queue->cond_not_full);
    pthread_mutex_unlock(&queue->mutex);
    free(p);

    return data;
}
