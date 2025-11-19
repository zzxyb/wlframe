/**
 * @file        wlf_va_jpeg.c
 * @brief       VA-API JPEG encoding and decoding implementation.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#include "wlf/va/wlf_va_jpeg.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <va/va_drmcommon.h>

/* Helper function to read file into memory */
static uint8_t *read_file_to_memory(const char *filename, size_t *size) {
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		wlf_log_errno(WLF_ERROR, "Failed to open file: %s", filename);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	uint8_t *data = malloc(*size);
	if (!data) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for file data");
		fclose(fp);
		return NULL;
	}

	if (fread(data, 1, *size, fp) != *size) {
		wlf_log(WLF_ERROR, "Failed to read file data");
		free(data);
		fclose(fp);
		return NULL;
	}

	fclose(fp);
	return data;
}

/* Helper function to write memory to file */
static bool write_memory_to_file(const char *filename, const uint8_t *data, size_t size) {
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		wlf_log_errno(WLF_ERROR, "Failed to open file for writing: %s", filename);
		return false;
	}

	if (fwrite(data, 1, size, fp) != size) {
		wlf_log(WLF_ERROR, "Failed to write file data");
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

/* JPEG Decoder Implementation */

struct wlf_va_jpeg_decoder *wlf_va_jpeg_decoder_create(
	struct wlf_va_display *va_display) {

	struct wlf_va_jpeg_decoder *decoder = calloc(1, sizeof(*decoder));
	if (!decoder) {
		wlf_log(WLF_ERROR, "Failed to allocate JPEG decoder");
		return NULL;
	}

	decoder->va_display = va_display;
	if (va_display) {
		decoder->display = va_display->display;
	}
	decoder->config_id = VA_INVALID_ID;
	decoder->context_id = VA_INVALID_ID;
	decoder->surface_id = VA_INVALID_SURFACE;
	decoder->initialized = false;

	return decoder;
}

void wlf_va_jpeg_decoder_destroy(struct wlf_va_jpeg_decoder *decoder) {
	if (!decoder) {
		return;
	}

	if (decoder->surface_id != VA_INVALID_SURFACE) {
		vaDestroySurfaces(decoder->display, &decoder->surface_id, 1);
	}

	if (decoder->context_id != VA_INVALID_ID) {
		vaDestroyContext(decoder->display, decoder->context_id);
	}

	if (decoder->config_id != VA_INVALID_ID) {
		vaDestroyConfig(decoder->display, decoder->config_id);
	}

	free(decoder);
}

static bool wlf_va_jpeg_decoder_init(
	struct wlf_va_jpeg_decoder *decoder,
	uint32_t width,
	uint32_t height) {

	if (decoder->initialized) {
		/* Reinitialize if dimensions changed */
		if (decoder->width == width && decoder->height == height) {
			return true;
		}

		/* Clean up old resources */
		if (decoder->surface_id != VA_INVALID_SURFACE) {
			vaDestroySurfaces(decoder->display, &decoder->surface_id, 1);
			decoder->surface_id = VA_INVALID_SURFACE;
		}
		if (decoder->context_id != VA_INVALID_ID) {
			vaDestroyContext(decoder->display, decoder->context_id);
			decoder->context_id = VA_INVALID_ID;
		}
		if (decoder->config_id != VA_INVALID_ID) {
			vaDestroyConfig(decoder->display, decoder->config_id);
			decoder->config_id = VA_INVALID_ID;
		}
	}

	decoder->width = width;
	decoder->height = height;
	decoder->fourcc = VA_FOURCC_NV12;  /* Default format */

	/* Create config for JPEG decoding */
	VAStatus status = vaCreateConfig(
		decoder->display,
		VAProfileJPEGBaseline,
		VAEntrypointVLD,
		NULL,
		0,
		&decoder->config_id
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create VA config for JPEG decoding: %d", status);
		return false;
	}

	/* Create surface */
	VASurfaceAttrib attribs[1];
	attribs[0].type = VASurfaceAttribPixelFormat;
	attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
	attribs[0].value.type = VAGenericValueTypeInteger;
	attribs[0].value.value.i = VA_FOURCC_NV12;

	status = vaCreateSurfaces(
		decoder->display,
		VA_RT_FORMAT_YUV420,
		width,
		height,
		&decoder->surface_id,
		1,
		attribs,
		1
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create VA surface: %d", status);
		vaDestroyConfig(decoder->display, decoder->config_id);
		decoder->config_id = VA_INVALID_ID;
		return false;
	}

	/* Create context */
	status = vaCreateContext(
		decoder->display,
		decoder->config_id,
		width,
		height,
		VA_PROGRESSIVE,
		&decoder->surface_id,
		1,
		&decoder->context_id
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create VA context: %d", status);
		vaDestroySurfaces(decoder->display, &decoder->surface_id, 1);
		decoder->surface_id = VA_INVALID_SURFACE;
		vaDestroyConfig(decoder->display, decoder->config_id);
		decoder->config_id = VA_INVALID_ID;
		return false;
	}

	decoder->initialized = true;
	wlf_log(WLF_DEBUG, "JPEG decoder initialized: %ux%u", width, height);
	return true;
}

bool wlf_va_jpeg_decode_data_to_dmabuf(
	struct wlf_va_jpeg_decoder *decoder,
	const uint8_t *data,
	size_t size,
	struct wlf_dmabuf_attributes *attribs) {

	if (!decoder || !data || !size || !attribs) {
		wlf_log(WLF_ERROR, "Invalid parameters for JPEG decode");
		return false;
	}

	/* Parse JPEG header to get dimensions */
	/* Simple JPEG header parser - looking for SOF0 marker */
	uint32_t width = 0, height = 0;
	bool found_sof = false;

	for (size_t i = 0; i < size - 9; i++) {
		if (data[i] == 0xFF && data[i + 1] == 0xC0) {  /* SOF0 marker */
			height = (data[i + 5] << 8) | data[i + 6];
			width = (data[i + 7] << 8) | data[i + 8];
			found_sof = true;
			break;
		}
	}

	if (!found_sof) {
		wlf_log(WLF_ERROR, "Failed to parse JPEG dimensions");
		return false;
	}

	wlf_log(WLF_DEBUG, "JPEG dimensions: %ux%u", width, height);

	/* Initialize decoder with dimensions */
	if (!wlf_va_jpeg_decoder_init(decoder, width, height)) {
		return false;
	}

	/* Create buffer for JPEG bitstream */
	VABufferID bitstream_buffer;
	VAStatus status = vaCreateBuffer(
		decoder->display,
		decoder->context_id,
		VASliceDataBufferType,
		size,
		1,
		(void *)data,
		&bitstream_buffer
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create bitstream buffer: %d", status);
		return false;
	}

	/* Create picture parameter buffer */
	VAPictureParameterBufferJPEGBaseline pic_param;
	memset(&pic_param, 0, sizeof(pic_param));
	pic_param.picture_width = width;
	pic_param.picture_height = height;
	pic_param.num_components = 3;

	VABufferID pic_param_buffer;
	status = vaCreateBuffer(
		decoder->display,
		decoder->context_id,
		VAPictureParameterBufferType,
		sizeof(pic_param),
		1,
		&pic_param,
		&pic_param_buffer
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create picture parameter buffer: %d", status);
		vaDestroyBuffer(decoder->display, bitstream_buffer);
		return false;
	}

	/* Begin picture */
	status = vaBeginPicture(decoder->display, decoder->context_id, decoder->surface_id);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaBeginPicture failed: %d", status);
		vaDestroyBuffer(decoder->display, pic_param_buffer);
		vaDestroyBuffer(decoder->display, bitstream_buffer);
		return false;
	}

	/* Render picture with parameters */
	VABufferID buffers[2] = { pic_param_buffer, bitstream_buffer };
	status = vaRenderPicture(decoder->display, decoder->context_id, buffers, 2);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaRenderPicture failed: %d", status);
		vaEndPicture(decoder->display, decoder->context_id);
		vaDestroyBuffer(decoder->display, pic_param_buffer);
		vaDestroyBuffer(decoder->display, bitstream_buffer);
		return false;
	}

	/* End picture */
	status = vaEndPicture(decoder->display, decoder->context_id);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaEndPicture failed: %d", status);
		vaDestroyBuffer(decoder->display, pic_param_buffer);
		vaDestroyBuffer(decoder->display, bitstream_buffer);
		return false;
	}

	/* Sync to ensure decoding is complete */
	status = vaSyncSurface(decoder->display, decoder->surface_id);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaSyncSurface failed: %d", status);
		vaDestroyBuffer(decoder->display, pic_param_buffer);
		vaDestroyBuffer(decoder->display, bitstream_buffer);
		return false;
	}

	/* Clean up buffers */
	vaDestroyBuffer(decoder->display, pic_param_buffer);
	vaDestroyBuffer(decoder->display, bitstream_buffer);

	/* Export surface to DMA-BUF */
	VADRMPRIMESurfaceDescriptor prime_desc;
	status = vaExportSurfaceHandle(
		decoder->display,
		decoder->surface_id,
		VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2,
		VA_EXPORT_SURFACE_READ_ONLY,
		&prime_desc
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to export surface to DMA-BUF: %d", status);
		return false;
	}

	/* Fill DMA-BUF attributes */
	attribs->width = width;
	attribs->height = height;
	attribs->format = prime_desc.fourcc;
	attribs->modifier = prime_desc.objects[0].drm_format_modifier;
	attribs->n_planes = prime_desc.num_layers;

	for (uint32_t i = 0; i < prime_desc.num_layers && i < GBM_MAX_PLANES; i++) {
		attribs->offset[i] = prime_desc.layers[i].offset[0];
		attribs->stride[i] = prime_desc.layers[i].pitch[0];
		attribs->fd[i] = fcntl(prime_desc.objects[prime_desc.layers[i].object_index[0]].fd,
			F_DUPFD_CLOEXEC, 0);

		if (attribs->fd[i] < 0) {
			wlf_log_errno(WLF_ERROR, "Failed to duplicate DMA-BUF fd");
			/* Clean up previously duplicated fds */
			for (uint32_t j = 0; j < i; j++) {
				close(attribs->fd[j]);
			}
			/* Close original fds */
			for (uint32_t j = 0; j < prime_desc.num_objects; j++) {
				close(prime_desc.objects[j].fd);
			}
			return false;
		}
	}

	/* Close original fds from VA-API */
	for (uint32_t i = 0; i < prime_desc.num_objects; i++) {
		close(prime_desc.objects[i].fd);
	}

	wlf_log(WLF_DEBUG, "JPEG decoded to DMA-BUF: %ux%u, format=0x%x, %d planes",
		width, height, attribs->format, attribs->n_planes);

	return true;
}

bool wlf_va_jpeg_decode_file_to_dmabuf(
	struct wlf_va_jpeg_decoder *decoder,
	const char *filename,
	struct wlf_dmabuf_attributes *attribs) {

	if (!decoder || !filename || !attribs) {
		wlf_log(WLF_ERROR, "Invalid parameters for JPEG decode from file");
		return false;
	}

	size_t size;
	uint8_t *data = read_file_to_memory(filename, &size);
	if (!data) {
		return false;
	}

	bool result = wlf_va_jpeg_decode_data_to_dmabuf(decoder, data, size, attribs);
	free(data);

	return result;
}

VASurfaceID wlf_va_jpeg_decoder_get_surface(struct wlf_va_jpeg_decoder *decoder) {
	if (!decoder) {
		return VA_INVALID_SURFACE;
	}
	return decoder->surface_id;
}

/* JPEG Encoder Implementation */

struct wlf_va_jpeg_encoder *wlf_va_jpeg_encoder_create(
	struct wlf_va_display *va_display,
	uint32_t quality) {

	struct wlf_va_jpeg_encoder *encoder = calloc(1, sizeof(*encoder));
	if (!encoder) {
		wlf_log(WLF_ERROR, "Failed to allocate JPEG encoder");
		return NULL;
	}

	encoder->va_display = va_display;
	if (va_display) {
		encoder->display = va_display->display;
	}
	encoder->config_id = VA_INVALID_ID;
	encoder->context_id = VA_INVALID_ID;
	encoder->quality = (quality > 0 && quality <= 100) ? quality : 85;
	encoder->initialized = false;

	return encoder;
}

void wlf_va_jpeg_encoder_destroy(struct wlf_va_jpeg_encoder *encoder) {
	if (!encoder) {
		return;
	}

	if (encoder->context_id != VA_INVALID_ID) {
		vaDestroyContext(encoder->display, encoder->context_id);
	}

	if (encoder->config_id != VA_INVALID_ID) {
		vaDestroyConfig(encoder->display, encoder->config_id);
	}

	free(encoder);
}

static bool wlf_va_jpeg_encoder_init(
	struct wlf_va_jpeg_encoder *encoder,
	uint32_t width,
	uint32_t height) {

	if (encoder->initialized) {
		/* Reinitialize if dimensions changed */
		if (encoder->width == width && encoder->height == height) {
			return true;
		}

		/* Clean up old resources */
		if (encoder->context_id != VA_INVALID_ID) {
			vaDestroyContext(encoder->display, encoder->context_id);
			encoder->context_id = VA_INVALID_ID;
		}
		if (encoder->config_id != VA_INVALID_ID) {
			vaDestroyConfig(encoder->display, encoder->config_id);
			encoder->config_id = VA_INVALID_ID;
		}
	}

	encoder->width = width;
	encoder->height = height;
	encoder->fourcc = VA_FOURCC_NV12;

	/* Create config for JPEG encoding */
	VAConfigAttrib attrib;
	attrib.type = VAConfigAttribRTFormat;
	attrib.value = VA_RT_FORMAT_YUV420;

	VAStatus status = vaCreateConfig(
		encoder->display,
		VAProfileJPEGBaseline,
		VAEntrypointEncPicture,
		&attrib,
		1,
		&encoder->config_id
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create VA config for JPEG encoding: %d", status);
		return false;
	}

	/* Context will be created during encoding as we need the surface */
	encoder->initialized = true;
	wlf_log(WLF_DEBUG, "JPEG encoder initialized: %ux%u, quality=%u",
		width, height, encoder->quality);
	return true;
}

bool wlf_va_jpeg_encode_dmabuf_to_data(
	struct wlf_va_jpeg_encoder *encoder,
	const struct wlf_dmabuf_attributes *attribs,
	uint8_t **data,
	size_t *size) {

	if (!encoder || !attribs || !data || !size) {
		wlf_log(WLF_ERROR, "Invalid parameters for JPEG encode");
		return false;
	}

	/* Initialize encoder with dimensions */
	if (!wlf_va_jpeg_encoder_init(encoder, attribs->width, attribs->height)) {
		return false;
	}

	/* Import DMA-BUF as VA surface */
	VASurfaceAttribExternalBuffers external;
	memset(&external, 0, sizeof(external));
	external.pixel_format = attribs->format;
	external.width = attribs->width;
	external.height = attribs->height;
	external.num_planes = attribs->n_planes;

	unsigned long fds[GBM_MAX_PLANES];
	for (int i = 0; i < attribs->n_planes; i++) {
		external.pitches[i] = attribs->stride[i];
		external.offsets[i] = attribs->offset[i];
		fds[i] = attribs->fd[i];
	}
	external.buffers = fds;
	external.num_buffers = attribs->n_planes;

	VASurfaceAttrib surface_attribs[2];
	surface_attribs[0].type = VASurfaceAttribMemoryType;
	surface_attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
	surface_attribs[0].value.type = VAGenericValueTypeInteger;
	surface_attribs[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;

	surface_attribs[1].type = VASurfaceAttribExternalBufferDescriptor;
	surface_attribs[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
	surface_attribs[1].value.type = VAGenericValueTypePointer;
	surface_attribs[1].value.value.p = &external;

	VASurfaceID surface_id;
	VAStatus status = vaCreateSurfaces(
		encoder->display,
		VA_RT_FORMAT_YUV420,
		attribs->width,
		attribs->height,
		&surface_id,
		1,
		surface_attribs,
		2
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to import DMA-BUF as VA surface: %d", status);
		return false;
	}

	/* Create context if not exists */
	if (encoder->context_id == VA_INVALID_ID) {
		status = vaCreateContext(
			encoder->display,
			encoder->config_id,
			attribs->width,
			attribs->height,
			VA_PROGRESSIVE,
			&surface_id,
			1,
			&encoder->context_id
		);

		if (status != VA_STATUS_SUCCESS) {
			wlf_log(WLF_ERROR, "Failed to create VA context: %d", status);
			vaDestroySurfaces(encoder->display, &surface_id, 1);
			return false;
		}
	}

	/* Create picture parameter buffer for JPEG encoding */
	VAEncPictureParameterBufferJPEG pic_param;
	memset(&pic_param, 0, sizeof(pic_param));
	pic_param.picture_width = attribs->width;
	pic_param.picture_height = attribs->height;
	pic_param.quality = encoder->quality;

	VABufferID pic_param_buffer;
	status = vaCreateBuffer(
		encoder->display,
		encoder->context_id,
		VAEncPictureParameterBufferType,
		sizeof(pic_param),
		1,
		&pic_param,
		&pic_param_buffer
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create picture parameter buffer: %d", status);
		vaDestroySurfaces(encoder->display, &surface_id, 1);
		return false;
	}

	/* Begin picture */
	status = vaBeginPicture(encoder->display, encoder->context_id, surface_id);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaBeginPicture failed: %d", status);
		vaDestroyBuffer(encoder->display, pic_param_buffer);
		vaDestroySurfaces(encoder->display, &surface_id, 1);
		return false;
	}

	/* Render picture */
	status = vaRenderPicture(encoder->display, encoder->context_id,
		&pic_param_buffer, 1);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaRenderPicture failed: %d", status);
		vaEndPicture(encoder->display, encoder->context_id);
		vaDestroyBuffer(encoder->display, pic_param_buffer);
		vaDestroySurfaces(encoder->display, &surface_id, 1);
		return false;
	}

	/* End picture */
	status = vaEndPicture(encoder->display, encoder->context_id);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaEndPicture failed: %d", status);
		vaDestroyBuffer(encoder->display, pic_param_buffer);
		vaDestroySurfaces(encoder->display, &surface_id, 1);
		return false;
	}

	/* Sync */
	status = vaSyncSurface(encoder->display, surface_id);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaSyncSurface failed: %d", status);
		vaDestroyBuffer(encoder->display, pic_param_buffer);
		vaDestroySurfaces(encoder->display, &surface_id, 1);
		return false;
	}

	/* Get encoded data */
	VACodedBufferSegment *segment;
	status = vaMapBuffer(encoder->display, pic_param_buffer, (void **)&segment);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "vaMapBuffer failed: %d", status);
		vaDestroyBuffer(encoder->display, pic_param_buffer);
		vaDestroySurfaces(encoder->display, &surface_id, 1);
		return false;
	}

	/* Copy encoded data */
	*size = segment->size;
	*data = malloc(*size);
	if (!*data) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for encoded data");
		vaUnmapBuffer(encoder->display, pic_param_buffer);
		vaDestroyBuffer(encoder->display, pic_param_buffer);
		vaDestroySurfaces(encoder->display, &surface_id, 1);
		return false;
	}

	memcpy(*data, segment->buf, *size);

	/* Clean up */
	vaUnmapBuffer(encoder->display, pic_param_buffer);
	vaDestroyBuffer(encoder->display, pic_param_buffer);
	vaDestroySurfaces(encoder->display, &surface_id, 1);

	wlf_log(WLF_DEBUG, "JPEG encoded from DMA-BUF: %zu bytes", *size);
	return true;
}

bool wlf_va_jpeg_encode_dmabuf_to_file(
	struct wlf_va_jpeg_encoder *encoder,
	const struct wlf_dmabuf_attributes *attribs,
	const char *filename) {

	if (!encoder || !attribs || !filename) {
		wlf_log(WLF_ERROR, "Invalid parameters for JPEG encode to file");
		return false;
	}

	uint8_t *data;
	size_t size;
	if (!wlf_va_jpeg_encode_dmabuf_to_data(encoder, attribs, &data, &size)) {
		return false;
	}

	bool result = write_memory_to_file(filename, data, size);
	free(data);

	return result;
}

bool wlf_va_jpeg_encoder_set_quality(
	struct wlf_va_jpeg_encoder *encoder,
	uint32_t quality) {

	if (!encoder) {
		return false;
	}

	if (quality < 1 || quality > 100) {
		wlf_log(WLF_ERROR, "JPEG quality must be between 1 and 100");
		return false;
	}

	encoder->quality = quality;
	return true;
}
