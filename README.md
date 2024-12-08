# wlframe
wlframe is a C UI framework for muti platform.

## Contributing to wlframe

Please refer to the [contributing document](CONTRIBUTING.md) for everything you need to know to get started contributing to wlframe.

### Compiling from Source
**Install dependencies:**

build dependencies(for arch linux):
```shell
pacman -S cairo mesa pixman libpng pango librsvg systemd wayland vulkan-headers libxkbcommon glslang wayland-protocols base-devel meson clang ninja cppcheck doxygen graphviz libxslt xmlto pkgconf
```

build dependencies(for debian):
```shell
apt install libcairo2-dev libegl1-mesa-dev libegl-dev libgles2-mesa-dev libpixman-1-dev libpng-dev libpango1.0-dev librsvg2-dev libsystemd-dev libvulkan-dev libwayland-dev libxkbcommon-dev wayland-protocols glslang-tools meson clang cppcheck doxygen graphviz libxslt1-dev xmlto dpkg-dev pkgconf
```

**Run these commands:**
```shell
    meson build/ --prefix=/usr --buildtype=debug
    ninja -C build/
```

## doxygen
set the documentation in meson_options.txt to enabled, reuse meson to compile, and you will see that the documentation has been generated in the build/doc/doxygen/html/wlframe directory.
