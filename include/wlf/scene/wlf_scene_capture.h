#ifndef SCENE_WLF_SCENE_CAPTURE_H
#define SCENE_WLF_SCENE_CAPTURE_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

struct wlf_scene_node;
struct wlf_buffer;

struct wlf_scene_capture_damage_event {
  struct wlf_scene_capture *capture;
  struct wlf_scene_node *node;
  struct wlf_scene_node *origin;
  const struct wlf_region *damage;
};

struct wlf_scene_capture_frame {
  struct wlf_scene_capture *capture;
  struct wlf_buffer *buffer;
  size_t n_locks;
  uint64_t sequence;
  bool dropped;
  bool drop_buffer;
  struct wlf_region damage;
  struct timespec timestamp;

  struct {
    struct wlf_signal destroy;
  } events;
};

struct wlf_scene_capture_frame_event {
  struct wlf_scene_capture *capture;
  struct wlf_scene_capture_frame *frame;
};

struct wlf_scene_capture_output_options {
  struct wlf_buffer *buffer;
  const struct wlf_region *damage;
  const struct timespec *timestamp;
  bool drop_buffer;
};

struct wlf_scene_capture {
  struct wlf_scene_node *node;
  size_t n_locks;
  uint64_t sequence;
  struct wlf_scene_capture_frame *latest_frame;

  struct {
		struct wlf_signal destroy;
    struct wlf_signal damage;
    struct wlf_signal frame;
	} events;
};

struct wlf_scene_capture_frame *wlf_scene_capture_frame_lock(
  struct wlf_scene_capture_frame *frame);
void wlf_scene_capture_frame_unlock(struct wlf_scene_capture_frame *frame);
struct wlf_scene_capture_frame *wlf_scene_capture_get_latest_frame(
  struct wlf_scene_capture *capture);
bool wlf_scene_capture_output_frame(struct wlf_scene_capture *capture,
  const struct wlf_scene_capture_output_options *options);
void wlf_scene_capture_init(struct wlf_scene_capture *capture,
  struct wlf_scene_node *node);
void wlf_scene_capture_finish(struct wlf_scene_capture *capture);
struct wlf_scene_capture *wlf_scene_capture_lock(struct wlf_scene_capture *capture);
void wlf_scene_capture_unlock(struct wlf_scene_capture *capture);
void wlf_scene_capture_damage(struct wlf_scene_capture *capture,
  const struct wlf_region *damage);
bool wlf_scene_node_is_captured(const struct wlf_scene_node *node);
void wlf_scene_node_damage_capture(struct wlf_scene_node *node,
  const struct wlf_region *damage);
void wlf_scene_node_damage_whole_capture(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_CAPTURE_H