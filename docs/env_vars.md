wlframe reads these environment variables

# wlframe specific

## backend

* **`WLF_BACKEND`**
  Forces the creation of a specified backend.

## renderer

* **`WLF_RENDERER`**
  Forces the creation of a specified renderer backend on Linux.
  Available renderers:

  * `pixman` — CPU-based 2D rendering
  * `vulkan` — GPU-accelerated rendering via Vulkan
  * `gles`   — GPU-accelerated rendering via OpenGL ES

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


