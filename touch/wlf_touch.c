#include "wlf/touch/wlf_touch.h"
#include "wlf/utils/wlf_log.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define WLF_PI 3.14159265

typedef struct wlf_touch_target {
	double x;
	double y;
	double w;
	double h;
} wlf_touch_target;

typedef struct touch_data {
	int slot;
	double startx;
	double starty;
	double curx;
	double cury;
} touch_data;

static double distance_dragged(touch_data *d) {
	return sqrt(
		pow(d->startx - d->curx, 2) +
		pow(d->starty - d->cury, 2)
	);
}

static enum wlf_touch_move_dir direction_dragged(touch_data *d) {
	double dx = d->startx - d->curx;
	double dy = d->starty - d->cury;

	enum wlf_touch_move_dir direction = 0;

	if (dx > 0) {
		direction |= WLF_TOUCH_MOVE_POSITIVE_X;
	} else if (dx < 0) {
		direction |= WLF_TOUCH_MOVE_NEGATIVE_X;
	}

	if (dy > 0) {
		direction |= WLF_TOUCH_MOVE_POSITIVE_Y;
	} else if (dy < 0) {
		direction |= WLF_TOUCH_MOVE_NEGATIVE_Y;
	}

	return direction;
}

static double get_incorrect_drag_distance(touch_data *d,
					   enum wlf_touch_move_dir direction) {
	double incorrect_squared = 0;

	double dx = d->curx - d->startx;
	double dy = d->cury - d->starty;

	if ((direction & WLF_TOUCH_MOVE_POSITIVE_X) != 0) {
		if (dx < 0) {
			incorrect_squared += pow(dx, 2);
		}
	} else if ((direction & WLF_TOUCH_MOVE_NEGATIVE_X) != 0) {
		if (dx > 0) {
			incorrect_squared += pow(dx, 2);
		}
	} else {
		// Stationary in X
		incorrect_squared += pow(dx, 2);
	}

	if ((direction & WLF_TOUCH_MOVE_POSITIVE_Y) != 0) {
		if (dy < 0) {
			incorrect_squared += pow(dy, 2);
		}
	} else if ((direction & WLF_TOUCH_MOVE_NEGATIVE_Y) != 0) {
		if (dy > 0) {
			incorrect_squared += pow(dy, 2);
		}
	} else {
		// Stationary in Y
		incorrect_squared += pow(dy, 2);
	}
	return sqrt(incorrect_squared);
}

typedef struct touch_list {
	touch_data data;
	struct touch_list *next;
} touch_list;

static touch_list *get_touch(touch_list *touch, int slot) {
	touch_list *ptr = touch;
	while (ptr != NULL) {
		if (ptr->data.slot == slot) {
			return ptr;
		}
	return NULL;
}

static void remove_touch(touch_list **touch, int slot) {
	if (*touch == NULL) {
		return;
	}

	if ((*touch)->data.slot == slot) {
		touch_list *tmp = *touch;
		(*touch) = (*touch)->next;
		free(tmp);
		return;
	}

	remove_touch(&(*touch)->next, slot);
}

static touch_data *get_touch_center(touch_list *touches) {
	int count = 0;
	touch_data *res = calloc(1, sizeof(touch_data));
	touch_list *iter = touches;

	while (iter != NULL) {
		count++;
		res->startx += iter->data.startx;
		res->starty += iter->data.starty;
		res->curx += iter->data.curx;
		res->cury += iter->data.cury;
		iter = iter->next;
	}

	res->curx /= count;
	res->cury /= count;
	res->startx /= count;
	res->starty /= count;

	return res;
}

static double get_pinch_scale(touch_list *touches) {
	int count = 0;
	touch_data *center = get_touch_center(touches);

	touch_list *iter = touches;
	double old = 0;
	double new = 0;
	while (iter != NULL) {
		count++;

		old += sqrt(
			pow(center->startx - iter->data.startx, 2) +
			pow(center->starty - iter->data.starty, 2));
		new += sqrt(
			pow(center->curx - iter->data.curx, 2) +
			pow(center->cury - iter->data.cury, 2));
		iter = iter->next;
	}
	old /= count;
	new /= count;

	free(center);
	return new / old;
}

static double get_rotate_angle(touch_list *touches) {
	int count = 0;
	touch_data *center = get_touch_center(touches);
	touch_list *iter = touches;
	double old = 0;
	double new = 0;
	while (iter != NULL) {
		count++;
		old += atan2(iter->data.startx - center->startx,
			     iter->data.starty - center->starty);

		new += atan2(iter->data.curx - center->curx,
			     iter->data.cury - center->cury);

		iter = iter->next;
	}

	old /= count;
	new /= count;

	return (new - old) * 180.0 / WLF_PI;
}

typedef struct wlf_touch_action {
	enum wlf_touch_action_type action_type;
	double move_tolerance;
	wlf_touch_target *target;
	int threshold;
	uint32_t duration_ms;
	union {
		// Touch Action
		struct {
			enum wlf_touch_mode mode;
		} touch;
		// Move Action
		struct {
			enum wlf_touch_move_dir dir;
		} move;
		// Rotate Action
		struct {
			enum wlf_touch_rotate_dir dir;
		} rotate;
		// Pinch Action
		struct {
			enum wlf_touch_scale_dir dir;
		} pinch;
		// Delay Action
		struct {
			// Empty
		} delay;
	};
} wlf_touch_action;

typedef struct wlf_touch_gesture {
	wlf_touch_action **actions;
	uint32_t n_actions;
} wlf_touch_gesture;

typedef struct wlf_touch_engine {
	wlf_touch_gesture **gestures;
	uint32_t n_gestures;

	wlf_touch_target **targets;
	uint32_t n_targets;
} wlf_touch_engine;

typedef struct wlf_touch_gesture_progress {
	wlf_touch_gesture *gesture;
	uint32_t completed_actions;
	uint32_t last_action_timestamp;

	double action_progress;

	touch_list *touches;
} wlf_touch_gesture_progress;

typedef struct wlf_touch_progress_tracker {
	wlf_touch_gesture_progress *gesture_progress;
	uint32_t n_gestures;
} wlf_touch_progress_tracker;

struct wlf_touch_engine *wlf_touch_engine_create(void) {
	wlf_touch_engine *e = malloc(sizeof(wlf_touch_engine));
	if (e == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate touch engine");
		return NULL;
	}
	e->targets = NULL;
	e->n_targets = 0;
	e->gestures = NULL;
	e->n_gestures = 0;
	return e;
}

struct wlf_touch_progress_tracker *wlf_touch_progress_tracker_create(
			  struct wlf_touch_engine *engine) {
	wlf_touch_progress_tracker *t =
		calloc(sizeof(wlf_touch_progress_tracker), 1);

	if (t == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate progress tracker");
		return NULL;
	}

	t->gesture_progress = calloc(sizeof(wlf_touch_gesture_progress),
				     engine->n_gestures);

	for (int i = 0; i < engine->n_gestures; i++) {
		t->gesture_progress[i].gesture = engine->gestures[i];
	}

	t->n_gestures = engine->n_gestures;

	return t;
}

uint32_t wlf_touch_progress_tracker_n_gestures(
		struct wlf_touch_progress_tracker *tracker) {
	return tracker->n_gestures;
}

struct wlf_touch_gesture *wlf_touch_gesture_create(
		struct wlf_touch_engine *engine) {
	// Increase gesture array size.
	wlf_touch_gesture **gestures = malloc(sizeof(wlf_touch_gesture *)
					     * (1 + engine->n_gestures));

	if (gestures == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate gestures array");
		return NULL;
	}

	memcpy(gestures, engine->gestures, sizeof(wlf_touch_gesture *)
	       * engine->n_gestures);

	free(engine->gestures);
	engine->gestures = gestures;
	engine->n_gestures++;

	// Add the gesture
	wlf_touch_gesture *gesture = malloc(sizeof(wlf_touch_gesture));
	if (gesture == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate gesture");
		return NULL;
	}
	gesture->actions = NULL;
	gesture->n_actions = 0;
	gestures[engine->n_gestures - 1] = gesture;

	return gesture;
}

void wlf_touch_action_move_tolerance(struct wlf_touch_action *action,
				     double min) {
	action->move_tolerance = min;
}

static void wlf_touch_gesture_move_tolerance(wlf_touch_gesture *gesture,
					     double min) {
	for (int i = 0; i < gesture->n_actions; i++) {
		wlf_touch_action_move_tolerance(gesture->actions[i], min);
	}
}

static void wlf_touch_engine_move_tolerance(wlf_touch_engine *engine,
					    double min) {
	for (int i = 0; i < engine->n_gestures; i++) {
		wlf_touch_gesture_move_tolerance(engine->gestures[i], min);
	}
}

struct wlf_touch_target *wlf_touch_target_create(
		struct wlf_touch_engine *engine,
		double x, double y,
		double width, double height) {
	// Increase array size
	wlf_touch_target **arr = malloc(sizeof(wlf_touch_target *)
				       * (1 + engine->n_targets));

	if (arr == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate targets array");
		return NULL;
	}

	memcpy(arr, engine->targets,
	       sizeof(wlf_touch_target *) * engine->n_targets);

	free(engine->targets);
	engine->targets = arr;
	engine->n_targets++;

	wlf_touch_target *t = malloc(sizeof(wlf_touch_target));
	if (t == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate target");
		return NULL;
	}
	t->x = x;
	t->y = y;
	t->w = width;
	t->h = height;
	engine->targets[engine->n_targets - 1] = t;
	return t;
}


static bool wlf_touch_target_contains(wlf_touch_target *target,
				      double x, double y) {
	return target == NULL ||
		(x > target->x &&
		 x < (target->x + target->w) &&
		 y > target->y &&
		 y < (target->y + target->h));
}

void wlf_touch_progress_register_touch(
		struct wlf_touch_progress_tracker *tracker,
		uint32_t timestamp, int slot,
		enum wlf_touch_mode mode,
		double x, double y) {
	wlf_touch_gesture *g;
	wlf_touch_action *a;
	wlf_touch_gesture_progress *p;
	for (int i = 0; i < tracker->n_gestures; i++) {
		p = &tracker->gesture_progress[i];
		g = p->gesture;
		if (p->completed_actions == g->n_actions) {
			// Gesture already completed, but not yet handled.
			continue;
		}
		a = g->actions[p->completed_actions];

		if ((p->completed_actions == 0 ||
		     a->duration_ms > (timestamp - p->last_action_timestamp)) &&
		    a->action_type == WLF_TOUCH_ACTION_TOUCH &&
		    (a->touch.mode & mode) == mode &&
		    wlf_touch_target_contains(a->target, x, y)) {

			p->action_progress += 1.0 / ((double)a->threshold);

			if (mode == WLF_TOUCH_DOWN) {
				touch_list *tl = malloc(sizeof(touch_list));
				if (tl == NULL) {
					wlf_log_errno(WLF_ERROR, "Failed to allocate touch list");
					continue;
				}
				tl->next = p->touches;
				tl->data.slot = slot;
				tl->data.startx = x;
				tl->data.starty = y;
				tl->data.curx = x;
				tl->data.cury = y;

				p->touches = tl;
			} else {
				remove_touch(&p->touches, slot);
			}

			if (p->action_progress > 0.9) {
				p->action_progress = 0;
				p->completed_actions++;
				p->last_action_timestamp = timestamp;
			}

		} else {
			wlf_touch_gesture_reset_progress(p);
		}
	}
}

static touch_data *get_touch_slot(wlf_touch_gesture_progress *g, int slot) {
	touch_list *t = g->touches;
	while (t != NULL && t->data.slot != slot) {
		t = t->next;
	}
	if (t == NULL) {
		return NULL;
	}
	return &t->data;
}

void wlf_touch_progress_register_move(
		struct wlf_touch_progress_tracker *tracker,
		uint32_t timestamp, int slot,
		double nx, double ny) {
	wlf_touch_gesture_progress *p;
	wlf_touch_gesture *g;
	wlf_touch_action *a;
	touch_data *avg;
	for (int i = 0; i < tracker->n_gestures; i++) {
		p = &tracker->gesture_progress[i];
		g = p->gesture;
		if (p->completed_actions == g->n_actions) {
			// Gesture already completed
			continue;
		}

		a = g->actions[p->completed_actions];

		touch_data *td = get_touch_slot(p, slot);
		if (td == NULL) {
			return;
		}
		td->curx = nx;
		td->cury = ny;

		avg = get_touch_center(p->touches);

		if (a->duration_ms < (timestamp - p->last_action_timestamp)) {
			// Timeout
			wlf_touch_gesture_reset_progress(p);
			free(avg);
			continue;
		}

		double rot, scl, distance, wrong, threshold;

		switch (a->action_type) {
		case WLF_TOUCH_ACTION_TOUCH:
		case WLF_TOUCH_ACTION_DELAY:
			if (distance_dragged(td) > a->move_tolerance) {
				wlf_touch_gesture_reset_progress(p);
			}
			break;
		case WLF_TOUCH_ACTION_MOVE:
			if (a->target != NULL) {
				if (wlf_touch_target_contains(
					   a->target, avg->curx, avg->cury)) {
					p->completed_actions++;
					p->action_progress = 0;
				}
			} else {
				// TODO: Handle movement in direction.
				distance = distance_dragged(avg);
				wrong = get_incorrect_drag_distance(
					avg, a->move.dir);
				if (wrong > a->move_tolerance) {
					wlf_touch_gesture_reset_progress(p);
				} else {
					p->action_progress = (distance - wrong) /
						a->threshold;
					if (p->action_progress > 1) {
						p->completed_actions++;
						p->action_progress = 0;
					}
				}
			}
			break;
		case WLF_TOUCH_ACTION_PINCH:
			distance = distance_dragged(avg);
			if (distance > a->move_tolerance) {
				wlf_touch_gesture_reset_progress(p);
			} else {
			threshold = ((double)a->threshold) / 100.0;
				scl = get_pinch_scale(p->touches);
				if (a->pinch.dir == WLF_TOUCH_PINCH_OUT) {
					p->action_progress =
						(scl - 1.0) / (threshold - 1.0);
				} else {
					p->action_progress =
						1.0 - (scl - threshold) /
						(1.0 - threshold);
				}
				p->action_progress *= 100;
				if (p->action_progress > 0.9) {
					p->completed_actions++;
					p->action_progress = 0;
				}
			}
			break;
		case WLF_TOUCH_ACTION_ROTATE:
			distance = distance_dragged(avg);
			if (distance > a->move_tolerance) {
				wlf_touch_gesture_reset_progress(p);
			} else {
				rot = get_rotate_angle(p->touches);
				if (rot > a->threshold) {
					p->completed_actions++;
					p->action_progress = 0;
				}
			}
			break;
		}
		free(avg);
	}
}

static void wlf_touch_add_action(wlf_touch_gesture *gesture,
		wlf_touch_action *action) {

	wlf_touch_action **new_array = malloc(sizeof(wlf_touch_action *)
					     * (gesture->n_actions + 1));

	if (new_array == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate actions array");
		return;
	}

	memcpy(new_array, gesture->actions,
	       sizeof(wlf_touch_action *) * (gesture->n_actions));

	free(gesture->actions);
	gesture->actions = new_array;
	gesture->actions[gesture->n_actions] = action;
	gesture->n_actions++;
}

static wlf_touch_action *create_action(void) {
	wlf_touch_action *action = malloc(sizeof(wlf_touch_action));
	if (action == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate action");
		return NULL;
	}
	action->duration_ms = 2000;
	action->target = NULL;
	action->move_tolerance = INFINITY;
	action->threshold = 1;
	return action;
}

struct wlf_touch_action *wlf_touch_gesture_add_touch(
		       struct wlf_touch_gesture *gesture,
		       uint32_t mode) {
	wlf_touch_action *action = create_action();
	if (action == NULL) {
		return NULL;
	}
	action->action_type = WLF_TOUCH_ACTION_TOUCH;
	action->touch.mode = mode;
	wlf_touch_add_action(gesture, action);
	return action;
}

struct wlf_touch_action *wlf_touch_gesture_add_move(
       		       struct wlf_touch_gesture *gesture,
		       uint32_t direction) {
	wlf_touch_action *action = create_action();
	if (action == NULL) {
		return NULL;
	}
	action->action_type = WLF_TOUCH_ACTION_MOVE;
	action->move.dir = direction;
	wlf_touch_add_action(gesture, action);
	return action;
}

struct wlf_touch_action *wlf_touch_gesture_add_rotate(
       		       struct wlf_touch_gesture *gesture,
		       uint32_t direction) {
	wlf_touch_action *action = create_action();
	if (action == NULL) {
		return NULL;
	}
	action->action_type = WLF_TOUCH_ACTION_ROTATE;
	action->rotate.dir = direction;
	wlf_touch_add_action(gesture, action);
	return action;
}

struct wlf_touch_action *wlf_touch_gesture_add_pinch(
       		       struct wlf_touch_gesture *gesture,
		       uint32_t direction) {
	wlf_touch_action *action = create_action();
	if (action == NULL) {
		return NULL;
	}
	action->action_type = WLF_TOUCH_ACTION_PINCH;
	action->pinch.dir = direction;
	wlf_touch_add_action(gesture, action);
	return action;
}

struct wlf_touch_action *wlf_touch_gesture_add_delay(
       		       struct wlf_touch_gesture *gesture,
		       uint32_t duration) {
	wlf_touch_action *action = create_action();
	if (action == NULL) {
		return NULL;
	}
	action->action_type = WLF_TOUCH_ACTION_DELAY;
	wlf_touch_add_action(gesture, action);
	return action;
}

void wlf_touch_action_set_threshold(struct wlf_touch_action *action,
				    int threshold) {
	action->threshold = threshold;
}

void wlf_touch_action_set_target(struct wlf_touch_action *action,
				 struct wlf_touch_target *target) {
	action->target = target;
}


void wlf_touch_action_set_duration(struct wlf_touch_action *action,
				  uint32_t duration_ms) {
	action->duration_ms = duration_ms;
}

double wlf_touch_gesture_progress_get_progress(
		struct wlf_touch_gesture_progress *gesture) {
	double n_actions = ((double)gesture->gesture->n_actions);
	double n_complete = ((double)gesture->completed_actions);
	double current_pr = ((double)gesture->action_progress);
	return (n_complete + current_pr) / n_actions;
}

void wlf_touch_gesture_reset_progress(
		struct wlf_touch_gesture_progress *progress) {
	while (progress->touches != NULL) {
		touch_list *l = progress->touches;
		progress->touches = l->next;
		free(l);
	}
	progress->completed_actions = 0;
	progress->action_progress = 0;
}

struct wlf_touch_gesture_progress *wlf_touch_gesture_get_progress(
		struct wlf_touch_progress_tracker *tracker,
		uint32_t index) {
	if (index > tracker->n_gestures - 1) {
		return NULL;
	}

	return &tracker->gesture_progress[index];
}

struct wlf_touch_action *wlf_touch_gesture_get_current_action(
		struct wlf_touch_gesture_progress *progress) {
	return progress->gesture->actions[progress->completed_actions];
}

struct wlf_touch_gesture *wlf_touch_handle_finished_gesture(
		 struct wlf_touch_progress_tracker *tracker) {
	for (int i = 0; i < tracker->n_gestures; i++) {
		if (wlf_touch_gesture_progress_get_progress(
				&tracker->gesture_progress[i]) > 0.9) {
			wlf_touch_gesture_reset_progress(
				&tracker->gesture_progress[i]);
			return tracker->gesture_progress[i].gesture;
		}
	}
	return NULL;
}
