# 字体后端系统

## 概述

wlframe字体模块现在支持插件化的后端系统，可以跨平台访问系统字体信息。目前支持：

- **macOS**: 使用Core Text框架
- **Linux**: 使用FontConfig库

## 特性

### 后端系统
- 自动检测并激活可用的平台后端
- 统一的API接口
- 运行时插件注册
- 平台特定的优化实现

### 字体发现
- 枚举所有系统字体
- 按模式搜索字体
- 获取字体详细信息（家族、样式、权重等）
- 支持的语言和字符集查询

### 系统字体访问
- 获取默认系统字体
- 获取等宽字体
- 按条件查找字体（家族、样式、权重）
- 语言特定的字体推荐

## API使用

### 初始化
```c
#include "wlf/font/wlf_font.h"
#include "wlf/font/wlf_font_backend.h"

// 初始化字体系统（包括后端）
if (!wlf_font_init()) {
    // 处理错误
}

// 获取活动后端信息
const struct wlf_font_backend *backend = wlf_font_backend_get_active();
if (backend) {
    printf("使用后端: %s\n", backend->name);
}
```

### 枚举系统字体
```c
bool font_callback(const struct wlf_font_info *info, void *user_data) {
    printf("字体: %s\n", info->family_name);
    printf("样式: %s\n", info->style_name);
    printf("文件: %s\n", info->file_path);
    return true; // 继续枚举
}

// 枚举所有系统字体
wlf_font_enumerate_system_fonts(font_callback, NULL);

// 搜索特定字体
wlf_font_find_system_fonts("monospace", font_callback, NULL);
```

### 加载系统字体
```c
// 加载默认系统字体
struct wlf_font *default_font = wlf_font_load_system_default(NULL, 14, NULL);

// 加载等宽字体
struct wlf_font *mono_font = wlf_font_load_system_monospace(12, NULL);

// 按条件加载字体
struct wlf_font *serif_font = wlf_font_load_system_font(
    "serif",
    WLF_FONT_STYLE_NORMAL,
    WLF_FONT_WEIGHT_NORMAL,
    16,
    NULL
);

// 使用完毕后销毁
wlf_font_destroy(default_font);
wlf_font_destroy(mono_font);
wlf_font_destroy(serif_font);
```

### 获取字体路径
```c
// 获取默认字体路径
char *default_path = wlf_font_get_system_default_font(NULL);
char *chinese_path = wlf_font_get_system_default_font("zh");

// 获取等宽字体路径
char *mono_path = wlf_font_get_system_monospace_font();

// 按条件获取字体路径
char *bold_path = wlf_font_get_system_font_path(
    "sans-serif",
    WLF_FONT_STYLE_NORMAL,
    WLF_FONT_WEIGHT_BOLD
);

// 记得释放内存
free(default_path);
free(chinese_path);
free(mono_path);
free(bold_path);
```

### 清理
```c
// 清理字体系统
wlf_font_cleanup();
```

## 字体信息结构

```c
struct wlf_font_info {
    char *family_name;                  // 字体家族名称
    char *style_name;                   // 字体样式名称
    char *postscript_name;              // PostScript名称
    char *file_path;                    // 字体文件路径
    enum wlf_font_weight weight;        // 字体权重
    enum wlf_font_style style;          // 字体样式
    enum wlf_font_width width;          // 字体宽度
    bool is_monospace;                  // 是否等宽
    bool is_scalable;                   // 是否可缩放
    char **languages;                   // 支持的语言
    char **character_sets;              // 支持的字符集
};
```

## 后端实现

### macOS后端 (Core Text)
- 使用Core Text和Core Foundation框架
- 直接访问系统字体数据库
- 支持所有macOS字体特性
- 自动处理字体缓存和优化

### Linux后端 (FontConfig)
- 使用FontConfig库
- 支持系统字体配置
- 兼容大多数Linux发行版
- 支持字体回退和替换

## 构建要求

### macOS
- Core Text框架（系统自带）
- Core Foundation框架（系统自带）

### Linux
- FontConfig开发库
  ```bash
  # Ubuntu/Debian
  sudo apt-get install libfontconfig1-dev

  # CentOS/RHEL
  sudo yum install fontconfig-devel

  # Arch Linux
  sudo pacman -S fontconfig
  ```

## 示例程序

查看 `examples/font_backend_example.c` 了解完整的使用示例。

编译并运行示例：
```bash
meson compile -C build
./build/examples/font_backend_example
```

## 错误处理

所有API函数都会返回适当的错误指示：
- 返回`NULL`的函数表示操作失败
- 返回`bool`的函数中`false`表示失败
- 使用`wlf_log`系统记录详细错误信息

## 内存管理

- 字体信息结构需要使用`wlf_font_info_free()`释放
- 字体路径字符串需要使用`free()`释放
- 字体对象需要使用`wlf_font_destroy()`销毁

## 线程安全

当前实现不是线程安全的。如果需要在多线程环境中使用，请添加适当的同步机制。
