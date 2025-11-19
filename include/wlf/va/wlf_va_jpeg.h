/**
 * @file        wlf_va_jpeg.h
 * @brief       VA-API JPEG encoding and decoding with DMA-BUF support.
 * @details     This file provides functions for JPEG encoding/decoding using
 *              VA-API hardware acceleration, with support for DMA-BUF import
 *              and export. This enables zero-copy workflows between JPEG images
 *              and other hardware-accelerated components.
 *
 *              Features:
 *              - JPEG decoding from file/memory to DMA-BUF
 *              - JPEG encoding from DMA-BUF to file/memory
 *              - Zero-copy integration with VA-API video pipelines
 *              - Hardware-accelerated JPEG processing
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-31, initial version\n
 */

#ifndef VA_WLF_VA_JPEG_H
#define VA_WLF_VA_JPEG_H

#include "wlf/va/wlf_va_display.h"
#include "wlf/dmabuf/wlf_dmabuf.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <va/va.h>

/**
 * @brief JPEG decoder context for VA-API.
 */
struct wlf_va_jpeg_decoder {
	struct wlf_va_display *va_display;  /**< VA display wrapper */
	VADisplay display;                   /**< VA display handle */
	VAConfigID config_id;                /**< VA configuration ID */
	VAContextID context_id;              /**< VA context ID */
	VASurfaceID surface_id;              /**< VA surface for decoded image */

	/* Image properties */
	uint32_t width;                      /**< Image width */
	uint32_t height;                     /**< Image height */
	uint32_t fourcc;                     /**< FourCC format */

	bool initialized;                    /**< Initialization status */
};

/**
 * @brief JPEG encoder context for VA-API.
 */
struct wlf_va_jpeg_encoder {
	struct wlf_va_display *va_display;  /**< VA display wrapper */
	VADisplay display;                   /**< VA display handle */
	VAConfigID config_id;                /**< VA configuration ID */
	VAContextID context_id;              /**< VA context ID */

	/* Encoding parameters */
	uint32_t quality;                    /**< JPEG quality (1-100) */
	uint32_t width;                      /**< Image width */
	uint32_t height;                     /**< Image height */
	uint32_t fourcc;                     /**< FourCC format */

	bool initialized;                    /**< Initialization status */
};

/**
 * @brief Create a VA-API JPEG decoder.
 *
 * @param va_display VA display wrapper (can be NULL to auto-create).
 * @return Pointer to the decoder context, or NULL on failure.
 */
struct wlf_va_jpeg_decoder *wlf_va_jpeg_decoder_create(
	struct wlf_va_display *va_display);

/**
 * @brief Destroy a VA-API JPEG decoder.
 *
 * @param decoder Pointer to the decoder context.
 */
void wlf_va_jpeg_decoder_destroy(struct wlf_va_jpeg_decoder *decoder);

/**
 * @brief Decode a JPEG file to DMA-BUF.
 *
 * Decodes a JPEG image file using VA-API hardware acceleration and exports
 * the result as DMA-BUF file descriptors for zero-copy sharing.
 *
 * @param decoder Pointer to the decoder context.
 * @param filename Path to the JPEG file.
 * @param attribs Output DMA-BUF attributes (caller should call wlf_dmabuf_attributes_finish).
 * @return true on success, false on failure.
 */
bool wlf_va_jpeg_decode_file_to_dmabuf(
	struct wlf_va_jpeg_decoder *decoder,
	const char *filename,
	struct wlf_dmabuf_attributes *attribs);

/**
 * @brief Decode JPEG data from memory to DMA-BUF.
 *
 * Decodes JPEG image data from memory using VA-API hardware acceleration
 * and exports the result as DMA-BUF file descriptors.
 *
 * @param decoder Pointer to the decoder context.
 * @param data Pointer to JPEG data in memory.
 * @param size Size of JPEG data in bytes.
 * @param attribs Output DMA-BUF attributes (caller should call wlf_dmabuf_attributes_finish).
 * @return true on success, false on failure.
 */
bool wlf_va_jpeg_decode_data_to_dmabuf(
	struct wlf_va_jpeg_decoder *decoder,
	const uint8_t *data,
	size_t size,
	struct wlf_dmabuf_attributes *attribs);

/**
 * @brief Get the decoded surface ID.
 *
 * Returns the VA surface ID of the last decoded JPEG image. This can be
 * used for further VA-API operations without going through DMA-BUF.
 *
 * @param decoder Pointer to the decoder context.
 * @return VA surface ID, or VA_INVALID_SURFACE if no image decoded.
 */
VASurfaceID wlf_va_jpeg_decoder_get_surface(struct wlf_va_jpeg_decoder *decoder);

/**
 * @brief Create a VA-API JPEG encoder.
 *
 * @param va_display VA display wrapper (can be NULL to auto-create).
 * @param quality JPEG quality (1-100, default 85).
 * @return Pointer to the encoder context, or NULL on failure.
 */
struct wlf_va_jpeg_encoder *wlf_va_jpeg_encoder_create(
	struct wlf_va_display *va_display,
	uint32_t quality);

/**
 * @brief Destroy a VA-API JPEG encoder.
 *
 * @param encoder Pointer to the encoder context.
 */
void wlf_va_jpeg_encoder_destroy(struct wlf_va_jpeg_encoder *encoder);

/**
 * @brief Encode DMA-BUF to JPEG file.
 *
 * Encodes image data from DMA-BUF to a JPEG file using VA-API hardware
 * acceleration. Supports zero-copy import of DMA-BUF from other hardware
 * components.
 *
 * @param encoder Pointer to the encoder context.
 * @param attribs DMA-BUF attributes of the source image.
 * @param filename Output JPEG filename.
 * @return true on success, false on failure.
 */
bool wlf_va_jpeg_encode_dmabuf_to_file(
	struct wlf_va_jpeg_encoder *encoder,
	const struct wlf_dmabuf_attributes *attribs,
	const char *filename);

/**
 * @brief Encode DMA-BUF to JPEG data in memory.
 *
 * Encodes image data from DMA-BUF to JPEG format in memory using VA-API
 * hardware acceleration.
 *
 * @param encoder Pointer to the encoder context.
 * @param attribs DMA-BUF attributes of the source image.
 * @param data Output pointer to JPEG data (caller must free).
 * @param size Output size of JPEG data in bytes.
 * @return true on success, false on failure.
 */
bool wlf_va_jpeg_encode_dmabuf_to_data(
	struct wlf_va_jpeg_encoder *encoder,
	const struct wlf_dmabuf_attributes *attribs,
	uint8_t **data,
	size_t *size);

/**
 * @brief Set JPEG encoding quality.
 *
 * @param encoder Pointer to the encoder context.
 * @param quality JPEG quality (1-100).
 * @return true on success, false on failure.
 */
bool wlf_va_jpeg_encoder_set_quality(
	struct wlf_va_jpeg_encoder *encoder,
	uint32_t quality);

#endif /* VA_WLF_VA_JPEG_H */
