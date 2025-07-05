/**
 * @file        wlf_blit.h
 * @brief       Blit operations for efficient memory-to-memory data transfer in wlframe.
 * @details     This file provides functionality for copying data between framebuffers, textures,
 *              and other graphics resources. It supports various filtering modes and can handle
 *              scaling operations during the transfer. The blit operations are designed to be
 *              hardware-accelerated when possible.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef BLIT_WLF_BLIT_H
#define BLIT_WLF_BLIT_H

#include "wlf/math/wlf_rect.h"

struct wlf_framebuffer;
struct wlf_texture;
struct wlf_render_context;

/**
 * @brief Filtering modes for blit operations.
 *
 * Defines how pixel data should be interpolated when scaling occurs during blit operations.
 */
enum wlf_blit_filter {
	WLF_BLIT_FILTER_NEAREST = 0,    /**< Nearest neighbor filtering (pixelated scaling) */
	WLF_BLIT_FILTER_LINEAR,         /**< Linear filtering (smooth scaling) */
};

/**
 * @brief Virtual function table for blit operations.
 *
 * Contains function pointers for different types of blit operations.
 * Different backends (OpenGL, Vulkan, etc.) will provide their own implementations.
 */
struct wlf_blit_impl {
	/**
	 * @brief Copy data from one framebuffer to another.
	 * @param context Render context for the operation.
	 * @param src Source framebuffer.
	 * @param dst Destination framebuffer.
	 * @param src_rect Source rectangle.
	 * @param dst_rect Destination rectangle.
	 * @param filter Filtering mode to use.
	 * @return true on success, false on failure.
	 */
	bool (*framebuffer_to_framebuffer)(struct wlf_render_context* context,
										struct wlf_framebuffer* src,
										struct wlf_framebuffer* dst,
										struct wlf_rect src_rect,
										struct wlf_rect dst_rect,
										enum wlf_blit_filter filter);

	/**
	 * @brief Copy texture data to a framebuffer.
	 * @param context Render context for the operation.
	 * @param src Source texture.
	 * @param dst Destination framebuffer.
	 * @param src_rect Source rectangle.
	 * @param dst_rect Destination rectangle.
	 * @param filter Filtering mode to use.
	 * @return true on success, false on failure.
	 */
	bool (*texture_to_framebuffer)(struct wlf_render_context* context,
									struct wlf_texture* src,
									struct wlf_framebuffer* dst,
									struct wlf_rect src_rect,
									struct wlf_rect dst_rect,
									enum wlf_blit_filter filter);

	/**
	 * @brief Copy framebuffer data to a texture.
	 * @param context Render context for the operation.
	 * @param src Source framebuffer.
	 * @param dst Destination texture.
	 * @param src_rect Source rectangle.
	 * @param dst_rect Destination rectangle.
	 * @param filter Filtering mode to use.
	 * @return true on success, false on failure.
	 */
	bool (*framebuffer_to_texture)(struct wlf_render_context* context,
									struct wlf_framebuffer* src,
									struct wlf_texture* dst,
									struct wlf_rect src_rect,
									struct wlf_rect dst_rect,
									enum wlf_blit_filter filter);

	/**
	 * @brief Copy data from one texture to another.
	 * @param context Render context for the operation.
	 * @param src Source texture.
	 * @param dst Destination texture.
	 * @param src_rect Source rectangle.
	 * @param dst_rect Destination rectangle.
	 * @param filter Filtering mode to use.
	 * @return true on success, false on failure.
	 */
	bool (*texture_to_texture)(struct wlf_render_context* context,
								struct wlf_texture* src,
								struct wlf_texture* dst,
								struct wlf_rect src_rect,
								struct wlf_rect dst_rect,
								enum wlf_blit_filter filter);

	/**
	 * @brief Synchronize and wait for all blit operations to complete.
	 * @param context Render context for the operation.
	 */
	void (*sync)(struct wlf_render_context* context);
};

/**
 * @brief Blit operation context.
 *
 * Contains the function table and render context needed for blit operations.
 * This structure provides a unified interface for different blit implementations.
 *
 * @code
 * struct wlf_blit *blit = wlf_blit_create(context);
 * wlf_blit_texture_to_framebuffer(blit, texture, framebuffer, src_rect, dst_rect, filter);
 * wlf_blit_destroy(blit);
 * @endcode
 */
struct wlf_blit {
	const struct wlf_blit_impl* impl; /**< Virtual function table */
	struct wlf_render_context* context;   /**< Associated render context */
};

/**
 * @brief Creates a new blit operation context.
 *
 * @param context Render context to associate with the blit operations.
 * @return New blit context on success, NULL on failure.
 */
struct wlf_blit* wlf_blit_create(struct wlf_render_context* context);

/**
 * @brief Destroys a blit operation context.
 *
 * @param blit Blit context to destroy.
 */
void wlf_blit_destroy(struct wlf_blit* blit);

/**
 * @brief Copies data from one framebuffer to another.
 *
 * @param blit Blit context.
 * @param src Source framebuffer.
 * @param dst Destination framebuffer.
 * @param src_rect Source region to copy from.
 * @param dst_rect Destination region to copy to.
 * @param filter Filtering mode for scaling.
 * @return true on success, false on failure.
 */
bool wlf_blit_framebuffer_to_framebuffer(struct wlf_blit* blit,
										struct wlf_framebuffer* src,
										struct wlf_framebuffer* dst,
										struct wlf_rect src_rect,
										struct wlf_rect dst_rect,
										enum wlf_blit_filter filter);

/**
 * @brief Copies texture data to a framebuffer.
 *
 * @param blit Blit context.
 * @param src Source texture.
 * @param dst Destination framebuffer.
 * @param src_rect Source region to copy from.
 * @param dst_rect Destination region to copy to.
 * @param filter Filtering mode for scaling.
 * @return true on success, false on failure.
 */
bool wlf_blit_texture_to_framebuffer(struct wlf_blit* blit,
									struct wlf_texture* src,
									struct wlf_framebuffer* dst,
									struct wlf_rect src_rect,
									struct wlf_rect dst_rect,
									enum wlf_blit_filter filter);

/**
 * @brief Copies framebuffer data to a texture.
 *
 * @param blit Blit context.
 * @param src Source framebuffer.
 * @param dst Destination texture.
 * @param src_rect Source region to copy from.
 * @param dst_rect Destination region to copy to.
 * @param filter Filtering mode for scaling.
 * @return true on success, false on failure.
 */
bool wlf_blit_framebuffer_to_texture(struct wlf_blit* blit,
									struct wlf_framebuffer* src,
									struct wlf_texture* dst,
									struct wlf_rect src_rect,
									struct wlf_rect dst_rect,
									enum wlf_blit_filter filter);

/**
 * @brief Copies data from one texture to another.
 *
 * @param blit Blit context.
 * @param src Source texture.
 * @param dst Destination texture.
 * @param src_rect Source region to copy from.
 * @param dst_rect Destination region to copy to.
 * @param filter Filtering mode for scaling.
 * @return true on success, false on failure.
 */
bool wlf_blit_texture_to_texture(struct wlf_blit* blit,
								struct wlf_texture* src,
								struct wlf_texture* dst,
								struct wlf_rect src_rect,
								struct wlf_rect dst_rect,
								enum wlf_blit_filter filter);

/**
 * @brief Copies entire framebuffer using full dimensions.
 *
 * Convenience function that automatically determines source and destination rectangles
 * based on the framebuffer dimensions.
 *
 * @param blit Blit context.
 * @param src Source framebuffer.
 * @param dst Destination framebuffer.
 * @param filter Filtering mode for scaling.
 * @return true on success, false on failure.
 */
bool wlf_blit_framebuffer_full(struct wlf_blit* blit,
								struct wlf_framebuffer* src,
								struct wlf_framebuffer* dst,
								enum wlf_blit_filter filter);

/**
 * @brief Copies entire texture using full dimensions.
 *
 * Convenience function that automatically determines source and destination rectangles
 * based on the texture dimensions.
 *
 * @param blit Blit context.
 * @param src Source texture.
 * @param dst Destination texture.
 * @param filter Filtering mode for scaling.
 * @return true on success, false on failure.
 */
bool wlf_blit_texture_full(struct wlf_blit* blit,
							struct wlf_texture* src,
							struct wlf_texture* dst,
							enum wlf_blit_filter filter);

/**
 * @brief Synchronizes and waits for all blit operations to complete.
 *
 * @param blit Blit context.
 */
void wlf_blit_sync(struct wlf_blit* blit);

#endif // BLIT_WLF_BLIT_H
