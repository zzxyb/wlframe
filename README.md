# wlframe
**wlframe** is a cross-platform C UI framework.

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

## Documentation

To generate API documentation with Doxygen:

1. Set the `documentation` option to `enabled` in `meson_options.txt`.
2. Re-run the Meson build.
3. The generated documentation will be available at:
   `build/doc/doxygen/html/wlframe/index.html`

## Contributing

See the [contributing guide](CONTRIBUTING.md) for details on how to get started with wlframe development.
