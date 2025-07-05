#ifndef RENDER_WLF_RENDER_H
#define RENDER_WLF_RENDER_H

#include <stdbool.h>
#include <wlf/math/wlf_matrix4x4.h>
#include <wlf/math/wlf_vector2.h>
#include <wlf/math/wlf_rect.h>

struct wlf_backend;
struct wlf_buffer;
struct wlf_texture;
struct wlf_render;

/**
 * @brief Renderer backend types
 */
enum wlf_render_type {
	WLF_RENDER_PIXMAN = 0,    /**< CPU software rendering with pixman */
	WLF_RENDER_GLES,          /**< OpenGL ES hardware rendering */
	WLF_RENDER_VULKAN,        /**< Vulkan hardware rendering */
};

/**
 * @brief Color structure with RGBA components
 */
struct wlf_color {
	float r, g, b, a;  /**< Red, Green, Blue, Alpha components (0.0-1.0) */
};

/**
 * @brief Rectangle drawing parameters
 */
struct wlf_draw_rect {
	struct wlf_vector2 position;      /**< Rectangle position */
	struct wlf_vector2 size;          /**< Rectangle size */
	struct wlf_color fill_color;      /**< Fill color */
	struct wlf_color border_color;    /**< Border color */
	float border_width;               /**< Border width */
	float corner_radius[4];           /**< Corner radii [TL, TR, BR, BL] */
};

/**
 * @brief Texture drawing parameters
 */
struct wlf_draw_texture {
	struct wlf_texture *texture;      /**< Texture to draw */
	struct wlf_vector2 position;      /**< Draw position */
	struct wlf_vector2 size;          /**< Draw size */
	struct wlf_rect src_rect;         /**< Source rectangle in texture */
	struct wlf_color tint_color;      /**< Tint color overlay */
	bool flip_horizontal;             /**< Flip horizontally */
	bool flip_vertical;               /**< Flip vertically */
};

/**
 * @brief Renderer implementation interface
 */
struct wlf_renderer_impl {
	enum wlf_render_type type;        /**< Renderer type */

	void (*destroy)(struct wlf_render *render);

	// Frame management
	void (*begin_frame)(struct wlf_render *render);
	void (*end_frame)(struct wlf_render *render);
	void (*clear)(struct wlf_render *render, struct wlf_color color);

	// State management
	void (*set_transform)(struct wlf_render *render, const struct wlf_matrix4x4 *transform);
	void (*push_transform)(struct wlf_render *render, const struct wlf_matrix4x4 *transform);
	void (*pop_transform)(struct wlf_render *render);
	void (*set_clip_rect)(struct wlf_render *render, const struct wlf_rect *clip);
	void (*push_clip_rect)(struct wlf_render *render, const struct wlf_rect *clip);
	void (*pop_clip_rect)(struct wlf_render *render);
	void (*set_alpha)(struct wlf_render *render, float alpha);

	// Drawing operations
	void (*draw_rectangle)(struct wlf_render *render, const struct wlf_draw_rect *rect);
	void (*draw_texture)(struct wlf_render *render, const struct wlf_draw_texture *tex);

	// Texture management
	struct wlf_texture *(*texture_from_buffer)(struct wlf_render *render,
		struct wlf_buffer *buffer);
};

/**
 * @brief Main renderer structure
 */
struct wlf_render {
	const struct wlf_renderer_impl *impl;
	void *backend_data;  /**< Backend-specific data */
};

/**
 * @brief Create a renderer with automatic backend detection
 * @param backend Pointer to the backend
 * @return Pointer to the created renderer, or NULL on failure
 */
struct wlf_render *wlf_renderer_autocreate(struct wlf_backend *backend);

/**
 * @brief Create a renderer with specific backend type
 * @param backend Pointer to the backend
 * @param type Renderer type to create
 * @return Pointer to the created renderer, or NULL on failure
 */
struct wlf_render *wlf_renderer_create(struct wlf_backend *backend, enum wlf_render_type type);

/**
 * @brief Destroy a renderer
 * @param render Pointer to the renderer to destroy
 */
void wlf_renderer_destroy(struct wlf_render *render);

/**
 * @brief Begin a new frame
 * @param render Pointer to the renderer
 */
void wlf_renderer_begin_frame(struct wlf_render *render);

/**
 * @brief End the current frame
 * @param render Pointer to the renderer
 */
void wlf_renderer_end_frame(struct wlf_render *render);

/**
 * @brief Clear the render target with a color
 * @param render Pointer to the renderer
 * @param color Clear color
 */
void wlf_renderer_clear(struct wlf_render *render, struct wlf_color color);

/**
 * @brief Set the current transformation matrix
 * @param render Pointer to the renderer
 * @param transform Pointer to the transformation matrix
 */
void wlf_renderer_set_transform(struct wlf_render *render, const struct wlf_matrix4x4 *transform);

/**
 * @brief Push a transformation matrix onto the stack
 * @param render Pointer to the renderer
 * @param transform Pointer to the transformation matrix
 */
void wlf_renderer_push_transform(struct wlf_render *render, const struct wlf_matrix4x4 *transform);

/**
 * @brief Pop the top transformation matrix from the stack
 * @param render Pointer to the renderer
 */
void wlf_renderer_pop_transform(struct wlf_render *render);

/**
 * @brief Set the clipping rectangle
 * @param render Pointer to the renderer
 * @param clip Pointer to the clipping rectangle
 */
void wlf_renderer_set_clip_rect(struct wlf_render *render, const struct wlf_rect *clip);

/**
 * @brief Push a clipping rectangle onto the stack
 * @param render Pointer to the renderer
 * @param clip Pointer to the clipping rectangle
 */
void wlf_renderer_push_clip_rect(struct wlf_render *render, const struct wlf_rect *clip);

/**
 * @brief Pop the top clipping rectangle from the stack
 * @param render Pointer to the renderer
 */
void wlf_renderer_pop_clip_rect(struct wlf_render *render);

/**
 * @brief Set the global alpha value
 * @param render Pointer to the renderer
 * @param alpha Alpha value (0.0-1.0)
 */
void wlf_renderer_set_alpha(struct wlf_render *render, float alpha);

/**
 * @brief Draw a rectangle
 * @param render Pointer to the renderer
 * @param rect Pointer to the rectangle drawing parameters
 */
void wlf_renderer_draw_rectangle(struct wlf_render *render, const struct wlf_draw_rect *rect);

/**
 * @brief Draw a texture
 * @param render Pointer to the renderer
 * @param tex Pointer to the texture drawing parameters
 */
void wlf_renderer_draw_texture(struct wlf_render *render, const struct wlf_draw_texture *tex);

/**
 * @brief Create a texture from a buffer
 * @param render Pointer to the renderer
 * @param buffer Pointer to the buffer
 * @return Pointer to the created texture, or NULL on failure
 */
struct wlf_texture *wlf_renderer_texture_from_buffer(struct wlf_render *render,
	struct wlf_buffer *buffer);

#endif // RENDER_WLF_RENDER_H
