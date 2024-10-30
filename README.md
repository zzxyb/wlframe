# wlframe
wayland frame is a C UI framework for building Wayland applications.

## Contributing to wlframe

Please refer to the [contributing document](CONTRIBUTING.md) for everything you need to know to get started contributing to wlframe.

### Compiling from Source
**Install dependencies:**

build dependencies(for arch linux):
* meson \*
* dpkg \*
* cairo \*
* mesa \*
* pixman \*
* libpng \*
* pango \*
* xpm \*
* jpeg \*
* librsvg \*
* systemd \*
* vulkan \*
* wayland \*
* xcbcommon \*
* pkgconf \*
* wayland-protocols \*
* glslang \*
* cppcheck
* doxygen
* graphviz
* libxslt
* xmlto

build dependencies(for debian):
* meson \*
* dpkg-dev \*
* libcairo2-dev \*
* libegl1-mesa-dev \*
* libegl-dev \*
* libgles2-mesa-dev \*
* libpixman-1-dev \*
* libpng-dev \*
* libpango1.0-dev \*
* libxpm-dev \*
* libjpeg-dev \*
* librsvg2-dev \*
* libsystemd-dev \*
* libvulkan-dev \*
* libwayland-dev \*
* libxkbcommon-dev \*
* pkgconf \*
* wayland-protocols \*
* glslang-tools \*
* cppcheck
* doxygen
* graphviz
* libxslt1-dev
* xmlto

**Run these commands:**
```shell
    meson build/ --prefix=/usr --buildtype=debug
    sudo ninja -C build/
```

## doxygen
set the documentation in meson_options.txt to enabled, reuse meson to compile, and you will see that the documentation has been generated in the build/doc/doxygen/html/wlframe directory.
