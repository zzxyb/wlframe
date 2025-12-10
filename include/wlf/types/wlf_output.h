/**
 * @file        wlf_output.h
 * @brief       Output device abstraction for wlframe.
 * @details     This file defines the structures, enumerations, and functions
 *              used to represent output devices (monitors), including geometry,
 *              physical properties, modes, transformations, events, and output
 *              management.
 *
 *              It provides a backend-independent representation of outputs
 *              similar to Wayland wl_output, and an output manager that tracks
 *              creation and destruction of outputs.
 *
 * @author      YaoBing Xiao
 * @date        2025-12-10
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-12-10, initial version\n
 */

#ifndef TYPES_WLF_OUTPUT_H
#define TYPES_WLF_OUTPUT_H

#include "wlf/math/wlf_size.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_linked_list.h"

struct wlf_output;

/**
 * @brief Output transform types.
 * @details This enum type specifies the various orientations a Output might have.
 */
enum wlf_output_transform {
	WLF_OUTPUT_TRANSFORM_NORMAL = 0,   /**< No rotation */
	WLF_OUTPUT_TRANSFORM_90,           /**< Rotated 90 degrees clockwise */
	WLF_OUTPUT_TRANSFORM_180,          /**< Rotated 180 degrees */
	WLF_OUTPUT_TRANSFORM_270,          /**< Rotated 270 degrees clockwise */
	WLF_OUTPUT_TRANSFORM_FLIPPED,      /**< Flipped horizontally */
	WLF_OUTPUT_TRANSFORM_FLIPPED_90,   /**< Flipped horizontally + rotated 90° */
	WLF_OUTPUT_TRANSFORM_FLIPPED_180,  /**< Flipped horizontally + rotated 180° */
	WLF_OUTPUT_TRANSFORM_FLIPPED_270   /**< Flipped horizontally + rotated 270° */
};

/**
 * @brief Subpixel layout of an output panel.
 * @details Matches wl_output.subpixel.
 */
enum wlf_output_subpixel {
	WLF_OUTPUT_SUBPIXEL_UNKNOWN = 0,       /**< Unknown layout */
	WLF_OUTPUT_SUBPIXEL_NONE,              /**< No subpixel structure */
	WLF_OUTPUT_SUBPIXEL_HORIZONTAL_RGB,    /**< Horizontal RGB (→) */
	WLF_OUTPUT_SUBPIXEL_HORIZONTAL_BGR,    /**< Horizontal BGR (→) */
	WLF_OUTPUT_SUBPIXEL_VERTICAL_RGB,      /**< Vertical RGB (↓) */
	WLF_OUTPUT_SUBPIXEL_VERTICAL_BGR       /**< Vertical BGR (↓) */
};

/**
 * @brief Output implementation type.
 * @details Backends extend output via wlf_output_impl.
 */
enum wlf_output_type {
	WLF_OUTPUT = 0,        /**< Real/physical monitor */
	WLF_OUTPUT_VIRTUAL     /**< Virtual output */
};

/**
 * @brief Backend-specific output implementation.
 */
struct wlf_output_impl {
	enum wlf_output_type type;                   /**< Output type */
	void (*destroy)(struct wlf_output *output);  /**< Implementation destroy callback */
};

/**
 * @brief Represents a display output (monitor).
 * @details
 * Contains identity information (name/model), geometry, physical properties,
 * transform, subpixel type, refresh rate, scale, and all relevant update signals.
 */
struct wlf_output {
	const struct wlf_output_impl *impl;  /**< Associated backend implementation */

	/**
	 * @brief Output-related events.
	 * @details Fired when corresponding properties change.
	 */
	struct {
		struct wlf_signal destroy;               /**< Output is being destroyed */
		struct wlf_signal name_change;           /**< Name updated */
		struct wlf_signal model_change;          /**< Model updated */
		struct wlf_signal manufacturer_change;   /**< Manufacturer updated */
		struct wlf_signal description_change;    /**< Description updated */

		struct wlf_signal geometry_change;       /**< Geometry updated */
		struct wlf_signal physical_size_change;  /**< Physical size updated */
		struct wlf_signal refreshRate_change;    /**< Refresh rate updated */
		struct wlf_signal scale_change;          /**< Scale updated */
		struct wlf_signal transform_change;      /**< Transform updated */
		struct wlf_signal subpixel_change;       /**< Subpixel layout updated */
	} events;

	/* ---- Identity ---- */
	char *name;          /**< Human-readable identifier (e.g., "HDMI-1") */
	char *model;         /**< Display model (e.g., "DELL U2720Q") */
	char *manufacturer;  /**< Manufacturer name */
	char *description;   /**< Optional description */

	/* ---- Geometry & physical properties ---- */
	struct wlf_rect geometry;     /**< Position + resolution in compositor space */
	struct wlf_size physical_size;/**< Physical size in millimeters */

	/* ---- Display properties ---- */
	int refresh_rate;                 /**< Refresh rate in Hz (e.g., 60000 = 60Hz) */
	int scale;                        /**< Output scale factor */
	enum wlf_output_transform transform; /**< Rotation/flip transform */
	enum wlf_output_subpixel subpixel;   /**< Subpixel layout */

	struct wlf_linked_list link; /**< Linked list node for output manager */
};

/**
 * @brief Initializes an output structure.
 * @param output Output to initialize.
 * @param impl Backend-specific implementation.
 */
void wlf_output_init(struct wlf_output *output,
	const struct wlf_output_impl *impl);

/**
 * @brief Destroys an output and emits destroy signal.
 * @param output Output to destroy.
 */
void wlf_output_destroy(struct wlf_output *output);

struct wlf_output_manager;

/**
 * @brief Backend implementation for wlf_output_manager.
 */
struct wlf_output_manager_impl {
	void (*destroy)(struct wlf_output_manager *manager); /**< Destroy callback */
};

/**
 * @brief Manages a list of outputs.
 * @details Emits signals when outputs are added or removed.
 */
struct wlf_output_manager {
	struct wlf_linked_list outputs; /**< All registered outputs */

	const struct wlf_output_manager_impl *impl; /**< Backend implementation */

	struct {
		struct wlf_signal destroy;         /**< Manager destroyed */
		struct wlf_signal output_added;    /**< Output added */
		struct wlf_signal output_removed;  /**< Output removed */
	} events;
};

/**
 * @brief Initializes an output manager.
 * @param manager Output manager.
 * @param impl Backend implementation.
 */
void wlf_output_manager_init(struct wlf_output_manager *manager,
	const struct wlf_output_manager_impl *impl);

/**
 * @brief Destroys the output manager and all associated outputs.
 * @param manager Output manager.
 */
void wlf_output_manager_destroy(struct wlf_output_manager *manager);

#endif // TYPES_WLF_OUTPUT_H
