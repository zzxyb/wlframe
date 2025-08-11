wlframe reads these environment variables

# wlframe specific

## renderer

* **`WSM_RENDERER`**
  Forces the creation of a specified renderer backend on Linux.
  Available renderers:

  * `pixman` — CPU-based 2D rendering
  * `vulkan` — GPU-accelerated rendering

* **`WSM_RENDER_FORCE_DISCRETE_GPU`**
  When set to `1`, wlframe will attempt to use a **discrete GPU** (dedicated graphics card) for Vulkan rendering.
  This variable overrides the default GPU selection logic, which might otherwise choose an integrated GPU for power efficiency.

  > 💡 Typical use: force Vulkan to run on an NVIDIA/AMD GPU instead of an Intel integrated GPU.

* **`WSM_RENDER_FORCE_SOFTWARE`**
  When set to `1`, forces Vulkan to use a **software rasterizer** (CPU-based Vulkan implementation) instead of a physical GPU.
  This is mainly used for debugging or headless environments where no GPU is available.
  For software rendering, it is recommended to install the **`vulkan-swrast`** or **`lavapipe`** driver.

  > 💡 Typical use: test rendering behavior without requiring a GPU.


* **`WSM_RENDER_DEBUG`**
  When set to `1`, enables verbose renderer-level debug output and Vulkan validation layers for troubleshooting rendering issues.


