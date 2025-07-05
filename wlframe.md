# WLFrame - 跨平台UI库架构设计

## 1. 整体架构概览

### 核心特性

- **跨平台支持**: Wayland、MacOS、X11等
- **多渲染后端**: Vulkan、Pixman、OpenGL ES等  
- **性能优化**: 离屏渲染、批量渲染、FBO管理
- **灵活扩展**: 钩子机制、自定义渲染、区域控制
- **类型安全**: 区分叶子节点和容器节点的树状结构
- **高级特效**: 完全可控的渲染管道，支持后处理和自定义合成
- **渲染目标感知**: 钩子函数能够区分渲染目标（窗口/FBO），支持适配性渲染

### 分层架构图

```
┌─────────────────────────────────────────────────────────────────┐
│                    Application Layer                           │
├─────────────────────────────────────────────────────────────────┤
│                    WLFrame Core API                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │
│  │ wlf_window  │  │  wlf_item   │  │     Event System        │ │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│                   Rendering Abstraction                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │
│  │   Vulkan    │  │   OpenGL    │  │        Pixman           │ │
│  │   Backend   │  │   Backend   │  │        Backend          │ │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│                  Platform Abstraction                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │
│  │   Wayland   │  │    MacOS    │  │         X11             │ │
│  │   Backend   │  │   Backend   │  │        Backend          │ │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│                     OS/Hardware                               │
└─────────────────────────────────────────────────────────────────┘
```

## 2. 核心组件设计

### 2.1 Platform Backend 架构

```c
// wlf_platform.h
typedef struct wlf_platform wlf_platform_t;
typedef struct wlf_platform_interface wlf_platform_interface_t;

typedef enum {
    WLF_PLATFORM_WAYLAND,
    WLF_PLATFORM_MACOS,
    WLF_PLATFORM_X11,
    WLF_PLATFORM_WIN32
} wlf_platform_type_t;

struct wlf_platform_interface {
    // 窗口管理
    int (*create_window)(wlf_platform_t *platform, wlf_window_t *window);
    void (*destroy_window)(wlf_platform_t *platform, wlf_window_t *window);
    void (*show_window)(wlf_platform_t *platform, wlf_window_t *window);
    void (*hide_window)(wlf_platform_t *platform, wlf_window_t *window);
    
    // 事件处理
    int (*poll_events)(wlf_platform_t *platform);
    int (*wait_events)(wlf_platform_t *platform);
    
    // 渲染表面
    void* (*create_surface)(wlf_platform_t *platform, wlf_window_t *window);
    void (*destroy_surface)(wlf_platform_t *platform, void *surface);
    
    // 输入处理
    void (*set_cursor)(wlf_platform_t *platform, wlf_window_t *window, int cursor_type);
    void (*grab_pointer)(wlf_platform_t *platform, wlf_window_t *window);
    void (*ungrab_pointer)(wlf_platform_t *platform, wlf_window_t *window);
};

struct wlf_platform {
    wlf_platform_type_t type;
    wlf_platform_interface_t *interface;
    void *platform_data;  // 平台特定数据
};

// 平台后端工厂函数
wlf_platform_t* wlf_platform_create_wayland(void);
wlf_platform_t* wlf_platform_create_macos(void);
wlf_platform_t* wlf_platform_create_x11(void);
```

### 2.2 Renderer Backend 架构

```c
// wlf_renderer.h
typedef struct wlf_renderer wlf_renderer_t;
typedef struct wlf_renderer_interface wlf_renderer_interface_t;

typedef enum {
    WLF_RENDERER_VULKAN,
    WLF_RENDERER_OPENGL_ES,
    WLF_RENDERER_PIXMAN
} wlf_renderer_type_t;

typedef struct {
    uint32_t width, height;
    uint32_t format;  // 像素格式
    void *data;       // 像素数据
} wlf_framebuffer_t;

struct wlf_renderer_interface {
    // 初始化和清理
    int (*init)(wlf_renderer_t *renderer, void *surface);
    void (*cleanup)(wlf_renderer_t *renderer);
    
    // 帧缓冲管理
    wlf_framebuffer_t* (*create_framebuffer)(wlf_renderer_t *renderer, 
                                            uint32_t width, uint32_t height);
    void (*destroy_framebuffer)(wlf_renderer_t *renderer, wlf_framebuffer_t *fb);
    
    // 渲染操作
    void (*begin_frame)(wlf_renderer_t *renderer);
    void (*end_frame)(wlf_renderer_t *renderer);
    void (*present)(wlf_renderer_t *renderer);
    
    // 绘制原语
    void (*clear)(wlf_renderer_t *renderer, uint32_t color);
    void (*draw_rect)(wlf_renderer_t *renderer, int x, int y, int w, int h, uint32_t color);
    void (*blit_framebuffer)(wlf_renderer_t *renderer, wlf_framebuffer_t *src, 
                           int src_x, int src_y, int dst_x, int dst_y, int w, int h);
    
    // 剪裁和变换
    void (*set_clip_rect)(wlf_renderer_t *renderer, int x, int y, int w, int h);
    void (*reset_clip_rect)(wlf_renderer_t *renderer);
};

struct wlf_renderer {
    wlf_renderer_type_t type;
    wlf_renderer_interface_t *interface;
    void *renderer_data;
    wlf_framebuffer_t *current_target;
};
```

### 2.3 Window 组件架构

```c
// wlf_window.h
typedef struct wlf_window wlf_window_t;
typedef struct wlf_menu wlf_menu_t;

typedef enum {
    WLF_WINDOW_TOPLEVEL,
    WLF_WINDOW_POPUP,
    WLF_WINDOW_DIALOG
} wlf_window_type_t;

typedef struct {
    void (*on_close)(wlf_window_t *window);
    void (*on_resize)(wlf_window_t *window, int width, int height);
    void (*on_focus)(wlf_window_t *window, bool focused);
    void (*on_key)(wlf_window_t *window, int key, int action, int mods);
    void (*on_mouse)(wlf_window_t *window, int button, int action, int mods);
    void (*on_mouse_move)(wlf_window_t *window, double x, double y);
} wlf_window_callbacks_t;

struct wlf_window {
    // 基本属性
    uint32_t id;
    wlf_window_type_t type;
    char *title;
    int x, y, width, height;
    bool visible;
    bool decorated;
    
    // 层次结构
    wlf_window_t *parent;
    wlf_item_tree_t *root_item;  // 根UI元素 (必须是容器类型)
    
    // 菜单系统
    wlf_menu_t *menu_bar;
    
    // 平台和渲染
    wlf_platform_t *platform;
    wlf_renderer_t *renderer;
    void *platform_window;  // 平台特定窗口句柄
    void *render_surface;   // 渲染表面
    
    // 回调函数
    wlf_window_callbacks_t callbacks;
    void *user_data;
};

// 窗口API
wlf_window_t* wlf_window_create(wlf_platform_t *platform, 
                              wlf_renderer_t *renderer,
                              const char *title,
                              int width, int height);
void wlf_window_destroy(wlf_window_t *window);
void wlf_window_show(wlf_window_t *window);
void wlf_window_hide(wlf_window_t *window);
void wlf_window_set_title(wlf_window_t *window, const char *title);
void wlf_window_set_size(wlf_window_t *window, int width, int height);
void wlf_window_set_callbacks(wlf_window_t *window, wlf_window_callbacks_t *callbacks);
```

### 2.4 Item 组件架构

```c
// wlf_item.h
typedef struct wlf_item wlf_item_t;
typedef struct wlf_item_tree wlf_item_tree_t;
typedef struct wlf_rect wlf_rect_t;
typedef struct wlf_region wlf_region_t;

struct wlf_rect {
    int x, y, width, height;
};

typedef struct {
    int count;
    wlf_rect_t *rects;
} wlf_region_t;

typedef enum {
    WLF_ITEM_TYPE_LEAF,     // 叶子节点 (wlf_item)
    WLF_ITEM_TYPE_TREE      // 容器节点 (wlf_item_tree)
} wlf_item_type_t;

// 渲染目标类型
typedef enum {
    WLF_RENDER_TARGET_WINDOW,       // 直接渲染到窗口
    WLF_RENDER_TARGET_FBO           // 渲染到离屏帧缓冲区
} wlf_render_target_type_t;

// 渲染上下文信息 (传递给on_paint钩子)
typedef struct {
    wlf_render_target_type_t target_type;   // 渲染目标类型
    // 实际只需要fbo就够了，因为window应该被设计为很容易获取
    union {
        struct {
            wlf_window_t *window;           // 目标窗口 (当target_type=WINDOW时)
        } window;
        struct {
            wlf_framebuffer_t *fbo;         // 目标FBO (当target_type=FBO时)
        } fbo;
    } target;
} wlf_render_context_t;

// 基础Item钩子 (用于叶子节点)
typedef struct {
    // 渲染钩子 (增强版本，包含渲染上下文信息)
    void (*on_paint)(wlf_item_t *item, wlf_renderer_t *renderer, 
                     wlf_rect_t *damage, wlf_render_context_t *context);
    
    // 布局钩子
    void (*on_layout)(wlf_item_t *item, wlf_rect_t *available);
    
    // 输入事件钩子
    bool (*on_mouse_event)(wlf_item_t *item, int button, int action, int x, int y);
    bool (*on_key_event)(wlf_item_t *item, int key, int action, int mods);
    bool (*on_mouse_move)(wlf_item_t *item, int x, int y);
    
    // 生命周期钩子
    void (*on_parent_added)(wlf_item_t *item, wlf_item_tree_t *parent);
    void (*on_parent_removed)(wlf_item_t *item, wlf_item_tree_t *parent);
} wlf_item_hooks_t;

// 容器Item钩子 (用于树节点)
typedef struct {
    // 继承基础钩子
    wlf_item_hooks_t base;
    
    // 批量渲染钩子 (增强版本，包含渲染上下文)
    void (*on_children_begin_render)(wlf_item_tree_t *tree, wlf_renderer_t *renderer, wlf_render_context_t *context);
    void (*on_children_end_render)(wlf_item_tree_t *tree, wlf_renderer_t *renderer, wlf_render_context_t *context);
    
    // 子元素渲染控制钩子 (增强版本)
    void (*on_child_paint)(wlf_item_tree_t *tree, wlf_item_t *child, wlf_renderer_t *renderer, 
                          wlf_rect_t *damage, wlf_render_context_t *context);
    bool (*should_render_to_fbo)(wlf_item_tree_t *tree, wlf_item_t *child, wlf_render_context_t *context);
    
    // 自定义合成钩子 (增强版本)
    void (*on_composite_children)(wlf_item_tree_t *tree, wlf_renderer_t *renderer, 
                                 wlf_framebuffer_t *children_fbo, wlf_render_context_t *context);
    
    // 子元素管理钩子
    void (*on_child_added)(wlf_item_tree_t *tree, wlf_item_t *child);
    void (*on_child_removed)(wlf_item_tree_t *tree, wlf_item_t *child);
} wlf_item_tree_hooks_t;

// 基础 wlf_item 结构体 (叶子节点)
struct wlf_item {
    // 基本属性
    uint32_t id;
    wlf_item_type_t type;           // 节点类型
    wlf_rect_t geometry;            // 相对于父元素的位置和大小
    wlf_rect_t content_rect;        // 内容区域（除去边距）
    
    // 可见性和交互
    bool visible;
    bool enabled;
    float opacity;                  // 透明度 0.0-1.0
    
    // 区域控制
    wlf_region_t *transparent_region;  // 透明区域
    wlf_region_t *input_region;        // 输入区域
    wlf_region_t *damage_region;       // 损坏区域
    
    // 层次结构关系
    wlf_item_tree_t *parent;        // 父容器
    int z_order;                    // Z轴顺序
    
    // 离屏渲染 (单个Item)
    wlf_framebuffer_t *offscreen_buffer;
    bool use_offscreen;
    bool buffer_dirty;
    
    // 事件和渲染钩子
    wlf_item_hooks_t hooks;
    void *user_data;
    
    // 所属窗口
    wlf_window_t *window;
};

// 容器 wlf_item_tree 结构体 (可包含子元素)
struct wlf_item_tree {
    // 继承基础Item的所有属性
    wlf_item_t base;
    
    // 子元素管理
    wlf_item_t **children;              // 子元素数组 (可以是item或item_tree)
    size_t children_count;
    size_t children_capacity;
    
    // 批量离屏渲染
    wlf_framebuffer_t *children_fbo;    // 子元素共享FBO
    bool use_children_fbo;              // 是否启用子元素批量离屏渲染
    bool children_fbo_dirty;            // 子元素FBO是否需要重绘
    wlf_rect_t children_bounds;         // 所有子元素的边界盒
    
    // 渲染控制 (新增)
    bool force_children_to_fbo;         // 强制所有子元素渲染到FBO，而不是直接到窗口
    bool custom_composite;              // 是否使用自定义合成方式
    
    // 扩展钩子
    wlf_item_tree_hooks_t tree_hooks;
};

// Item API (叶子节点)
wlf_item_t* wlf_item_create(wlf_window_t *window);
void wlf_item_destroy(wlf_item_t *item);
void wlf_item_set_geometry(wlf_item_t *item, wlf_rect_t *rect);
void wlf_item_set_visible(wlf_item_t *item, bool visible);
void wlf_item_set_opacity(wlf_item_t *item, float opacity);
void wlf_item_set_hooks(wlf_item_t *item, wlf_item_hooks_t *hooks);

// Item Tree API (容器节点)
wlf_item_tree_t* wlf_item_tree_create(wlf_window_t *window);
void wlf_item_tree_destroy(wlf_item_tree_t *tree);
void wlf_item_tree_add_child(wlf_item_tree_t *parent, wlf_item_t *child);
void wlf_item_tree_remove_child(wlf_item_tree_t *parent, wlf_item_t *child);
void wlf_item_tree_set_hooks(wlf_item_tree_t *tree, wlf_item_tree_hooks_t *hooks);

// 类型转换和检查
bool wlf_item_is_tree(wlf_item_t *item);
wlf_item_tree_t* wlf_item_to_tree(wlf_item_t *item);
wlf_item_t* wlf_item_tree_to_item(wlf_item_tree_t *tree);

// 区域管理
wlf_region_t* wlf_region_create(void);
void wlf_region_destroy(wlf_region_t *region);
void wlf_region_add_rect(wlf_region_t *region, wlf_rect_t *rect);
bool wlf_region_contains_point(wlf_region_t *region, int x, int y);

// 离屏渲染控制
void wlf_item_enable_offscreen(wlf_item_t *item, bool enable);
void wlf_item_mark_dirty(wlf_item_t *item, wlf_rect_t *damage);

// 批量离屏渲染控制 (仅用于item_tree)
void wlf_item_tree_enable_children_fbo(wlf_item_tree_t *tree, bool enable);
void wlf_item_tree_mark_children_dirty(wlf_item_tree_t *tree);
void wlf_item_tree_update_children_bounds(wlf_item_tree_t *tree);

// 渲染控制 (新增)
void wlf_item_tree_set_force_children_to_fbo(wlf_item_tree_t *tree, bool force);
void wlf_item_tree_set_custom_composite(wlf_item_tree_t *tree, bool custom);
```

## 3. 工作流程图

### 3.1 渲染上下文管理

```c
// 渲染上下文辅助函数
static void wlf_render_context_init_window(wlf_render_context_t *ctx, wlf_window_t *window, 
                                          wlf_rect_t *viewport, float opacity) {
    ctx->target_type = WLF_RENDER_TARGET_WINDOW;
    ctx->target.window.window = window;
    ctx->viewport = *viewport;
    ctx->opacity_factor = opacity;
    ctx->allow_caching = true;
    ctx->requires_alpha_blending = (opacity < 1.0f);
    
    // 初始化单位变换矩阵
    memset(ctx->transform_matrix, 0, sizeof(ctx->transform_matrix));
    ctx->transform_matrix[0] = ctx->transform_matrix[5] = ctx->transform_matrix[10] = ctx->transform_matrix[15] = 1.0f;
}

static void wlf_render_context_init_fbo(wlf_render_context_t *ctx, wlf_framebuffer_t *fbo,
                                       wlf_rect_t *viewport, float opacity, 
                                       bool is_batch, wlf_item_tree_t *container) {
    ctx->target_type = WLF_RENDER_TARGET_FBO;
    ctx->target.fbo.fbo = fbo;
    ctx->target.fbo.is_children_batch = is_batch;
    ctx->target.fbo.batch_container = container;
    ctx->viewport = *viewport;
    ctx->opacity_factor = opacity;
    ctx->allow_caching = !is_batch;  // 批量渲染时通常不缓存
    ctx->requires_alpha_blending = true;
    
    // 初始化单位变换矩阵
    memset(ctx->transform_matrix, 0, sizeof(ctx->transform_matrix));
    ctx->transform_matrix[0] = ctx->transform_matrix[5] = ctx->transform_matrix[10] = ctx->transform_matrix[15] = 1.0f;
}
```

### 3.2 窗口创建流程

```
Start
  ↓
创建 wlf_window 对象
  ↓
调用 platform->create_window()
  ↓
创建渲染表面 platform->create_surface()
  ↓
初始化渲染器 renderer->init()
  ↓
创建根 wlf_item
  ↓
注册事件回调
  ↓
显示窗口 platform->show_window()
  ↓
End
```

### 3.3 渲染流程图

```
Begin Frame
  ↓
遍历所有可见 Item (深度优先)
  ↓
检查是否需要重绘 (damage_region)
  ↓
[需要重绘] → 开始离屏渲染 (如果启用)
  ↓
调用 on_paint 钩子
  ↓
渲染子 Items
  ↓
结束离屏渲染 → 合成到父缓冲区
  ↓
应用透明度和混合
  ↓
继续下一个 Item
  ↓
Present Frame
```

### 3.4 事件处理流程

```
Platform Event
  ↓
转换为 WLF 事件格式
  ↓
确定目标窗口
  ↓
[鼠标事件] → 命中测试 (考虑input_region)
  ↓
找到目标 Item
  ↓
调用相应事件钩子
  ↓
[事件被处理] → 停止传播
  ↓
[事件未处理] → 传播给父 Item
  ↓
重复直到根 Item 或事件被处理
```

## 4. 关键实现细节

### 4.1 平台抽象实现示例 (Wayland)

```c
// platform_wayland.c
typedef struct {
    struct wl_display *display;
    struct wl_compositor *compositor;
    struct wl_shell *shell;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
    
    // 窗口映射表
    struct wl_surface **surfaces;
    wlf_window_t **windows;
    size_t window_count;
} wayland_platform_data_t;

static int wayland_create_window(wlf_platform_t *platform, wlf_window_t *window) {
    wayland_platform_data_t *data = platform->platform_data;
    
    struct wl_surface *surface = wl_compositor_create_surface(data->compositor);
    if (!surface) return -1;
    
    struct wl_shell_surface *shell_surface = wl_shell_get_shell_surface(data->shell, surface);
    wl_shell_surface_set_toplevel(shell_surface);
    
    // 存储平台特定数据
    window->platform_window = shell_surface;
    
    // 注册到窗口映射表
    // ... 实现细节
    
    return 0;
}

static void wayland_surface_frame_callback(void *data, struct wl_callback *callback, uint32_t time) {
    wlf_window_t *window = data;
    
    // 触发重绘
    wlf_window_request_redraw(window);
    
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener frame_listener = {
    wayland_surface_frame_callback
};
```

### 4.2 Vulkan 渲染器实现示例

```c
// renderer_vulkan.c
typedef struct {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkQueue graphics_queue;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    
    VkSwapchainKHR swapchain;
    VkImage *swapchain_images;
    VkImageView *swapchain_image_views;
    VkFramebuffer *framebuffers;
    
    VkRenderPass render_pass;
    VkPipeline graphics_pipeline;
} vulkan_renderer_data_t;

static void vulkan_begin_frame(wlf_renderer_t *renderer) {
    vulkan_renderer_data_t *data = renderer->renderer_data;
    
    // 获取下一个交换链图像
    uint32_t image_index;
    vkAcquireNextImageKHR(data->device, data->swapchain, UINT64_MAX, 
                         VK_NULL_HANDLE, VK_NULL_HANDLE, &image_index);
    
    // 开始命令缓冲区录制
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(data->command_buffer, &begin_info);
    
    // 开始渲染通道
    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = data->render_pass,
        .framebuffer = data->framebuffers[image_index],
        // ... 其他参数
    };
    vkCmdBeginRenderPass(data->command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}
```

### 4.3 Item 离屏渲染实现

```c
// wlf_item.c
void wlf_item_render_recursive(wlf_item_t *item, wlf_renderer_t *renderer, wlf_rect_t *clip) {
    if (!item->visible || item->opacity <= 0.0f) return;
    
    // 计算实际绘制区域
    wlf_rect_t item_rect = item->geometry;
    wlf_rect_t draw_rect;
    if (!wlf_rect_intersect(&item_rect, clip, &draw_rect)) return;
    
    // 检查是否使用离屏渲染
    if (item->use_offscreen) {
        if (item->buffer_dirty || !item->offscreen_buffer) {
            // 重新渲染到离屏缓冲区
            wlf_item_render_offscreen(item, renderer);
        }
        
        // 将离屏缓冲区合成到当前目标
        renderer->interface->blit_framebuffer(renderer, item->offscreen_buffer,
                                            0, 0, item_rect.x, item_rect.y,
                                            item_rect.width, item_rect.height);
    } else {
        // 直接渲染
        if (item->hooks.on_paint) {
            renderer->interface->set_clip_rect(renderer, draw_rect.x, draw_rect.y,
                                             draw_rect.width, draw_rect.height);
            item->hooks.on_paint(item, renderer, &draw_rect);
        }
        
        // 渲染子元素
        for (size_t i = 0; i < item->children_count; i++) {
            wlf_item_render_recursive(item->children[i], renderer, &draw_rect);
        }
    }
}

static void wlf_item_render_offscreen(wlf_item_t *item, wlf_renderer_t *renderer) {
    // 创建或更新离屏缓冲区
    if (!item->offscreen_buffer) {
        item->offscreen_buffer = renderer->interface->create_framebuffer(
            renderer, item->geometry.width, item->geometry.height);
    }
    
    // 设置离屏缓冲区为渲染目标
    wlf_framebuffer_t *old_target = renderer->current_target;
    renderer->current_target = item->offscreen_buffer;
    
    // 清除缓冲区
    renderer->interface->clear(renderer, 0x00000000);  // 透明
    
    // 渲染内容
    if (item->hooks.on_paint) {
        wlf_rect_t full_rect = {0, 0, item->geometry.width, item->geometry.height};
        item->hooks.on_paint(item, renderer, &full_rect);
    }
    
    // 渲染子元素
    for (size_t i = 0; i < item->children_count; i++) {
        wlf_rect_t child_clip = item->children[i]->geometry;
        wlf_item_render_recursive(item->children[i], renderer, &child_clip);
    }
    
    // 恢复原渲染目标
    renderer->current_target = old_target;
    item->buffer_dirty = false;
}
```

### 4.4 新架构的批量离屏渲染实现

```c
// wlf_item_tree.c

// 更新所有子元素的边界盒
void wlf_item_tree_update_children_bounds(wlf_item_tree_t *tree) {
    if (tree->children_count == 0) {
        tree->children_bounds = (wlf_rect_t){0, 0, 0, 0};
        return;
    }
    
    wlf_rect_t bounds = tree->children[0]->geometry;
    
    // 计算所有子元素的包围盒
    for (size_t i = 1; i < tree->children_count; i++) {
        wlf_item_t *child = tree->children[i];
        if (!child->visible) continue;
        
        wlf_rect_t child_rect = child->geometry;
        
        // 扩展边界
        int right = bounds.x + bounds.width;
        int bottom = bounds.y + bounds.height;
        int child_right = child_rect.x + child_rect.width;
        int child_bottom = child_rect.y + child_rect.height;
        
        bounds.x = MIN(bounds.x, child_rect.x);
        bounds.y = MIN(bounds.y, child_rect.y);
        bounds.width = MAX(right, child_right) - bounds.x;
        bounds.height = MAX(bottom, child_bottom) - bounds.y;
    }
    
    tree->children_bounds = bounds;
}

// 启用/禁用子元素批量离屏渲染
void wlf_item_tree_enable_children_fbo(wlf_item_tree_t *tree, bool enable) {
    if (tree->use_children_fbo == enable) return;
    
    tree->use_children_fbo = enable;
    
    if (enable) {
        // 更新子元素边界
        wlf_item_tree_update_children_bounds(tree);
        tree->children_fbo_dirty = true;
    } else {
        // 清理FBO资源
        if (tree->children_fbo) {
            wlf_renderer_t *renderer = tree->base.window->renderer;
            renderer->interface->destroy_framebuffer(renderer, tree->children_fbo);
            tree->children_fbo = NULL;
        }
    }
}

// 渲染控制函数 (新增)
void wlf_item_tree_set_force_children_to_fbo(wlf_item_tree_t *tree, bool force) {
    tree->force_children_to_fbo = force;
    if (force && !tree->use_children_fbo) {
        // 如果强制渲染到FBO，自动启用children_fbo
        wlf_item_tree_enable_children_fbo(tree, true);
    }
}

void wlf_item_tree_set_custom_composite(wlf_item_tree_t *tree, bool custom) {
    tree->custom_composite = custom;
}

// 检查子元素是否应该渲染到FBO
static bool wlf_item_tree_should_render_child_to_fbo(wlf_item_tree_t *tree, wlf_item_t *child) {
    // 如果有自定义钩子，优先使用
    if (tree->tree_hooks.should_render_to_fbo) {
        // 构造渲染上下文
        wlf_render_context_t context;
        wlf_render_context_init_window(&context, tree->base.window);
        return tree->tree_hooks.should_render_to_fbo(tree, child, &context);
    }
    
    // 默认策略：如果强制或者启用了children_fbo就渲染到FBO
    return tree->force_children_to_fbo || tree->use_children_fbo;
}

// 渲染单个子元素 (支持FBO重定向)
static void wlf_item_tree_render_child(wlf_item_tree_t *tree, wlf_item_t *child, 
                                      wlf_renderer_t *renderer, wlf_rect_t *clip,
                                      bool render_to_fbo) {
    if (!child->visible) return;
    
    // 如果有自定义子元素渲染钩子，使用它
    if (tree->tree_hooks.on_child_paint) {
        // 构造渲染上下文
        wlf_render_context_t context;
        if (render_to_fbo && tree->children_fbo) {
            wlf_render_context_init_fbo(&context, tree->children_fbo);
        } else {
            wlf_render_context_init_window(&context, tree->base.window);
        }
        tree->tree_hooks.on_child_paint(tree, child, renderer, clip, &context);
        return;
    }
    
    // 如果需要渲染到FBO，但当前不是FBO目标，则跳过
    // (这种情况下子元素会在批量FBO渲染时被处理)
    if (render_to_fbo && renderer->current_target != tree->children_fbo) {
        return;
    }
    
    // 调用标准的Item渲染流程
    wlf_item_render_recursive(child, renderer, clip);
}

// 添加子元素
void wlf_item_tree_add_child(wlf_item_tree_t *parent, wlf_item_t *child) {
    // 扩容检查
    if (parent->children_count >= parent->children_capacity) {
        size_t new_capacity = parent->children_capacity ? parent->children_capacity * 2 : 4;
        parent->children = realloc(parent->children, new_capacity * sizeof(wlf_item_t*));
        parent->children_capacity = new_capacity;
    }
    
    // 添加子元素
    parent->children[parent->children_count++] = child;
    child->parent = parent;
    
    // 调用钩子
    if (parent->tree_hooks.on_child_added) {
        parent->tree_hooks.on_child_added(parent, child);
    }
    
    if (child->hooks.on_added) {
        child->hooks.on_added(child, parent);
    }
    
    // 标记需要重新计算边界和重绘
    if (parent->use_children_fbo) {
        wlf_item_tree_mark_children_dirty(parent);
    }
}

// 移除子元素
void wlf_item_tree_remove_child(wlf_item_tree_t *parent, wlf_item_t *child) {
    // 查找子元素
    for (size_t i = 0; i < parent->children_count; i++) {
        if (parent->children[i] == child) {
            // 调用钩子
            if (child->hooks.on_removed) {
                child->hooks.on_removed(child, parent);
            }
            
            if (parent->tree_hooks.on_child_removed) {
                parent->tree_hooks.on_child_removed(parent, child);
            }
            
            // 移除元素
            memmove(&parent->children[i], &parent->children[i + 1], 
                   (parent->children_count - i - 1) * sizeof(wlf_item_t*));
            parent->children_count--;
            child->parent = NULL;
            
            // 标记需要重新计算边界和重绘
            if (parent->use_children_fbo) {
                wlf_item_tree_mark_children_dirty(parent);
            }
            break;
        }
    }
}

// 类型检查和转换
bool wlf_item_is_tree(wlf_item_t *item) {
    return item->type == WLF_ITEM_TYPE_TREE;
}

wlf_item_tree_t* wlf_item_to_tree(wlf_item_t *item) {
    if (wlf_item_is_tree(item)) {
        return (wlf_item_tree_t*)item;
    }
    return NULL;
}

wlf_item_t* wlf_item_tree_to_item(wlf_item_tree_t *tree) {
    return &tree->base;
}

// 渲染所有子元素到共享FBO
static void wlf_item_tree_render_children_to_fbo(wlf_item_tree_t *tree, wlf_renderer_t *renderer) {
    if (!tree->use_children_fbo || tree->children_count == 0) return;
    
    // 更新边界盒
    wlf_item_tree_update_children_bounds(tree);
    
    // 创建或调整FBO大小
    wlf_rect_t bounds = tree->children_bounds;
    if (!tree->children_fbo || 
        tree->children_fbo->width != bounds.width || 
        tree->children_fbo->height != bounds.height) {
        
        if (tree->children_fbo) {
            renderer->interface->destroy_framebuffer(renderer, tree->children_fbo);
        }
        
        tree->children_fbo = renderer->interface->create_framebuffer(
            renderer, bounds.width, bounds.height);
    }
    
    // 保存当前渲染目标
    wlf_framebuffer_t *old_target = renderer->current_target;
    renderer->current_target = tree->children_fbo;
    
    // 清除FBO
    renderer->interface->clear(renderer, 0x00000000);  // 透明背景
    
    // 调用开始渲染钩子
    if (tree->tree_hooks.on_children_begin_render) {
        wlf_render_context_t context;
        wlf_render_context_init_fbo(&context, tree->children_fbo);
        tree->tree_hooks.on_children_begin_render(tree, renderer, &context);
    }
    
    // 渲染所有子元素到FBO
    for (size_t i = 0; i < tree->children_count; i++) {
        wlf_item_t *child = tree->children[i];
        if (!child->visible) continue;
        
        // 计算子元素在FBO中的相对位置
        wlf_rect_t child_clip = {
            child->geometry.x - bounds.x,
            child->geometry.y - bounds.y,
            child->geometry.width,
            child->geometry.height
        };
        
        // 临时调整子元素几何位置
        wlf_rect_t original_geometry = child->geometry;
        child->geometry = child_clip;
        
        // 使用新的子元素渲染方法
        wlf_item_tree_render_child(tree, child, renderer, &child_clip, true);
        
        // 恢复原始几何位置
        child->geometry = original_geometry;
    }
    
    // 调用结束渲染钩子
    if (tree->tree_hooks.on_children_end_render) {
        wlf_render_context_t context;
        wlf_render_context_init_fbo(&context, tree->children_fbo);
        tree->tree_hooks.on_children_end_render(tree, renderer, &context);
    }
    
    // 恢复原渲染目标
    renderer->current_target = old_target;
    tree->children_fbo_dirty = false;
}

// 通用的Item渲染函数 (支持叶子节点和树节点)
void wlf_item_render_recursive(wlf_item_t *item, wlf_renderer_t *renderer, wlf_rect_t *clip) {
    if (!item->visible || item->opacity <= 0.0f) return;
    
    // 计算实际绘制区域
    wlf_rect_t item_rect = item->geometry;
    wlf_rect_t draw_rect;
    if (!wlf_rect_intersect(&item_rect, clip, &draw_rect)) return;
    
    // 检查是否使用单个Item离屏渲染
    if (item->use_offscreen) {
        if (item->buffer_dirty || !item->offscreen_buffer) {
            wlf_item_render_offscreen(item, renderer);
        }
        
        // 将离屏缓冲区合成到当前目标
        renderer->interface->blit_framebuffer(renderer, item->offscreen_buffer,
                                            0, 0, item_rect.x, item_rect.y,
                                            item_rect.width, item_rect.height);
        return;
    }
    
    // 渲染自身内容
    if (item->hooks.on_paint) {
        renderer->interface->set_clip_rect(renderer, draw_rect.x, draw_rect.y,
                                         draw_rect.width, draw_rect.height);
        // 构造渲染上下文
        wlf_render_context_t context;
        wlf_render_context_init_window(&context, item->window);
        item->hooks.on_paint(item, renderer, &draw_rect, &context);
    }
    
    // 如果是树节点，处理子元素
    if (wlf_item_is_tree(item)) {
        wlf_item_tree_t *tree = wlf_item_to_tree(item);
        
        if (tree->use_children_fbo && tree->children_count > 0) {
            // 批量FBO渲染
            if (tree->children_fbo_dirty || !tree->children_fbo) {
                wlf_item_tree_render_children_to_fbo(tree, renderer);
            }
            
            // 检查是否使用自定义合成
            if (tree->custom_composite && tree->tree_hooks.on_composite_children) {
                // 使用自定义合成方法
                wlf_render_context_t context;
                wlf_render_context_init_window(&context, tree->base.window);
                tree->tree_hooks.on_composite_children(tree, renderer, tree->children_fbo, &context);
            } else {
                // 默认合成：将子元素FBO一次性合成到当前目标
                if (tree->children_fbo) {
                    wlf_rect_t bounds = tree->children_bounds;
                    renderer->interface->blit_framebuffer(renderer, tree->children_fbo,
                        0, 0,  // 源FBO的位置
                        item_rect.x + bounds.x, item_rect.y + bounds.y,  // 目标位置
                        bounds.width, bounds.height);
                }
            }
        } else {
            // 传统方式：逐个渲染子元素
            for (size_t i = 0; i < tree->children_count; i++) {
                wlf_item_t *child = tree->children[i];
                
                // 检查是否应该渲染到FBO
                bool should_render_to_fbo = wlf_item_tree_should_render_child_to_fbo(tree, child);
                
                if (should_render_to_fbo) {
                    // 如果需要渲染到FBO但FBO未启用，跳过直接渲染
                    continue;
                } else {
                    // 直接渲染到当前目标
                    wlf_item_tree_render_child(tree, child, renderer, &draw_rect, false);
                }
            }
        }
    }
    // 叶子节点无需处理子元素
}

// 单个Item离屏渲染 (适用于叶子节点和树节点)
static void wlf_item_render_offscreen(wlf_item_t *item, wlf_renderer_t *renderer) {
    // 创建或更新离屏缓冲区
    if (!item->offscreen_buffer) {
        item->offscreen_buffer = renderer->interface->create_framebuffer(
            renderer, item->geometry.width, item->geometry.height);
    }
    
    // 设置离屏缓冲区为渲染目标
    wlf_framebuffer_t *old_target = renderer->current_target;
    renderer->current_target = item->offscreen_buffer;
    
    // 清除缓冲区
    renderer->interface->clear(renderer, 0x00000000);  // 透明
    
    // 渲染内容
    if (item->hooks.on_paint) {
        wlf_rect_t full_rect = {0, 0, item->geometry.width, item->geometry.height};
        // 构造渲染上下文
        wlf_render_context_t context;
        wlf_render_context_init_fbo(&context, item->offscreen_buffer);
        item->hooks.on_paint(item, renderer, &full_rect, &context);
    }
    
    // 如果是树节点，也需要渲染子元素
    if (wlf_item_is_tree(item)) {
        wlf_item_tree_t *tree = wlf_item_to_tree(item);
        for (size_t i = 0; i < tree->children_count; i++) {
            wlf_rect_t child_clip = tree->children[i]->geometry;
            wlf_item_render_recursive(tree->children[i], renderer, &child_clip);
        }
    }
    
    // 恢复原渲染目标
    renderer->current_target = old_target;
    item->buffer_dirty = false;
}
```

### 4.5 渲染上下文使用指南

### 6.1 渲染上下文概述

`wlf_render_context_t` 为所有钩子函数提供了渲染目标的详细信息，使得渲染逻辑可以根据不同的渲染场景进行优化。

### 6.2 渲染上下文字段说明

```c
typedef struct {
    wlf_render_target_type_t target_type;    // 渲染目标类型
    union {
        wlf_window_t *window;                // 窗口目标
        wlf_framebuffer_t *framebuffer;      // FBO目标
    } target;
    
    int target_width;                        // 目标宽度
    int target_height;                       // 目标高度
    bool allows_hardware_acceleration;       // 是否支持硬件加速
    bool allows_post_processing;             // 是否支持后处理
    bool allow_caching;                     // 是否允许缓存渲染结果
    bool requires_alpha_blending;           // 是否需要Alpha混合
} wlf_render_context_t;
```

### 6.3 根据渲染上下文优化渲染策略

#### 6.3.1 基本用法

```c
void my_item_on_paint(wlf_item_t *item, wlf_renderer_t *renderer, 
                     wlf_rect_t *damage, wlf_render_context_t *context) {
    switch (context->target_type) {
        case WLF_RENDER_TARGET_WINDOW:
            // 直接渲染到窗口 - 优化显示性能
            printf("直接渲染到窗口 %dx%d\n", 
                   context->target_width, context->target_height);
            render_optimized_for_display(item, renderer, damage);
            break;
            
        case WLF_RENDER_TARGET_FBO:
            // 渲染到FBO - 可以使用更高质量的渲染
            printf("渲染到FBO %dx%d，支持后处理: %s\n", 
                   context->target_width, context->target_height,
                   context->allows_post_processing ? "是" : "否");
            render_high_quality_for_fbo(item, renderer, damage);
            break;
    }
}
}
```

#### 6.3.2 硬件加速优化

```c
void advanced_item_on_paint(wlf_item_t *item, wlf_renderer_t *renderer, 
                           wlf_rect_t *damage, wlf_render_context_t *context) {
    if (context->allows_hardware_acceleration) {
        // 使用GPU加速渲染
        if (context->target_type == WLF_RENDER_TARGET_FBO) {
            // FBO支持复杂的GPU特效
            render_with_gpu_effects(item, renderer);
        } else {
            // 窗口渲染使用优化的GPU路径
            render_with_gpu_optimization(item, renderer);
        }
    } else {
        // 软件渲染fallback
        render_with_software(item, renderer);
    }
}
```

#### 6.3.3 缓存策略优化

```c
void cached_item_on_paint(wlf_item_t *item, wlf_renderer_t *renderer, 
                         wlf_rect_t *damage, wlf_render_context_t *context) {
    my_item_t *my_item = (my_item_t*)item;
    
    if (context->allow_caching && !my_item->content_changed) {
        // 可以使用缓存，跳过重绘
        if (my_item->cached_texture) {
            blit_cached_texture(renderer, my_item->cached_texture);
            return;
        }
    }
    
    // 重新渲染并更新缓存
    render_item_content(item, renderer);
    
    if (context->allow_caching) {
        update_item_cache(my_item, renderer);
    }
}
```

#### 6.3.4 容器节点的高级控制

```c
void container_on_composite_children(wlf_item_tree_t *tree, wlf_renderer_t *renderer, 
                                   wlf_framebuffer_t *children_fbo, 
                                   wlf_render_context_t *context) {
    if (context->target_type == WLF_RENDER_TARGET_WINDOW) {
        // 直接合成到窗口 - 优化显示延迟
        if (context->allows_hardware_acceleration) {
            gpu_composite_to_window(renderer, children_fbo);
        } else {
            software_composite_to_window(renderer, children_fbo);
        }
    } else {
        // 合成到其他FBO - 可以应用更复杂的效果
        apply_container_effects(renderer, children_fbo);
        
        if (context->allows_post_processing) {
            apply_post_processing_effects(renderer, children_fbo);
        }
        
        composite_to_target(renderer, children_fbo, context->target.framebuffer);
    }
}
```

### 6.4 最佳实践

1. **始终检查渲染目标类型**: 不同的目标类型有不同的性能特征和限制
2. **利用硬件加速**: 当`allows_hardware_acceleration`为true时，优先使用GPU渲染路径
3. **合理使用缓存**: 在允许缓存时，缓存复杂的渲染结果以提高性能
4. **优化Alpha混合**: 根据`requires_alpha_blending`标志调整渲染策略
5. **后处理优化**: 只在支持后处理且有需要时才应用复杂的效果

### 6.5 兼容性考虑

为了保持向后兼容性，可以提供不带上下文参数的旧版本钩子函数：

```c
// 新版本（推荐）
void item_on_paint_v2(wlf_item_t *item, wlf_renderer_t *renderer, 
                     wlf_rect_t *damage, wlf_render_context_t *context);

// 旧版本（兼容性）
void item_on_paint_v1(wlf_item_t *item, wlf_renderer_t *renderer, 
                     wlf_rect_t *damage);

// 自动适配包装器
void item_on_paint_wrapper(wlf_item_t *item, wlf_renderer_t *renderer, 
                          wlf_rect_t *damage, wlf_render_context_t *context) {
    if (item->hooks.on_paint_v2) {
        item->hooks.on_paint_v2(item, renderer, damage, context);
    } else if (item->hooks.on_paint_v1) {
        item->hooks.on_paint_v1(item, renderer, damage);
    }
}
```

## 5. 使用示例

### 5.1 创建简单窗口

```c
#include "wlframe.h"

// 自定义按钮组件
typedef struct {
    wlf_item_t base;
    char *text;
    uint32_t bg_color;
    bool pressed;
} button_item_t;

void button_on_paint(wlf_item_t *item, wlf_renderer_t *renderer, wlf_rect_t *damage, wlf_render_context_t *context) {
    button_item_t *button = (button_item_t*)item;
    
    // 根据渲染上下文优化
    if (context->target_type == WLF_RENDER_TARGET_FBO) {
        // 渲染到FBO时可以使用更高质量
        printf("按钮渲染到FBO\n");
    }
    
    // 绘制背景
    uint32_t color = button->pressed ? 0xFF666666 : button->bg_color;
    renderer->interface->draw_rect(renderer, 0, 0, 
                                 item->geometry.width, item->geometry.height, color);
    
    // 这里可以添加文本渲染...
}

bool button_on_mouse_event(wlf_item_t *item, int button, int action, int x, int y) {
    button_item_t *btn = (button_item_t*)item;
    
    if (button == 1) {  // 左键
        if (action == 1) {  // 按下
            btn->pressed = true;
            wlf_item_mark_dirty(item, NULL);  // 标记需要重绘
        } else if (action == 0) {  // 释放
            btn->pressed = false;
            wlf_item_mark_dirty(item, NULL);
            
            // 触发点击事件
            printf("Button clicked!\n");
        }
        return true;  // 事件已处理
    }
    return false;
}

int main() {
    // 初始化平台和渲染器
    wlf_platform_t *platform = wlf_platform_create_wayland();
    wlf_renderer_t *renderer = wlf_renderer_create_vulkan();
    
    // 创建窗口
    wlf_window_t *window = wlf_window_create(platform, renderer, "Test Window", 800, 600);
    
    // 创建按钮
    button_item_t *button = malloc(sizeof(button_item_t));
    wlf_item_init(&button->base, window);
    button->text = strdup("Click Me");
    button->bg_color = 0xFF4444FF;
    button->pressed = false;
    
    // 设置按钮属性
    wlf_rect_t button_rect = {100, 100, 200, 50};
    wlf_item_set_geometry(&button->base, &button_rect);
    
    // 设置钩子函数
    wlf_item_hooks_t hooks = {
        .on_paint = button_on_paint,
        .on_mouse_event = button_on_mouse_event
    };
    wlf_item_set_hooks(&button->base, &hooks);
    
    // 添加到窗口
    wlf_item_add_child(window->root_item, &button->base);
    
    // 显示窗口
    wlf_window_show(window);
    
    // 主循环
    while (1) {
        platform->interface->poll_events(platform);
        
        // 渲染
        renderer->interface->begin_frame(renderer);
        wlf_window_render(window);
        renderer->interface->end_frame(renderer);
        renderer->interface->present(renderer);
    }
    
    return 0;
}
```

## 7. 总结

这个重新设计的架构提供了：

### **核心优势**

1. **清晰的类型分离**: 
   - **wlf_item**: 轻量级叶子节点，适合按钮、文本、图片等简单组件
   - **wlf_item_tree**: 功能完整的容器节点，支持子元素管理和高级渲染控制

2. **内存效率**: 
   - 叶子节点无需children数组和相关管理代码，显著减少内存占用
   - 只有需要容器功能的节点才使用更复杂的结构

3. **多层次渲染控制**:
   - **单元素离屏渲染**: 每个Item可以有自己的framebuffer
   - **批量子元素离屏渲染**: 容器可将所有子元素渲染到共享FBO
   - **细粒度渲染控制**: 容器可以控制每个子元素的渲染方式和目标
   - **自定义合成**: 支持对子元素FBO进行后处理和自定义合成

4. **灵活的钩子系统**: 
   - **基础钩子**: 所有节点通用（渲染、事件、生命周期）
   - **容器钩子**: 专门用于容器节点的高级功能
   - **子元素渲染钩子**: `on_child_paint` 允许容器完全控制子元素渲染
   - **渲染决策钩子**: `should_render_to_fbo` 动态决定渲染目标
   - **自定义合成钩子**: `on_composite_children` 实现复杂的后处理效果
   - **渲染上下文感知**: 所有钩子都接收 `wlf_render_context_t` 参数，可根据渲染目标类型（窗口/FBO）优化渲染策略

5. **跨平台支持**: 统一的API，底层适配不同平台

6. **精确的区域控制**: 透明区域、输入区域的精确管理

### **新增的高级渲染控制功能**

- **强制FBO渲染**: `force_children_to_fbo` 确保所有子元素渲染到FBO而非直接到窗口
- **自定义合成模式**: `custom_composite` 启用完全自定义的FBO合成方式
- **子元素渲染拦截**: 容器可以完全接管子元素的渲染流程
- **后处理管道**: 支持模糊、阴影、颜色调整等复杂特效
- **渲染上下文区分**: 钩子函数可根据渲染目标类型（窗口/FBO）自适应渲染逻辑，优化不同场景下的渲染性能和质量

### **工作流程优势**

1. **渲染目标控制**: 
   ```
   容器决定 → 子元素渲染到FBO → 应用后处理 → 合成到窗口
   ```

2. **性能优化**: 
   - 减少渲染状态切换
   - 批量处理多个子元素
   - 智能缓存和脏区域检测

3. **特效支持**: 
   - 所有子元素在FBO中统一处理
   - 支持复杂的滤镜和后处理效果
   - 可实现毛玻璃、阴影、发光等高级效果

### **使用场景**

- **叶子节点**: 按钮、标签、图片、输入框等简单UI元素
- **普通容器**: 面板、列表、对话框等基础容器组件
- **高级容器**: 需要特效的面板、毛玻璃效果、复杂合成的组件

### **架构特点**

- **类型安全**: 编译时和运行时都可以区分叶子节点和容器节点
- **渐进增强**: 从简单容器到高级渲染控制的平滑升级路径
- **组合模式**: 支持树状结构，容器可以包含其他容器或叶子节点
- **渲染管道**: 完全可定制的渲染流程控制
- **上下文感知**: 钩子接口能区分渲染目标，支持针对不同渲染场景的优化策略

该架构具有优秀的可扩展性、性能和渲染控制能力，非常适合构建现代化的跨平台应用程序。通过分层的钩子系统，开发者可以从简单的UI组件逐步构建复杂的视觉效果，而无需修改底层架构。

## 8. 更新日志

### v2.0 - 渲染上下文增强 (2025-07-04)

#### 新增功能
- **渲染上下文支持**: 所有钩子函数现在都接收 `wlf_render_context_t` 参数
- **目标类型感知**: 钩子可以区分渲染到窗口或FBO（简化设计，专注于主要使用场景）
- **性能优化标志**: 支持硬件加速、后处理、缓存等优化策略的检测
- **智能渲染策略**: 根据渲染上下文自动优化渲染路径

#### 更新的接口
- `on_paint`: 新增 `wlf_render_context_t *context` 参数
- `on_children_begin_render`: 新增 `wlf_render_context_t *context` 参数  
- `on_children_end_render`: 新增 `wlf_render_context_t *context` 参数
- `on_child_paint`: 新增 `wlf_render_context_t *context` 参数
- `should_render_to_fbo`: 新增 `wlf_render_context_t *context` 参数
- `on_composite_children`: 新增 `wlf_render_context_t *context` 参数

#### 新增辅助函数
- `wlf_render_context_init_window()`: 初始化窗口渲染上下文
- `wlf_render_context_init_fbo()`: 初始化FBO渲染上下文

#### 兼容性
- 提供了向后兼容的包装器支持旧版本钩子
- 渐进式升级路径，可以逐步迁移到新接口

#### 示例更新
- 更新了所有示例代码以使用新的渲染上下文接口
- 展示了根据渲染目标类型进行优化的最佳实践
- 添加了硬件加速和缓存策略的示例

该版本大幅增强了渲染系统的灵活性和性能优化能力，为复杂的UI特效和跨平台渲染优化提供了强大的基础。

---

*文档版本: 2.0*  
*最后更新: 2025年7月4日*