# JPEG Image Support for wlframe

## Overview

The wlframe framework now includes comprehensive JPEG image support through the `wlf_jpeg_image` module, providing high-quality JPEG encoding and decoding capabilities with extensive customization options.

## Features

### Core Features
- **JPEG Loading and Saving**: Full support for reading and writing JPEG files
- **Quality Control**: Adjustable compression quality from 0-100
- **Progressive JPEG**: Support for progressive encoding for better web delivery
- **Multiple Colorspaces**: Support for RGB, Grayscale, YCbCr, CMYK, and YCCK colorspaces
- **Chroma Subsampling**: Configurable subsampling modes (4:4:4, 4:2:2, 4:2:0, 4:1:1)
- **Optimization Options**: Huffman table optimization and arithmetic coding support

### API Components

#### Core Structures
- `struct wlf_jpeg_image`: Main JPEG image structure extending `wlf_image`
- `struct wlf_jpeg_options`: Compression and encoding options
- `enum wlf_jpeg_colorspace`: Supported colorspace types
- `enum wlf_jpeg_subsampling`: Chroma subsampling modes

#### Key Functions
- `wlf_jpeg_image_create()`: Create new JPEG image objects
- `wlf_jpeg_image_create_with_options()`: Create with custom options
- `wlf_jpeg_image_set_quality()`: Set compression quality
- `wlf_jpeg_image_set_subsampling()`: Configure chroma subsampling
- `wlf_jpeg_image_set_progressive()`: Enable/disable progressive encoding

## Usage Examples

### Basic JPEG Creation and Saving

```c
#include "wlf/image/wlf_jpeg_image.h"

// Create JPEG image
struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_create();

// Set quality (0-100, where 100 is best quality)
wlf_jpeg_image_set_quality(jpeg_image, 85);

// Configure image data
struct wlf_image *base = &jpeg_image->base;
base->width = 800;
base->height = 600;
base->format = WLF_COLOR_TYPE_RGB;
// ... set image data ...

// Save to file
base->impl->save(base, "output.jpg");
```

### Advanced Configuration

```c
// Create with custom options
struct wlf_jpeg_options options = {
    .quality = 95,
    .subsampling = WLF_JPEG_SUBSAMPLING_444,  // No subsampling
    .progressive = true,                       // Progressive encoding
    .optimize = true,                         // Optimize Huffman tables
    .arithmetic = false                       // Use Huffman coding
};

struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_create_with_options(&options);
```

### Loading JPEG Files

```c
struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_create();
struct wlf_image *base = &jpeg_image->base;

// Load JPEG file
if (base->impl->load(base, "input.jpg", false)) {
    printf("Loaded JPEG: %dx%d, colorspace: %d\n",
           base->width, base->height, jpeg_image->colorspace);

    // Access image data
    unsigned char *pixels = base->data;
    // ... process pixels ...
}
```

## Quality and File Size Trade-offs

The JPEG implementation provides excellent control over the quality/size trade-off:

- **Quality 30**: Highly compressed, smaller files, visible artifacts
- **Quality 60**: Good compression with acceptable quality
- **Quality 85**: Standard quality, good balance (default)
- **Quality 95**: High quality, larger files
- **Quality 100**: Maximum quality, largest files

## Subsampling Options

Chroma subsampling reduces file size by compressing color information:

- **4:4:4**: No subsampling, best quality, largest files
- **4:2:2**: Horizontal subsampling, good for most images
- **4:2:0**: Both directions, standard for web (default)
- **4:1:1**: Aggressive compression, smallest files

## Alpha Channel Handling

JPEG format doesn't support alpha channels. The implementation automatically:
- Converts RGBA to RGB (drops alpha channel)
- Converts Grayscale+Alpha to Grayscale (drops alpha channel)
- Logs informational messages about these conversions

## Progressive JPEG

Progressive JPEG enables:
- Faster perceived loading on web
- Better user experience for large images
- Slightly larger file sizes
- Compatible with all modern browsers

## Integration with wlframe

The JPEG support is fully integrated with the wlframe image system:
- Uses the same `wlf_image` base interface as PNG and other formats
- Shares memory management and error handling
- Compatible with existing image processing pipelines
- Follows wlframe coding standards and patterns

## Test Programs

### Basic Test: `jpeg_image_test`
Tests core functionality:
- Option setting and validation
- Color type conversions
- Save/load operations
- Quality comparisons

### Advanced Demo: `jpeg_advanced_demo`
Demonstrates real-world usage:
- PNG to JPEG conversion
- Progressive encoding
- Subsampling comparisons
- File size analysis

## Build Requirements

- libjpeg (IJG JPEG library or compatible)
- Standard C library
- wlframe core libraries

## Files Added

### Headers
- `include/wlf/image/wlf_jpeg_image.h`: Public API

### Implementation
- `image/wlf_jpeg_image.c`: Core implementation

### Examples
- `examples/image/jpeg_image_test.c`: Basic functionality test
- `examples/image/jpeg_advanced_demo.c`: Advanced usage demonstration

### Build Configuration
- Updated `meson.build` files to include JPEG support
- Added libjpeg dependency

## Performance Notes

- JPEG encoding is CPU-intensive, especially with optimization enabled
- Progressive JPEG requires additional memory during encoding
- Quality settings above 95 show diminishing returns in visual quality
- Subsampling provides significant file size reduction with minimal quality loss for most photographic content

## Error Handling

The implementation includes robust error handling:
- Custom JPEG error manager with longjmp recovery
- Detailed error logging through wlframe logging system
- Graceful fallback for unsupported features
- Memory leak prevention in error paths
