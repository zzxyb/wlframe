/**
 * @file        wlf_zxdg_foreign_v1.h
 * @brief       zxdg-foreign-v1 client wrappers.
 * @details     Provides export and import helpers for associating surfaces
 *              across Wayland clients.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_ZXDG_FOREIGN_V1_H
#define WLF_ZXDG_FOREIGN_V1_H

#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct wl_registry;
struct wl_surface;
struct zxdg_exporter_v1;
struct zxdg_importer_v1;
struct zxdg_exported_v1;
struct zxdg_imported_v1;

/**
 * @brief Exported surface handle wrapper.
 *
 * The handle is valid until the compositor sends the destroyed event.
 */
struct wlf_zxdg_exported_v1 {
	struct zxdg_exported_v1 *base; /**< Protocol object. */
	char *handle; /**< Exported handle received from the compositor. */

	struct {
		struct wlf_signal handle;   /**< Payload: wlf_zxdg_exported_v1. */
		struct wlf_signal destroy;  /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Imported surface handle wrapper.
 *
 * The imported object can be assigned as the parent of a local surface.
 */
struct wlf_zxdg_imported_v1 {
	struct zxdg_imported_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroyed; /**< Payload: wlf_zxdg_imported_v1. */
		struct wlf_signal destroy;    /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Exporter global wrapper.
 *
 * The wrapper creates exported-handle objects for local surfaces.
 */
struct wlf_zxdg_exporter_v1 {
	struct zxdg_exporter_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Importer global wrapper.
 *
 * The wrapper creates imported-handle objects from exported handles.
 */
struct wlf_zxdg_importer_v1 {
	struct zxdg_importer_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Creates an exporter wrapper from a registry global.
 * @param registry Wayland registry used to bind the exporter.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated exporter, or NULL on failure.
 */
struct wlf_zxdg_exporter_v1 *wlf_zxdg_exporter_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys an exporter wrapper.
 * @param exporter Exporter to destroy.
 */
void wlf_zxdg_exporter_v1_destroy(struct wlf_zxdg_exporter_v1 *exporter);

/**
 * @brief Exports a surface and returns its handle wrapper.
 * @param exporter Exporter used for the request.
 * @param surface Surface to export.
 * @return Newly allocated exported handle, or NULL on failure.
 */
struct wlf_zxdg_exported_v1 *wlf_zxdg_exporter_v1_export(
	struct wlf_zxdg_exporter_v1 *exporter, struct wl_surface *surface);

/**
 * @brief Destroys an exported-surface wrapper.
 * @param exported Exported handle to destroy.
 */
void wlf_zxdg_exported_v1_destroy(struct wlf_zxdg_exported_v1 *exported);

/**
 * @brief Creates an importer wrapper from a registry global.
 * @param registry Wayland registry used to bind the importer.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated importer, or NULL on failure.
 */
struct wlf_zxdg_importer_v1 *wlf_zxdg_importer_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys an importer wrapper.
 * @param importer Importer to destroy.
 */
void wlf_zxdg_importer_v1_destroy(struct wlf_zxdg_importer_v1 *importer);

/**
 * @brief Imports an exported handle.
 * @param importer Importer used for the request.
 * @param handle Exported handle string.
 * @return Newly allocated imported handle, or NULL on failure.
 */
struct wlf_zxdg_imported_v1 *wlf_zxdg_importer_v1_import(
	struct wlf_zxdg_importer_v1 *importer, const char *handle);

/**
 * @brief Sets the parent surface for an imported surface.
 * @param imported Imported handle to configure.
 * @param surface Parent surface.
 */
void wlf_zxdg_imported_v1_set_parent_of(
	struct wlf_zxdg_imported_v1 *imported, struct wl_surface *surface);

/**
 * @brief Destroys an imported-surface wrapper.
 * @param imported Imported handle to destroy.
 */
void wlf_zxdg_imported_v1_destroy(struct wlf_zxdg_imported_v1 *imported);

#endif /* WLF_ZXDG_FOREIGN_V1_H */
