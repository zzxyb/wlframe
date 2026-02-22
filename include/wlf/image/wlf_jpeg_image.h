/**
 * @file        wlf_jpeg_image.h
 * @brief       JPEG image handling and utility functions for wlframe.
 * @details     This file defines the wlf_jpeg_image structure and related functions,
 *              providing a unified interface for creating, converting, and processing
 *              JPEG images within wlframe. It includes support for quality settings,
 *              colorspace conversion, and progressive JPEG formats.
 *
 *              Typical usage:
 *                  - Create a JPEG image object.
 *                  - Convert between wlf_image and wlf_jpeg_image.
 *                  - Set quality parameters for JPEG compression.
 *                  - Handle progressive JPEG encoding/decoding.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-16
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-16, initial version\n
 */

#ifndef IMAGE_WLF_JPEG_IMAGE_H
#define IMAGE_WLF_JPEG_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief Supported JPEG colorspace types.
 * @note These colorspaces represent how color information is encoded
 *       in JPEG files. Different colorspaces are suitable for different
 *       types of images and applications.
 */
enum wlf_jpeg_colorspace {
	WLF_JPEG_COLORSPACE_UNKNOWN = 0, /**< Unknown colorspace */
	WLF_JPEG_COLORSPACE_GRAYSCALE,   /**< Grayscale (1 component) - for black & white images */
	WLF_JPEG_COLORSPACE_RGB,         /**< RGB (3 components) - standard computer graphics */
	WLF_JPEG_COLORSPACE_YCC,         /**< YCbCr (3 components) - standard JPEG colorspace */
	WLF_JPEG_COLORSPACE_CMYK,        /**< CMYK (4 components) - printing applications */
	WLF_JPEG_COLORSPACE_YCCK,        /**< YCCK (4 components) - CMYK with luminance/chrominance */
};

/**
 * @brief Supported JPEG subsampling modes.
 * @note Chroma subsampling exploits the human visual system's lower
 *       sensitivity to color information compared to brightness.
 *       The numbers indicate the sampling ratio for Y:Cb:Cr components.
 */
enum wlf_jpeg_subsampling {
	WLF_JPEG_SUBSAMPLING_444 = 0, /**< 4:4:4 - No subsampling (best quality, largest files) */
	WLF_JPEG_SUBSAMPLING_422,     /**< 4:2:2 - Horizontal subsampling (good balance) */
	WLF_JPEG_SUBSAMPLING_420,     /**< 4:2:0 - Both horizontal and vertical subsampling (most common) */
	WLF_JPEG_SUBSAMPLING_411,     /**< 4:1:1 - Aggressive horizontal subsampling (smallest files) */
};

/**
 * @brief JPEG compression options.
 * @note These options control the trade-off between file size and quality.
 *       Different combinations are suitable for different use cases:
 *       - Web images: quality 70-85, subsampling 420, progressive enabled
 *       - Print images: quality 90-95, subsampling 444, optimize enabled
 *       - Thumbnails: quality 60-75, subsampling 420, optimize disabled
 */
struct wlf_jpeg_options {
	int quality;                            /**< Quality setting (0-100, where 100 is best) */
	enum wlf_jpeg_subsampling subsampling;  /**< Chroma subsampling mode */
	bool progressive;                       /**< Enable progressive JPEG encoding */
	bool optimize;                          /**< Enable Huffman table optimization */
	bool arithmetic;                        /**< Use arithmetic coding instead of Huffman */
};

/**
 * @brief JPEG image structure, extending wlf_image with JPEG-specific information.
 */
struct wlf_jpeg_image {
	struct wlf_image base;                  /**< Base image structure */
	enum wlf_jpeg_colorspace colorspace;    /**< JPEG colorspace */
	struct wlf_jpeg_options options;        /**< JPEG compression options */
	bool is_progressive;                    /**< True if image uses progressive encoding */
};

/**
 * @brief Create a new wlf_jpeg_image object.
 * @return Pointer to a newly allocated wlf_jpeg_image structure, or NULL on failure.
 */
struct wlf_jpeg_image *wlf_jpeg_image_create(void);

/**
 * @brief Create a new wlf_jpeg_image object with specific options.
 * @param options Pointer to JPEG compression options.
 * @return Pointer to a newly allocated wlf_jpeg_image structure, or NULL on failure.
 */
struct wlf_jpeg_image *wlf_jpeg_image_create_with_options(const struct wlf_jpeg_options *options);

/**
 * @brief Convert a wlf_image pointer to a wlf_jpeg_image pointer.
 * @param wlf_image Pointer to the base wlf_image structure.
 * @return Pointer to the corresponding wlf_jpeg_image structure.
 */
struct wlf_jpeg_image *wlf_jpeg_image_from_image(struct wlf_image *wlf_image);

/**
 * @brief Check if a wlf_image is a JPEG image.
 * @param image Pointer to the wlf_image structure to check.
 * @return true if the image is a JPEG image, false otherwise.
 * @note This function checks the image type and implementation to determine
 *       if the image is a JPEG. It's useful for type checking before
 *       calling JPEG-specific functions.
 */
bool wlf_image_is_jpeg(const struct wlf_image *image);

/**
 * @brief Convert a wlf_image color type to JPEG colorspace.
 * @param image Pointer to the wlf_image structure.
 * @return Corresponding JPEG colorspace value.
 * @note This function maps wlframe color types to JPEG colorspace enums.
 *       RGBA and GRAY_ALPHA formats will be mapped to their non-alpha
 *       equivalents since JPEG doesn't support transparency.
 */
enum wlf_jpeg_colorspace wlf_color_type_to_jpeg_colorspace(const struct wlf_image *image);

/**
 * @brief Set JPEG quality for an image.
 * @param jpeg_image Pointer to the wlf_jpeg_image structure.
 * @param quality Quality setting (0-100, where 100 is best quality).
 * @return true on success, false on failure.
 * @note Quality affects both file size and visual quality. Higher values
 *       produce better quality but larger files. Typical values:
 *       - 10-30: Low quality, small files
 *       - 50-70: Medium quality, reasonable files
 *       - 80-95: High quality, larger files
 *       - 95-100: Maximum quality, very large files
 */
bool wlf_jpeg_image_set_quality(struct wlf_jpeg_image *jpeg_image, int quality);

/**
 * @brief Set JPEG subsampling mode for an image.
 * @param jpeg_image Pointer to the wlf_jpeg_image structure.
 * @param subsampling Subsampling mode to set.
 * @return true on success, false on failure.
 * @note Chroma subsampling reduces file size by storing color information
 *       at lower resolution than brightness. Options:
 *       - 4:4:4: No subsampling (best quality, largest size)
 *       - 4:2:2: Horizontal subsampling (good balance)
 *       - 4:2:0: Both directions (standard, smaller size)
 *       - 4:1:1: Aggressive horizontal (smallest size)
 */
bool wlf_jpeg_image_set_subsampling(struct wlf_jpeg_image *jpeg_image, enum wlf_jpeg_subsampling subsampling);

/**
 * @brief Enable or disable progressive JPEG encoding.
 * @param jpeg_image Pointer to the wlf_jpeg_image structure.
 * @param progressive True to enable progressive encoding, false to disable.
 * @return true on success, false on failure.
 * @note Progressive JPEG allows images to be displayed incrementally
 *       as they download, showing a low-quality version first that
 *       gradually improves. This is useful for web applications but
 *       may slightly increase file size.
 */
bool wlf_jpeg_image_set_progressive(struct wlf_jpeg_image *jpeg_image, bool progressive);

/**
 * @brief Get default JPEG options.
 * @return Default JPEG compression options.
 */
struct wlf_jpeg_options wlf_jpeg_get_default_options(void);

#endif // IMAGE_WLF_JPEG_IMAGE_H
