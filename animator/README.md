# wlframe Animator System

基于Qt Quick动画框架设计的动画系统，为wlframe提供流畅的属性动画支持。

## 设计理念

wlframe的动画系统参考了Qt Quick的动画架构，采用了以下核心概念：

### 1. 分层架构

- **基础层** (`wlf_animator`): 提供动画的基本功能,如时间管理、循环控制、回调等
- **缓动曲线层** (`wlf_animator_curve`): 提供丰富的缓动函数,控制动画的速度变化
  - 使用继承式设计,每个曲线类型都有自己的结构体
  - 通过工厂函数创建: `wlf_animator_curve_*_create()`
  - 支持多态调用: 统一的 `wlf_animator_curve_value_at()` 接口
- **属性动画层**: 为特定属性提供动画实现(opacity, x, y, width, height等)
- **变换动画层**: 提供变换相关的动画(rotation, scale等)

### 2. 核心特性

#### 缓动曲线 (Easing Curves)

采用继承式设计,每个曲线类型都有自己的struct和工厂函数。支持60+种缓动曲线:

- **基础**: Linear - `wlf_animator_curve_linear_create()`
- **幂函数**: Quad, Cubic, Quart, Quint (各有In/Out/InOut/OutIn变体)
  - 例如: `wlf_animator_curve_in_quad_create()`, `wlf_animator_curve_out_cubic_create()`
- **三角函数**: Sine (In/Out/InOut/OutIn)
  - 例如: `wlf_animator_curve_in_sine_create()`
- **指数函数**: Expo (In/Out/InOut/OutIn)
- **圆形**: Circ (In/Out/InOut/OutIn)
- **弹性**: Elastic (In/Out/InOut/OutIn) - 支持自定义振幅和周期参数
  - 例如: `wlf_animator_curve_out_elastic_create(amplitude, period)`
- **回弹**: Back (In/Out/InOut/OutIn) - 支持自定义overshoot参数
  - 例如: `wlf_animator_curve_in_back_create(overshoot)`
- **弹跳**: Bounce (In/Out/InOut/OutIn)


#### 动画控制

- **状态管理**: Stopped, Paused, Running
- **循环控制**: 支持单次、多次和无限循环
- **方向控制**: Forward, Backward, Alternate
- **时间控制**: 可设置持续时间，支持暂停/恢复

#### 事件系统 (Signal Events)

提供完整的生命周期事件信号：

```c
struct {
    struct wlf_signal started;   // 动画开始时触发
    struct wlf_signal stopped;   // 动画停止时触发
    struct wlf_signal finished;  // 动画完成时触发
    struct wlf_signal paused;    // 动画暂停时触发
    struct wlf_signal resumed;   // 动画恢复时触发
    struct wlf_signal updated;   // 每次更新时触发
} events;
```

## 架构设计

### 基础动画器 (Base Animator)

```c
struct wlf_animator {
    const struct wlf_animator_impl *impl;  // 实现接口
    struct wlf_animator_curve *curve;      // 缓动曲线 (指针)
    int64_t duration;                      // 持续时间(ms)
    int64_t current_time;                  // 当前时间
    enum wlf_animator_state state;         // 状态
    int loop_count;                        // 循环次数
    enum wlf_animator_direction direction; // 方向
    // ... 更多字段
};
```

### 实现接口 (Implementation Interface)

类似Qt的QAbstractAnimation，使用虚函数表模式：

```c
struct wlf_animator_impl {
    void (*start)(struct wlf_animator *);
    void (*stop)(struct wlf_animator *);
    void (*pause)(struct wlf_animator *);
    void (*resume)(struct wlf_animator *);
    void (*update)(struct wlf_animator *, int64_t dt);
    bool (*write_back)(struct wlf_animator *);
    // ...
};
```

### 属性动画器

每个属性动画器都继承自基础动画器，例如：

```c
struct wlf_opacity_animator {
    struct wlf_animator base;  // 基类
    float from;                // 起始值
    float to;                  // 结束值
    float current;             // 当前值
    float *target;             // 目标变量指针
};
```

## 使用方法

### 基本示例

```c
// 1. 创建动画器
float opacity = 0.0f;
struct wlf_opacity_animator *animator = wlf_opacity_animator_create(
    1000,    // 持续时间 1秒
    0.0f,    // 从0
    1.0f,    // 到1
    &opacity // 目标变量
);

struct wlf_animator *base = wlf_opacity_animator_get_base(animator);

// 2. 配置动画
wlf_animator_set_curve(base, wlf_animator_curve_out_cubic_create());
wlf_animator_set_loop_count(base, 1);

// 3. 添加事件监听器（可选）
struct wlf_listener started_listener = {
    .notify = my_started_callback,
    .user_data = user_data
};
struct wlf_listener finished_listener = {
    .notify = my_finished_callback,
    .user_data = user_data
};
wlf_signal_add(&base->events.started, &started_listener);
wlf_signal_add(&base->events.finished, &finished_listener);

// 4. 启动动画
wlf_animator_start(base);

// 5. 在主循环中更新（假设60fps）
while (wlf_animator_is_running(base)) {
    wlf_animator_update(base, 16);  // 16ms = 1/60秒
    // 使用 opacity 值进行渲染
}

// 6. 清理
wlf_opacity_animator_destroy(animator);
```

### 高级示例

#### 循环动画

```c
// 创建无限循环的旋转动画
float rotation = 0.0f;
struct wlf_rotation_animator *animator = wlf_rotation_animator_create(
    2000, 0.0f, 360.0f, &rotation
);
struct wlf_animator *base = wlf_rotation_animator_get_base(animator);

wlf_animator_set_loop_count(base, WLF_ANIMATOR_LOOP_INFINITE);
wlf_animator_set_curve(base, wlf_animator_curve_linear_create());
wlf_animator_start(base);
```

#### 往复动画

```c
// 创建往复的缩放动画
float scale_x = 1.0f, scale_y = 1.0f;
struct wlf_scale_animator *animator = wlf_scale_animator_create(
    1000, 1.0f, 1.5f, &scale_x, &scale_y
);
struct wlf_animator *base = wlf_scale_animator_get_base(animator);

wlf_animator_set_direction(base, WLF_ANIMATOR_DIRECTION_ALTERNATE);
wlf_animator_set_loop_count(base, 5);  // 来回5次
wlf_animator_start(base);
```

#### 弹性效果

```c
// 带弹性效果的位置动画
float x = 0.0f;
struct wlf_x_animator *animator = wlf_x_animator_create(
    1500, 0.0f, 800.0f, &x
);
struct wlf_animator *base = wlf_x_animator_get_base(animator);

wlf_animator_set_curve(base, wlf_animator_curve_out_elastic_create(1.2f, 0.4f));
wlf_animator_start(base);
```

## 可用的动画器类型

| 动画器类型 | 说明 | 创建函数 |
|-----------|------|---------|
| OpacityAnimator | 透明度动画 | `wlf_opacity_animator_create()` |
| XAnimator | X坐标动画 | `wlf_x_animator_create()` |
| YAnimator | Y坐标动画 | `wlf_y_animator_create()` |
| WidthAnimator | 宽度动画 | `wlf_width_animator_create()` |
| HeightAnimator | 高度动画 | `wlf_height_animator_create()` |
| RotationAnimator | 旋转动画 | `wlf_rotation_animator_create()` |
| ScaleAnimator | 缩放动画 | `wlf_scale_animator_create()` |
| UniformAnimator | 通用浮点数动画 | `wlf_uniform_animator_create()` |

## 与Qt Quick的对应关系

| Qt Quick | wlframe | 说明 |
|----------|---------|------|
| QAbstractAnimation | wlf_animator | 抽象基类 |
| QPropertyAnimation | wlf_*_animator | 属性动画 |
| QEasingCurve | wlf_animator_curve | 缓动曲线 |
| NumberAnimation | wlf_uniform_animator | 数值动画 |
| OpacityAnimator | wlf_opacity_animator | 透明度动画 |
| XAnimator | wlf_x_animator | X动画 |
| YAnimator | wlf_y_animator | Y动画 |
| RotationAnimator | wlf_rotation_animator | 旋转动画 |
| ScaleAnimator | wlf_scale_animator | 缩放动画 |

## 最佳实践

1. **使用合适的缓动曲线**:
   - UI进入: `OUT_CUBIC`, `OUT_QUAD`
   - UI退出: `IN_CUBIC`, `IN_QUAD`
   - 强调效果: `OUT_BACK`, `OUT_ELASTIC`
   - 自然移动: `IN_OUT_SINE`, `IN_OUT_CUBIC`

2. **持续时间建议**:
   - 小型UI变化: 150-300ms
   - 中型UI变化: 300-500ms
   - 大型UI变化: 500-800ms
   - 页面转场: 300-600ms

3. **性能优化**:
   - 批量更新动画器（在同一帧内更新所有动画）
   - 对不可见元素停止动画
   - 使用适当的帧率（通常60fps即可）

4. **内存管理**:
   - 及时销毁不需要的动画器
   - 注意目标指针的生命周期

## 示例程序

运行 `examples/animator_demo` 查看完整示例：

```bash
meson setup build
meson compile -C build
./build/examples/animator_demo
```

## 扩展

要创建自定义动画器：

1. 定义结构体，包含 `struct wlf_animator base`
2. 实现 `wlf_animator_impl` 接口
3. 实现create/destroy函数
4. 在update函数中使用 `wlf_animator_get_progress()` 获取缓动后的进度

参考现有的动画器实现作为模板。

## 未来计划

- [ ] 动画组 (AnimationGroup) - 并行和顺序执行
- [ ] 路径动画 (PathAnimation) - 沿路径移动
- [ ] 颜色动画 (ColorAnimation) - RGB/HSV插值
- [ ] 状态机集成 (State Machine)
- [ ] 动画缓存和复用系统
- [ ] GPU加速的动画计算

## 许可证

MIT License - 与wlframe项目相同
