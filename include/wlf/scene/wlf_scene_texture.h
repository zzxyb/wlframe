#ifndef WLF_SCENE_SCENE_TEXTURE_H
#define WLF_SCENE_SCENE_TEXTURE_H

struct wlf_scene_node;
struct wlf_texture;

struct wlf_scene_image {
	struct wlf_scene_node *base; // pointer to the scene node
	struct wlf_texture *texture; // pointer to the texture
};

#endif
