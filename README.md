# wlframe

[![Arch Linux build](https://github.com/zzxyb/wlframe/actions/workflows/archlinux-build.yaml/badge.svg)](https://github.com/zzxyb/wlframe/actions/workflows/archlinux-build.yaml)
[![macOS build](https://github.com/zzxyb/wlframe/actions/workflows/macos-build.yaml/badge.svg)](https://github.com/zzxyb/wlframe/actions/workflows/macos-build.yaml)
[![Windows build](https://github.com/zzxyb/wlframe/actions/workflows/windows-build.yaml/badge.svg)](https://github.com/zzxyb/wlframe/actions/workflows/windows-build.yaml)

**wlframe** is an experimental, cross-platform UI and rendering framework
written in C. It provides the building blocks for native graphical
applications without imposing a widget toolkit or application architecture.

The project combines platform backends, GPU and software renderers, swapchains,
window management, a retained scene graph, vector graphics, text shaping,
images, input events, effects, and animations behind a small C API. Its design
is: objects are explicit, ownership is predictable, and applications can use
the low-level pieces independently.

> [!IMPORTANT]
> wlframe is under active development. The API is not stable yet and production
> use is not recommended without pinning a known-good revision.

## Screenshots

### Scene graph, text, shapes, and images

The scene graph can combine multilingual text, raster images, and vector
primitives in the same window.

<p align="center">
  <img src="screenshots/window.png" alt="wlframe rendering text, an image, and several vector shapes" width="900">
</p>

| SVG scene node | Damage-tracked rendering |
| --- | --- |
| <img src="screenshots/svg.png" alt="SVG scene rendered by wlframe" width="580"> | <img src="screenshots/damage.png" alt="wlframe scene damage example and Wayland protocol trace" width="580"> |
| SVG shapes and multilingual Pango/HarfBuzz text | Only changed regions are submitted to the compositor |

## What wlframe provides

- **Cross-platform foundations:** Wayland and X11 integration on Linux, AppKit event
  integration on macOS, and a Win32 event backend on Windows.
- **Multiple rendering paths:** OpenGL ES, Vulkan, and Pixman on Linux; Metal on
  macOS; and Direct3D 12 on Windows.
- **Retained scene graph:** trees and nodes for rectangles, circles, ellipses,
  lines, polygons, paths, textures, text, and SVG content.
- **Damage-aware rendering:** accumulated scene damage, visibility calculation,
  buffer-age repair, and debug visualization for damaged regions.
- **Vector graphics and effects:** SVG parsing, reusable shape trees, gradients,
  Gaussian blur, and drop shadows.
- **Text and images:** Pango/HarfBuzz text shaping on Linux and loaders for PNG,
  JPEG, WebP, GIF, BMP, PPM, and XPM images.
- **Window and input primitives:** toplevels, popups, dialogs, layer-shell
  surfaces, decorations, pointer, keyboard, and touch events.
- **Animation tools:** parallel and sequential animation groups with a collection
  of easing curves.
- **Small infrastructure utilities:** signals, intrusive lists, arrays, logging,
  command parsing, geometry, vectors, matrices, quaternions, and rays.

The platform and renderer selected by the default build are:

| Host | Platform backend | Renderer |
| --- | --- | --- |
| Linux | Wayland | OpenGL ES, then Vulkan, then Pixman fallback |
| Linux | X11 (XCB) | OpenGL ES, then Pixman fallback |
| macOS | AppKit | Metal |
| Windows | Win32 | Direct3D 12 |

The graphical window examples support Wayland and X11 on Linux. The
macOS and Windows backends and renderers are built continuously, but are still
being developed toward the same end-to-end feature coverage.

## Building from source

wlframe uses [Meson](https://mesonbuild.com/) 1.3 or newer and Ninja. Examples
are built by default; pass `-Dexamples=false` to omit them. The commands below
match the environments used by the project's continuous integration jobs.

Clone the repository first:

```sh
git clone https://github.com/zzxyb/wlframe.git
cd wlframe
```

### Linux (Arch Linux)

Install the compiler, build tools, and runtime development dependencies:

```sh
sudo pacman -S --needed \
    base-devel meson ninja gcc pkgconf \
    wayland wayland-protocols wlr-protocols libxcb xcb-util-wm xcb-util-xrm libxcursor \
    mesa libdrm vulkan-icd-loader vulkan-headers glslang \
    pixman cairo pango fontconfig harfbuzz glib2 \
    libpng libjpeg-turbo libwebp giflib
```

Configure and compile:

```sh
meson setup build \
    --buildtype=debug \
    --default-library=both \
    --prefix=/usr
meson compile -C build
```

Run the tests and, optionally, install the library:

```sh
meson test -C build --print-errorlogs
sudo meson install -C build
```

The Linux graphical examples can be launched directly from the build tree:

```sh
./build/examples/window/window_test
./build/examples/window/svg_node_test
./build/examples/window/scene_damage_test
```

On X11, run any of the graphical examples with the X11 backend:

```sh
WLF_BACKEND=x11 WLF_RENDERER=gles ./build/examples/window/scene_damage_test
WLF_BACKEND=x11 WLF_RENDERER=pixman ./build/examples/window/scene_damage_test
```

The backend automatically tries `WAYLAND_DISPLAY`, then `DISPLAY`. Set
`WLF_BACKEND=wayland` or `x11` to select one explicitly. Create X11 toplevels
with `wlf_x11_window_create_from_backend()` and pass their `base` to the usual
window, renderer, and scene APIs. Destroy windows before the renderer and backend.

X11 currently supports opaque toplevel windows, titles, size constraints,
server-side decorations, fullscreen/maximize requests, core pointer events,
and raw keyboard keycodes. Pixman requires a 24-bit TrueColor visual with
32-bit XRGB pixels. GLES requires `EGL_EXT_platform_xcb` and an EGL config
matching the root visual. Frames use a 16 ms software clock. X11 Vulkan,
transparent windows, fractional scaling, popups/layer-shell, XKB keymaps,
and IME integration are not implemented yet.

For software GLES, set `WLF_RENDER_FORCE_SOFTWARE=1`. See
[Environment variables](docs/env_vars.md) for all runtime switches.

### X11 font and cursor resources

The X11 backend owns an independent Xresource manager declared in
`include/wlf/platform/x11/xresource.h` and implemented in
`platform/x11/xresource.c`. It uses xcb-util-xrm for instance/class matching,
wildcards, includes, escapes, and database precedence. Resources are loaded
from screen zero's `RESOURCE_MANAGER`; when absent, `~/.Xresources` then
`~/.Xdefaults` are tried. `XENVIRONMENT`, or `~/.Xdefaults-hostname` when it is
unset, is merged with override enabled. This is Xresource support; XSettings
properties such as `Gtk/FontName` are not interpreted as Xresources.

Supported fields are `Xft.dpi`, `Xft.antialias`, `Xft.hinting`, `Xft.hintstyle`,
`Xft.rgba`, `Xft.lcdfilter`, `Xcursor.theme`, `Xcursor.size`, and
`Xcursor.theme_core`. Invalid values fall back safely. Missing Xft values leave
fontconfig preferences intact. An explicit `Xft.dpi` scales logical text sizes
relative to 96 DPI, independently of window geometry and raster scale. Hinting,
antialiasing, subpixel order, and LCD filters apply to the Cairo/Pango text path
used by text scene nodes, with separate font maps per text object.

Cursor themes are loaded through libXcursor, including inheritance and animated
images. XCB Render installs them; missing themes or unsupported Render fall back
to standard core cursor glyphs. `Xcursor.theme_core: false` selects core glyphs.
`XCURSOR_THEME` and a valid `XCURSOR_SIZE` override their resource counterparts.
The default cursor size is 24 pixels scaled by Xft DPI. Accepted sizes are 1–1024
and DPI values must be finite, positive, and at most 1000.

`RESOURCE_MANAGER` replacement or deletion reloads settings automatically, then
refreshes text nodes and cursor caches. File/environment changes need an explicit
`wlf_xresource_manager_reload(backend->resources)` call. Direct text API callers
can pass their backend through `wlf_text_options.backend`; NULL uses defaults.
The manager also exposes generic string queries and a `changed` signal.

Inspect effective settings or watch future server resource changes:

```sh
./build/examples/platform/xresource_info
./build/examples/platform/xresource_info --watch
```

For example, these lines can be merged using the usual `xrdb` workflow:

```xresources
Xft.dpi: 144
Xft.antialias: true
Xft.hinting: true
Xft.hintstyle: hintslight
Xft.rgba: rgb
Xft.lcdfilter: lcddefault
Xcursor.theme: Adwaita
Xcursor.size: 32
Xcursor.theme_core: true
```

### macOS

Install the Xcode command-line tools and dependencies from Homebrew:

```sh
xcode-select --install
brew install \
    meson ninja pkg-config \
    libpng libjpeg-turbo webp giflib pixman
```

Configure and compile the Metal backend:

```sh
meson setup build-macos --buildtype=debug
meson compile -C build-macos
meson test -C build-macos --print-errorlogs
```

The build supports Apple Silicon and uses the active Xcode toolchain selected
by `xcode-select`.

### Windows

Use a **Developer PowerShell for Visual Studio 2022** with the Desktop
development with C++ workload and a Windows SDK installed. Python and Git must
also be available. The commands below use vcpkg with the `x64-windows` triplet,
as does the Windows CI job.

Install Meson and Ninja:

```powershell
python -m pip install meson ninja
```

Bootstrap vcpkg and install wlframe's dependencies:

```powershell
$env:WLF_VCPKG_ROOT = "$PWD\vcpkg"
$env:VCPKG_TRIPLET = "x64-windows"

git clone https://github.com/microsoft/vcpkg.git $env:WLF_VCPKG_ROOT
& "$env:WLF_VCPKG_ROOT\bootstrap-vcpkg.bat" -disableMetrics
& "$env:WLF_VCPKG_ROOT\vcpkg.exe" install `
    libpng libjpeg-turbo libwebp giflib pixman pkgconf `
    --triplet $env:VCPKG_TRIPLET
```

Export the vcpkg paths for Meson:

```powershell
$installed = "$env:WLF_VCPKG_ROOT\installed\$env:VCPKG_TRIPLET"
$env:PKG_CONFIG = "$installed\tools\pkgconf\pkgconf.exe"
$env:PKG_CONFIG_PATH = "$installed\lib\pkgconfig;$installed\share\pkgconfig"
$env:INCLUDE = "$installed\include;$env:INCLUDE"
$env:LIB = "$installed\lib;$env:LIB"
$env:PATH = "$installed\tools\pkgconf;$installed\bin;$env:PATH"
```

Configure and compile a static library:

```powershell
meson setup build-windows `
    --buildtype=debug `
    --wrap-mode=nofallback `
    --default-library=static
meson compile -C build-windows
meson test -C build-windows --print-errorlogs
```

## Documentation

API documentation can be generated with Doxygen. Install Doxygen, Graphviz,
libxslt, XMLto, and libxml2, then enable the Meson option:

```sh
meson setup build-docs -Ddocumentation=enabled
meson compile -C build-docs
```

Open `build-docs/docs/doxygen/html/wlframe/index.html` after the build
completes.

## Contributing

Bug reports, experiments, documentation improvements, and focused patches are
welcome. See the [contributing guide](CONTRIBUTING.md) for the development
workflow, coding style, and commit-message guidelines.

## License

wlframe is distributed under the [MIT License](LICENSE).
