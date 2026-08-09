/**
 * @file        wlf_zwp_linux_dmabuf_v1.h
 * @brief       Wayland zwp_linux_dmabuf_v1 protocol wrapper for wlframe.
 * @details     Implements the stable linux-dmabuf-v1 protocol (version 5),
 *              which allows clients to import DMA-BUF (dma_buf fd) based
 *              buffers into the Wayland compositor.
 *
 *              The protocol has three objects:
 *
 *              1. zwp_linux_dmabuf_v1 – global manager.  Use
 *                 wlf_zwp_linux_dmabuf_v1_create_params() to start building
 *                 a buffer.  Use wlf_zwp_linux_dmabuf_v1_get_default_feedback()
 *                 / get_surface_feedback() (v4+) to receive per-device format
 *                 and modifier feedback.
 *
 *              2. zwp_linux_buffer_params_v1 – accumulated per-plane metadata
 *                 (fd, offset, stride, modifier).  Call
 *                 wlf_zwp_linux_buffer_params_v1_add() for each plane, then
 *                 wlf_zwp_linux_buffer_params_v1_create() to request an async
 *                 wl_buffer, or wlf_zwp_linux_buffer_params_v1_create_immed()
 *                 for a synchronous import (v2+).  The created/failed signals
 *                 report the result.
 *
 *              3. zwp_linux_dmabuf_feedback_v1 – format/modifier and
 *                 target-device information, delivered as a sequence of events.
 *                 The done signal fires when one complete parameter set has
 *                 been delivered.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_ZWP_LINUX_DMABUF_V1_H
#define WAYLAND_WLF_ZWP_LINUX_DMABUF_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stddef.h>

struct wl_buffer;
struct wl_registry;
struct wl_surface;
struct zwp_linux_dmabuf_v1;
struct zwp_linux_buffer_params_v1;
struct zwp_linux_dmabuf_feedback_v1;

/**
 * @brief One entry in the DMA-BUF format-modifier table.
 *
 * The table is mmap'd from the format_table event fd.  Layout is as defined
 * by the protocol: 4-byte format, 4-byte padding, 8-byte modifier.
 */
struct wlf_dmabuf_format_entry {
	uint32_t format;   /**< DRM format fourcc */
	uint32_t padding;  /**< Reserved; must be ignored */
	uint64_t modifier; /**< DRM format modifier */
};

/**
 * @brief Bitmask flags for a preference tranche
 *        (mirrors zwp_linux_dmabuf_feedback_v1.tranche_flags).
 */
enum wlf_dmabuf_tranche_flags {
	WLF_DMABUF_TRANCHE_SCANOUT = 1, /**< Tranche is suitable for direct scanout */
};

/**
 * @brief One preference tranche within a feedback object.
 *
 * Each tranche lists the target device and a set of format+modifier pairs
 * (referenced by index into the format table) that are preferred for that
 * device.
 */
struct wlf_zwp_linux_dmabuf_tranche {
	uint64_t target_device; /**< dev_t of the preferred scan-out/render device */
	uint32_t flags;         /**< wlf_dmabuf_tranche_flags bitfield */
	uint16_t *indices;      /**< Indices into the format table */
	size_t    n_indices;    /**< Number of valid entries in indices */
};

/**
 * @brief Wrapper around a zwp_linux_dmabuf_feedback_v1 object.
 *
 * Created by wlf_zwp_linux_dmabuf_v1_get_default_feedback() or
 * wlf_zwp_linux_dmabuf_v1_get_surface_feedback().  The caller owns and must
 * destroy this object.
 *
 * All fields are updated atomically before the done signal fires.  Callers
 * should read format_table / main_device / tranches only inside a done
 * listener.
 */
struct wlf_zwp_linux_dmabuf_feedback_v1 {
	struct zwp_linux_dmabuf_feedback_v1 *base;

	/* Format table (mmap'd; valid between done signals) */
	struct wlf_dmabuf_format_entry *format_table; /**< Mapped pointer        */
	size_t format_table_len;                       /**< Number of entries     */

	/** dev_t of the compositor's main (preferred) device. */
	uint64_t main_device;

	/** Committed preference tranches (valid inside done handler). */
	struct wlf_zwp_linux_dmabuf_tranche *tranches;
	size_t n_tranches;

	struct {
		/**
		 * Emitted when the compositor finishes sending a complete parameter
		 * set.  The listener receives a pointer to this struct; read the
		 * format_table, main_device, and tranches fields.
		 */
		struct wlf_signal done;

		struct wlf_signal destroy;
	} events;

	/* Private: pending tranche being accumulated */
	struct wlf_zwp_linux_dmabuf_tranche _pending;
	size_t _tranches_cap;
	int    _fmt_table_fd; /* owned fd; -1 when unmapped */
};

/**
 * @brief Buffer params flags (mirrors zwp_linux_buffer_params_v1.flags).
 */
enum wlf_linux_buffer_params_flags {
	WLF_LINUX_BUFFER_PARAMS_Y_INVERT    = 1, /**< Buffer is y-inverted           */
	WLF_LINUX_BUFFER_PARAMS_INTERLACED  = 2, /**< Buffer is interlaced           */
	WLF_LINUX_BUFFER_PARAMS_BOTTOM_FIRST = 4, /**< Bottom field comes first      */
};

/**
 * @brief Wrapper around a zwp_linux_buffer_params_v1 object.
 *
 * Created by wlf_zwp_linux_dmabuf_v1_create_params().  The caller accumulates
 * plane information with wlf_zwp_linux_buffer_params_v1_add(), then requests
 * the wl_buffer via create() or create_immed().  After the created or failed
 * signal fires, this object should be destroyed.
 */
struct wlf_zwp_linux_buffer_params_v1 {
	struct zwp_linux_buffer_params_v1 *base;

	struct {
		/**
		 * Emitted when the compositor successfully creates the wl_buffer.
		 * The signal data is a pointer to the new wl_buffer.  The caller
		 * is responsible for destroying the wl_buffer when done.
		 */
		struct wlf_signal created;

		/**
		 * Emitted when the compositor rejects the buffer parameters.
		 * Signal data is NULL.
		 */
		struct wlf_signal failed;

		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Wrapper around a bound zwp_linux_dmabuf_v1 global.
 */
struct wlf_zwp_linux_dmabuf_v1 {
	struct zwp_linux_dmabuf_v1 *base;
	uint32_t version; /**< Negotiated bind version */

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the zwp_linux_dmabuf_v1 global from the registry.
 */
struct wlf_zwp_linux_dmabuf_v1 *wlf_zwp_linux_dmabuf_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the global manager.
 */
void wlf_zwp_linux_dmabuf_v1_destroy(struct wlf_zwp_linux_dmabuf_v1 *dmabuf);

/**
 * @brief Create a buffer params builder object.
 *
 * @return Newly allocated params object, or NULL on failure.
 */
struct wlf_zwp_linux_buffer_params_v1 *
wlf_zwp_linux_dmabuf_v1_create_params(struct wlf_zwp_linux_dmabuf_v1 *dmabuf);

/**
 * @brief Get the default (compositor-wide) dmabuf feedback.
 *
 * Requires bind version >= 4.
 *
 * @return Newly allocated feedback object, or NULL on failure.
 */
struct wlf_zwp_linux_dmabuf_feedback_v1 *
wlf_zwp_linux_dmabuf_v1_get_default_feedback(
	struct wlf_zwp_linux_dmabuf_v1 *dmabuf);

/**
 * @brief Get per-surface dmabuf feedback.
 *
 * Requires bind version >= 4.
 *
 * @param surface  wl_surface to query feedback for.
 * @return Newly allocated feedback object, or NULL on failure.
 */
struct wlf_zwp_linux_dmabuf_feedback_v1 *
wlf_zwp_linux_dmabuf_v1_get_surface_feedback(
	struct wlf_zwp_linux_dmabuf_v1 *dmabuf, struct wl_surface *surface);

/**
 * @brief Add a DMA-BUF plane to the params object.
 *
 * @param params    Buffer params object.
 * @param fd        File descriptor of the DMA-BUF plane.
 * @param plane_idx Plane index (0..3).
 * @param offset    Byte offset of the plane within the buffer.
 * @param stride    Bytes per row for the plane.
 * @param modifier_hi High 32 bits of the DRM format modifier.
 * @param modifier_lo Low 32 bits of the DRM format modifier.
 */
void wlf_zwp_linux_buffer_params_v1_add(
	struct wlf_zwp_linux_buffer_params_v1 *params,
	int fd, uint32_t plane_idx, uint32_t offset, uint32_t stride,
	uint32_t modifier_hi, uint32_t modifier_lo);

/**
 * @brief Request asynchronous wl_buffer creation.
 *
 * The created or failed signal will be emitted on the next dispatch.
 *
 * @param params   Buffer params object.
 * @param width    Buffer width in pixels.
 * @param height   Buffer height in pixels.
 * @param format   DRM format fourcc.
 * @param flags    wlf_linux_buffer_params_flags bitfield.
 */
void wlf_zwp_linux_buffer_params_v1_create(
	struct wlf_zwp_linux_buffer_params_v1 *params,
	int32_t width, int32_t height, uint32_t format, uint32_t flags);

/**
 * @brief Import a DMA-BUF and immediately return a wl_buffer (v2+).
 *
 * Unlike create(), this request returns the wl_buffer synchronously.  The
 * server may still send a failed event later, at which point the returned
 * wl_buffer becomes invalid.
 *
 * @return New wl_buffer, or NULL on protocol error.
 */
struct wl_buffer *wlf_zwp_linux_buffer_params_v1_create_immed(
	struct wlf_zwp_linux_buffer_params_v1 *params,
	int32_t width, int32_t height, uint32_t format, uint32_t flags);

/**
 * @brief Destroy a buffer params object.
 */
void wlf_zwp_linux_buffer_params_v1_destroy(
	struct wlf_zwp_linux_buffer_params_v1 *params);

/**
 * @brief Destroy a dmabuf feedback object.
 */
void wlf_zwp_linux_dmabuf_feedback_v1_destroy(
	struct wlf_zwp_linux_dmabuf_feedback_v1 *feedback);

#endif /* WAYLAND_WLF_ZWP_LINUX_DMABUF_V1_H */
