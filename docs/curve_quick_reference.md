# wlf_animator_curve 快速参考

## 创建曲线

### 简单曲线 (无参数)

```c
// Linear
struct wlf_animator_curve *curve = wlf_animator_curve_linear_create();

// Quadratic
wlf_animator_curve_in_quad_create();
wlf_animator_curve_out_quad_create();
wlf_animator_curve_in_out_quad_create();
wlf_animator_curve_out_in_quad_create();

// Cubic
wlf_animator_curve_in_cubic_create();
wlf_animator_curve_out_cubic_create();
wlf_animator_curve_in_out_cubic_create();
wlf_animator_curve_out_in_cubic_create();

// Quartic
wlf_animator_curve_in_quart_create();
wlf_animator_curve_out_quart_create();
wlf_animator_curve_in_out_quart_create();
wlf_animator_curve_out_in_quart_create();

// Quintic
wlf_animator_curve_in_quint_create();
wlf_animator_curve_out_quint_create();
wlf_animator_curve_in_out_quint_create();
wlf_animator_curve_out_in_quint_create();

// Sine
wlf_animator_curve_in_sine_create();
wlf_animator_curve_out_sine_create();
wlf_animator_curve_in_out_sine_create();
wlf_animator_curve_out_in_sine_create();

// Exponential
wlf_animator_curve_in_expo_create();
wlf_animator_curve_out_expo_create();
wlf_animator_curve_in_out_expo_create();
wlf_animator_curve_out_in_expo_create();

// Circular
wlf_animator_curve_in_circ_create();
wlf_animator_curve_out_circ_create();
wlf_animator_curve_in_out_circ_create();
wlf_animator_curve_out_in_circ_create();

// Bounce
wlf_animator_curve_in_bounce_create();
wlf_animator_curve_out_bounce_create();
wlf_animator_curve_in_out_bounce_create();
wlf_animator_curve_out_in_bounce_create();
```

### 参数化曲线

#### Elastic (弹性)
```c
// 参数: amplitude (振幅), period (周期)
// 默认值: amplitude=1.0, period=0.3
struct wlf_animator_curve *curve =
    wlf_animator_curve_out_elastic_create(1.0f, 0.3f);

wlf_animator_curve_in_elastic_create(amplitude, period);
wlf_animator_curve_out_elastic_create(amplitude, period);
wlf_animator_curve_in_out_elastic_create(amplitude, period);
wlf_animator_curve_out_in_elastic_create(amplitude, period);
```

#### Back (回弹)
```c
// 参数: overshoot (超调量)
// 默认值: overshoot=1.70158
struct wlf_animator_curve *curve =
    wlf_animator_curve_out_back_create(1.70158f);

wlf_animator_curve_in_back_create(overshoot);
wlf_animator_curve_out_back_create(overshoot);
wlf_animator_curve_in_out_back_create(overshoot);
wlf_animator_curve_out_in_back_create(overshoot);
```

## 使用曲线

### 基本用法
```c
// 1. 创建曲线
struct wlf_animator_curve *curve = wlf_animator_curve_out_cubic_create();

// 2. 计算缓动值
float t = 0.5f;  // 时间在 [0.0, 1.0] 范围内
float eased = wlf_animator_curve_value_at(curve, t);

// 3. 销毁曲线
wlf_animator_curve_destroy(curve);
```

### 与 animator 配合使用
```c
// 创建 animator
struct wlf_opacity_animator *animator =
    wlf_opacity_animator_create(1000, 0.0f, 1.0f, &opacity);
struct wlf_animator *base = wlf_opacity_animator_get_base(animator);

// 设置曲线 (animator 会拥有曲线的所有权)
wlf_animator_set_curve(base, wlf_animator_curve_out_cubic_create());

// 启动动画
wlf_animator_start(base);

// animator 销毁时会自动销毁曲线
wlf_opacity_animator_destroy(animator);
```

## 常用曲线推荐

### UI 动画
```c
// 淡入淡出
wlf_animator_curve_in_out_cubic_create()    // 平滑
wlf_animator_curve_in_out_quart_create()    // 更柔和

// 滑入
wlf_animator_curve_out_cubic_create()       // 快速开始，慢速结束
wlf_animator_curve_out_quart_create()       // 更明显的减速

// 滑出
wlf_animator_curve_in_cubic_create()        // 慢速开始，快速结束
```

### 特效动画
```c
// 弹性效果
wlf_animator_curve_out_elastic_create(1.0f, 0.3f)  // 轻微弹性
wlf_animator_curve_out_elastic_create(1.5f, 0.5f)  // 明显弹性

// 回弹效果
wlf_animator_curve_out_back_create(1.70158f)       // 标准回弹
wlf_animator_curve_out_back_create(2.5f)           // 更大回弹

// 弹跳效果
wlf_animator_curve_out_bounce_create()             // 自然弹跳
```

### 循环动画
```c
// 平滑循环
wlf_animator_curve_linear_create()                 // 恒速
wlf_animator_curve_in_out_sine_create()            // 正弦波形
```

## 曲线类型说明

- **In**: 缓慢开始，加速结束
- **Out**: 快速开始，减速结束
- **InOut**: 缓慢开始和结束，中间加速
- **OutIn**: 快速开始和结束，中间减速

## 内存管理

```c
// ✅ 正确：让 animator 管理
wlf_animator_set_curve(animator, wlf_animator_curve_out_cubic_create());

// ❌ 错误：重复释放
struct wlf_animator_curve *curve = wlf_animator_curve_out_cubic_create();
wlf_animator_set_curve(animator, curve);
wlf_animator_curve_destroy(curve);  // 不要这样做！animator 会自动释放

// ✅ 正确：独立使用
struct wlf_animator_curve *curve = wlf_animator_curve_out_cubic_create();
float value = wlf_animator_curve_value_at(curve, 0.5f);
wlf_animator_curve_destroy(curve);  // 需要手动释放
```
