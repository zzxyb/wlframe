/**
 * @file        wlf_va_video_decoder.h
 * @brief       VA-API video decoder implementation for wlframe.
 * @details     This file defines the VA-API-based video decoder that uses
 *              VA-API for hardware-accelerated video decoding on Linux.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-30
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-30, initial version\n
 */

#ifndef VA_WLF_VA_VIDEO_DECODER_H
#define VA_WLF_VA_VIDEO_DECODER_H

#include "wlf/va/wlf_video_decoder.h"
#include <va/va.h>

struct wlf_va_display;

/**
 * @struct wlf_va_video_decoder_config
 * @brief Configuration for VA-API video decoder creation.
 */
struct wlf_va_video_decoder_config {
	struct wlf_video_decoder_config base; /**< Base decoder configuration */

	struct wlf_va_display *va_display;    /**< VA display (optional, will create if NULL) */
	VADisplay display;                    /**< Custom VA display handle (optional) */
};

/**
 * @struct wlf_va_video_decoder
 * @brief VA-API video decoder instance.
 */
struct wlf_va_video_decoder {
	struct wlf_video_decoder base;        /**< Base decoder (must be first) */

	/* VA-API resources */
	struct wlf_va_display *va_display;    /**< VA display wrapper */
	bool owns_va_display;                 /**< Whether we own the VA display */

	VADisplay display;                    /**< VA display handle */
	VAConfigID config_id;                 /**< VA configuration ID */
	VAContextID context_id;               /**< VA context ID */
	VAProfile profile;                    /**< VA profile */
	VAEntrypoint entrypoint;              /**< VA entrypoint */

	/* Decode surfaces */
	VASurfaceID *surfaces;                /**< VA surfaces for decoding */
	uint32_t surface_count;               /**< Number of surfaces */

	/* Codec-specific data */
	void *codec_data;                     /**< Codec-specific data */
};

/**
 * @brief Create a VA-API video decoder.
 *
 * @param config VA-API decoder configuration
 * @return Pointer to created decoder, or NULL on failure
 */
struct wlf_video_decoder *wlf_va_video_decoder_create(
	const struct wlf_va_video_decoder_config *config);

/**
 * @brief Create a VA-API video decoder from an existing VA display.
 *
 * @param va_display Pointer to VA display wrapper
 * @param config Base decoder configuration
 * @return Pointer to created decoder, or NULL on failure
 */
struct wlf_video_decoder *wlf_va_video_decoder_create_from_display(
	struct wlf_va_display *va_display,
	const struct wlf_video_decoder_config *config);

/**
 * @brief Check if a decoder is a VA-API decoder.
 *
 * @param decoder Pointer to video decoder
 * @return true if decoder is VA-API-based, false otherwise
 */
bool wlf_video_decoder_is_va(struct wlf_video_decoder *decoder);

/**
 * @brief Convert a base decoder to VA-API decoder.
 *
 * @param decoder Pointer to base decoder
 * @return Pointer to VA-API decoder, or NULL if not a VA-API decoder
 */
struct wlf_va_video_decoder *wlf_va_video_decoder_from_decoder(
	struct wlf_video_decoder *decoder);

/**
 * @brief Query video decode capabilities for VA-API.
 *
 * @param display VA display handle
 * @param codec Video codec to query
 * @return true if codec is supported, false otherwise
 */
bool wlf_va_video_decoder_query_capabilities(VADisplay display,
	enum wlf_video_codec codec);

#endif /* VA_WLF_VA_VIDEO_DECODER_H */
