# wlframe
wlframe is a C UI framework for muti platform.

## Contributing to wlframe

Please refer to the [contributing document](CONTRIBUTING.md) for everything you need to know to get started contributing to wlframe.

### Compiling from Source
**Install dependencies:**

build dependencies(for arch linux):
```shell
pacman -S \
	base-devel meson gcc clang ninja pkgconf cppcheck wayland libpng
```

**Run these commands:**
```shell
    meson build/ --prefix=/usr --buildtype=debug
    ninja -C build/
```

## doxygen
set the documentation in meson_options.txt to enabled, reuse meson to compile, and you will see that the documentation has been generated in the build/doc/doxygen/html/wlframe directory.
