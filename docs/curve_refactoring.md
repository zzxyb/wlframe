# wlf_animator_curve 重构：从枚举到继承式设计

## 概述

将 `wlf_animator_curve` 从基于枚举的设计重构为继承式设计，每个曲线类型现在都有自己的 struct。

## 主要变化

### 之前 (枚举方式)

```c
// 枚举定义所有曲线类型
enum wlf_animator_curve_type {
    WLF_ANIMATOR_CURVE_LINEAR,
    WLF_ANIMATOR_CURVE_IN_QUAD,
    WLF_ANIMATOR_CURVE_OUT_CUBIC,
    // ...
};

// 单一结构体，使用 type 字段区分
struct wlf_animator_curve {
    enum wlf_animator_curve_type type;
    float amplitude;
    float period;
    float overshoot;
};

// 初始化函数
void wlf_animator_curve_init(struct wlf_animator_curve *curve,
                              enum wlf_animator_curve_type type);

// 使用方式
struct wlf_animator_curve curve;
wlf_animator_curve_init(&curve, WLF_ANIMATOR_CURVE_OUT_CUBIC);
wlf_animator_set_curve(animator, WLF_ANIMATOR_CURVE_OUT_CUBIC);
```

### 之后 (继承式设计)

```c
// 基础曲线结构体（虚函数表）
struct wlf_animator_curve {
    float (*value_at)(const struct wlf_animator_curve *curve, float t);
    void (*destroy)(struct wlf_animator_curve *curve);
};

// 每个曲线类型有自己的结构体
struct wlf_animator_curve_linear {
    struct wlf_animator_curve base;
};

struct wlf_animator_curve_elastic {
    struct wlf_animator_curve base;
    float amplitude;
    float period;
};

struct wlf_animator_curve_back {
    struct wlf_animator_curve base;
    float overshoot;
};

// 工厂函数创建实例
struct wlf_animator_curve *wlf_animator_curve_linear_create(void);
struct wlf_animator_curve *wlf_animator_curve_out_cubic_create(void);
struct wlf_animator_curve *wlf_animator_curve_out_elastic_create(float amplitude, float period);
struct wlf_animator_curve *wlf_animator_curve_in_back_create(float overshoot);

// 使用方式
wlf_animator_set_curve(animator, wlf_animator_curve_out_cubic_create());
wlf_animator_set_curve(animator, wlf_animator_curve_out_elastic_create(1.5f, 0.4f));
```

## 优势

### 1. 更好的封装
- 每个曲线类型是独立的实体
- 只有需要参数的曲线才包含参数字段
- 减少内存浪费（之前所有曲线都包含 amplitude, period, overshoot）

### 2. 类型安全
- 编译时检查曲线类型
- 参数在创建时就明确指定
- 不会给不需要参数的曲线设置无意义的参数

### 3. 更灵活的扩展
- 添加新曲线类型只需：
  1. 定义新的结构体
  2. 实现 value_at 函数
  3. 添加工厂函数
- 不需要修改大的 switch 语句

### 4. 更清晰的 API
```c
// 之前：参数含义不明确
struct wlf_animator_curve curve;
wlf_animator_curve_init(&curve, WLF_ANIMATOR_CURVE_OUT_ELASTIC);
curve.amplitude = 1.5f;  // 必须手动设置参数
curve.period = 0.4f;

// 之后：参数在创建时指定，含义明确
struct wlf_animator_curve *curve =
    wlf_animator_curve_out_elastic_create(1.5f, 0.4f);
```

### 5. 内存管理更清晰
- 曲线对象动态分配
- 通过 `wlf_animator_curve_destroy()` 释放
- animator 拥有曲线的所有权

## 实现细节

### 多态实现

使用 C 语言的"继承"模式：
```c
// 基类作为第一个成员
struct wlf_animator_curve_quad {
    struct wlf_animator_curve base;  // 必须是第一个成员
};

// 类型转换
struct wlf_animator_curve_quad *quad = malloc(sizeof(*quad));
struct wlf_animator_curve *base = &quad->base;  // 向上转换
```

### 虚函数表
```c
static float in_quad_value_at(const struct wlf_animator_curve *curve, float t) {
    return ease_in_quad(clamp_t(t));
}

// 在工厂函数中设置
curve->base.value_at = in_quad_value_at;
curve->base.destroy = default_destroy;
```

### 统一接口
```c
// 多态调用
float wlf_animator_curve_value_at(const struct wlf_animator_curve *curve, float t) {
    if (!curve || !curve->value_at)
        return t;
    return curve->value_at(curve, t);
}
```

## 迁移指南

### 旧代码
```c
struct wlf_animator_curve curve;
wlf_animator_curve_init(&curve, WLF_ANIMATOR_CURVE_OUT_CUBIC);
wlf_animator_set_curve(animator, WLF_ANIMATOR_CURVE_OUT_CUBIC);
```

### 新代码
```c
wlf_animator_set_curve(animator, wlf_animator_curve_out_cubic_create());
```

### 带参数的曲线

**旧代码：**
```c
wlf_animator_set_curve(animator, WLF_ANIMATOR_CURVE_OUT_ELASTIC);
animator->curve.amplitude = 1.5f;
animator->curve.period = 0.4f;
```

**新代码：**
```c
wlf_animator_set_curve(animator,
    wlf_animator_curve_out_elastic_create(1.5f, 0.4f));
```

## 示例

查看以下示例了解新 API 的使用：
- `examples/animator_demo.c` - 完整的动画示例
- `examples/curve_inheritance_demo.c` - 曲线继承设计演示

## 技术细节

### 文件变更
- `include/wlf/animator/wlf_animator_curve.h` - 完全重写
- `animator/wlf_animator_curve.c` - 完全重写
- `include/wlf/animator/wlf_animator.h` - curve 字段改为指针
- `animator/wlf_animator.c` - 更新初始化和销毁逻辑
- `examples/animator_demo.c` - 更新所有示例
- `animator/README.md` - 更新文档

### 向后兼容性
此次重构不向后兼容。所有使用旧 API 的代码都需要更新。

### 性能影响
- 动态分配：每个曲线需要单独分配（通常很小，16-32字节）
- 虚函数调用：增加一次间接调用（可忽略的开销）
- 内存占用：减少（只为需要的参数分配空间）
