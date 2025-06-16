# wlframe
**wlframe** is a cross-platform C UI framework with comprehensive image processing capabilities.

## Features

### Image Processing
- **PNG Support**: Full PNG loading and saving with transparency support
- **JPEG Support**: High-quality JPEG encoding/decoding with customizable compression
  - Quality control (0-100)
  - Progressive JPEG encoding
  - Multiple chroma subsampling modes (4:4:4, 4:2:2, 4:2:0, 4:1:1)
  - Colorspace support (RGB, Grayscale, YCbCr, CMYK, YCCK)
- **Unified Image API**: Common interface for all image formats

### Core Framework
- Cross-platform compatibility (Linux, macOS)
- Wayland support for modern Linux desktop environments
- Comprehensive logging and debugging utilities
- Memory management and data structures
- Signal/event system

### Compiling from Source
**Install dependencies:**

build dependencies(for arch linux):
```shell
pacman -S \
	base-devel meson gcc clang ninja pkgconf cppcheck wayland libpng libjpeg-turbo
```

**Run these commands:**
```shell
    meson build/ --prefix=/usr --buildtype=debug
    ninja -C build/
```

## Documentation

To generate API documentation with Doxygen:

1. Set the `documentation` option to `enabled` in `meson_options.txt`.
2. Re-run the Meson build.
3. The generated documentation will be available at:
   `build/doc/doxygen/html/wlframe/index.html`

## Contributing

See the [contributing guide](CONTRIBUTING.md) for details on how to get started with wlframe development.
