/**
 * @file        wlf_ext_image_capture_source_manager_v1.h
 * @brief       ext-image-capture-source-v1 client wrappers.
 * @details     Wraps output and foreign-toplevel capture-source managers and
 *              exposes their lifecycle through wlframe signals.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_EXT_IMAGE_CAPTURE_SOURCE_MANAGER_V1_H
#define WLF_EXT_IMAGE_CAPTURE_SOURCE_MANAGER_V1_H

#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct ext_foreign_toplevel_handle_v1;
struct ext_foreign_toplevel_image_capture_source_manager_v1;
struct ext_image_capture_source_v1;
struct ext_output_image_capture_source_manager_v1;
struct wl_output;
struct wl_registry;

/**
 * @brief Generic image-capture source wrapper.
 *
 * The source is consumed by an image-copy-capture session and emits destroy
 * when its protocol object is no longer usable.
 */
struct wlf_ext_image_capture_source_v1 {
	struct ext_image_capture_source_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Output image-capture source manager wrapper.
 *
 * The manager creates capture sources referring to wl_output objects.
 */
struct wlf_ext_output_image_capture_source_manager_v1 {
	struct ext_output_image_capture_source_manager_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Foreign-toplevel image-capture source manager wrapper.
 *
 * The manager creates capture sources referring to foreign-toplevel handles.
 */
struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 {
	struct ext_foreign_toplevel_image_capture_source_manager_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Creates an output image-capture source manager wrapper.
 * @param registry Wayland registry used to bind the manager.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_ext_output_image_capture_source_manager_v1 *
wlf_ext_output_image_capture_source_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys an output image-capture source manager.
 * @param manager Manager to destroy.
 */
void wlf_ext_output_image_capture_source_manager_v1_destroy(
	struct wlf_ext_output_image_capture_source_manager_v1 *manager);

/**
 * @brief Creates a capture source for @p output.
 * @param manager Output source manager.
 * @param output Output to capture.
 * @return Newly allocated capture source, or NULL on failure.
 */
struct wlf_ext_image_capture_source_v1 *
wlf_ext_output_image_capture_source_manager_v1_create_source(
	struct wlf_ext_output_image_capture_source_manager_v1 *manager,
	struct wl_output *output);

/**
 * @brief Creates a foreign-toplevel image-capture source manager wrapper.
 * @param registry Wayland registry used to bind the manager.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 *
wlf_ext_foreign_toplevel_image_capture_source_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys a foreign-toplevel image-capture source manager.
 * @param manager Manager to destroy.
 */
void wlf_ext_foreign_toplevel_image_capture_source_manager_v1_destroy(
	struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 *manager);

/**
 * @brief Creates a capture source for @p toplevel_handle.
 * @param manager Foreign-toplevel source manager.
 * @param toplevel_handle Foreign-toplevel handle to capture.
 * @return Newly allocated capture source, or NULL on failure.
 */
struct wlf_ext_image_capture_source_v1 *
wlf_ext_foreign_toplevel_image_capture_source_manager_v1_create_source(
	struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 *manager,
	struct ext_foreign_toplevel_handle_v1 *toplevel_handle);

/**
 * @brief Destroys an image-capture source wrapper.
 * @param source Source to destroy.
 */
void wlf_ext_image_capture_source_v1_destroy(
	struct wlf_ext_image_capture_source_v1 *source);

#endif /* WLF_EXT_IMAGE_CAPTURE_SOURCE_MANAGER_V1_H */
