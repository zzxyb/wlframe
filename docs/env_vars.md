wlframe reads these environment variables

# wlframe specific

## backend

* **`WLF_BACKEND`**
  Selects `wayland` or `x11` on Linux. Without an override, wlframe tries
  `WAYLAND_DISPLAY` first and falls back to `DISPLAY`. An explicit selection
  fails if that backend cannot connect.

## renderer

* **`WLF_RENDERER`**
  Forces the creation of a specified renderer backend on Linux.
  Available renderers:

  * `pixman` — CPU-based 2D rendering
  * `vulkan` — GPU-accelerated rendering via Vulkan (Wayland only)
  * `gles`   — GPU-accelerated rendering via OpenGL ES

  Automatic X11 selection tries GLES followed by Pixman.

* **`WLF_RENDER_FORCE_DISCRETE_GPU`**
  When set to `1`, wlframe will attempt to use a **discrete GPU** (dedicated graphics card) for Vulkan rendering.
  This variable overrides the default GPU selection logic, which might otherwise choose an integrated GPU for power efficiency.

  > 💡 Typical use: force Vulkan to run on an NVIDIA/AMD GPU instead of an Intel integrated GPU.

* **`WLF_RENDER_FORCE_SOFTWARE`**
  When set to `1`, forces the renderer to use a **software rasterizer** (CPU-based implementation) instead of a physical GPU.
  This is mainly used for debugging or headless environments where no GPU is available.

  * **Vulkan**: selects a CPU-type physical device. Install **`vulkan-swrast`** or **`lavapipe`**.
  * **GLES**: sets `LIBGL_ALWAYS_SOFTWARE=1` before EGL initialisation, directing Mesa to use a software driver (llvmpipe / softpipe).

  > 💡 Typical use: test rendering behavior without requiring a GPU.

* **`WLF_RENDER_DEBUG`**
  When set to `1`, enables verbose renderer-level debug output and Vulkan validation layers for troubleshooting rendering issues.

## scene damage

* **`WLF_SCENE_DEBUG_DAMAGE`**
  Controls scene damage visualization:

  * `none` — normal damage-based rendering (default)
  * `rerender` — redraw and submit the whole window whenever a frame is damaged
  * `highlight` — overlay damaged pixels in red and fade them out over 250 ms


## X11 cursor resources

* **`XCURSOR_THEME`**
  A nonempty value overrides `Xcursor.theme` for the X11 backend.

* **`XCURSOR_SIZE`**
  A decimal integer from 1 to 1024 overrides `Xcursor.size`. Invalid values are
  ignored. Without a resource or valid override, the size is 24 scaled by
  `Xft.dpi / 96`.

* **`XENVIRONMENT`**
  An optional Xresource file merged after `RESOURCE_MANAGER` or the user's
  fallback resource file. This follows xcb-util-xrm's default database rules.
  File/environment changes take effect after an explicit resource-manager reload.
