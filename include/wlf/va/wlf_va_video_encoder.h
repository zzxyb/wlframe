/**
 * @file        wlf_va_video_encoder.h
 * @brief       VA-API video encoder implementation for wlframe.
 * @details     This file defines the VA-API-based video encoder that uses
 *              VA-API for hardware-accelerated video encoding on Linux.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-30
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-30, initial version\n
 */

#ifndef VA_WLF_VA_VIDEO_ENCODER_H
#define VA_WLF_VA_VIDEO_ENCODER_H

#include "wlf/va/wlf_video_encoder.h"
#include <va/va.h>

struct wlf_va_display;

/**
 * @struct wlf_va_video_encoder_config
 * @brief Configuration for VA-API video encoder creation.
 */
struct wlf_va_video_encoder_config {
	struct wlf_video_encoder_config base; /**< Base encoder configuration */

	struct wlf_va_display *va_display;    /**< VA display (optional, will create if NULL) */
	VADisplay display;                    /**< Custom VA display handle (optional) */
};

/**
 * @struct wlf_va_video_encoder
 * @brief VA-API video encoder instance.
 */
struct wlf_va_video_encoder {
	struct wlf_video_encoder base;        /**< Base encoder (must be first) */

	/* VA-API resources */
	struct wlf_va_display *va_display;    /**< VA display wrapper */
	bool owns_va_display;                 /**< Whether we own the VA display */

	VADisplay display;                    /**< VA display handle */
	VAConfigID config_id;                 /**< VA configuration ID */
	VAContextID context_id;               /**< VA context ID */
	VAProfile profile;                    /**< VA profile */
	VAEntrypoint entrypoint;              /**< VA entrypoint */

	/* Encode surfaces */
	VASurfaceID *surfaces;                /**< VA surfaces for encoding */
	uint32_t surface_count;               /**< Number of surfaces */

	/* Output buffer */
	VABufferID coded_buf_id;              /**< Coded buffer for output */

	/* Codec-specific data */
	void *codec_data;                     /**< Codec-specific data */
};

/**
 * @brief Create a VA-API video encoder.
 *
 * @param config VA-API encoder configuration
 * @return Pointer to created encoder, or NULL on failure
 */
struct wlf_video_encoder *wlf_va_video_encoder_create(
	const struct wlf_va_video_encoder_config *config);

/**
 * @brief Create a VA-API video encoder from an existing VA display.
 *
 * @param va_display Pointer to VA display wrapper
 * @param config Base encoder configuration
 * @return Pointer to created encoder, or NULL on failure
 */
struct wlf_video_encoder *wlf_va_video_encoder_create_from_display(
	struct wlf_va_display *va_display,
	const struct wlf_video_encoder_config *config);

/**
 * @brief Check if an encoder is a VA-API encoder.
 *
 * @param encoder Pointer to video encoder
 * @return true if encoder is VA-API-based, false otherwise
 */
bool wlf_video_encoder_is_va(struct wlf_video_encoder *encoder);

/**
 * @brief Convert a base encoder to VA-API encoder.
 *
 * @param encoder Pointer to base encoder
 * @return Pointer to VA-API encoder, or NULL if not a VA-API encoder
 */
struct wlf_va_video_encoder *wlf_va_video_encoder_from_encoder(
	struct wlf_video_encoder *encoder);

/**
 * @brief Query video encode capabilities for VA-API.
 *
 * @param display VA display handle
 * @param codec Video codec to query
 * @return true if codec is supported, false otherwise
 */
bool wlf_va_video_encoder_query_capabilities(VADisplay display,
	enum wlf_video_codec codec);

#endif /* VA_WLF_VA_VIDEO_ENCODER_H */
