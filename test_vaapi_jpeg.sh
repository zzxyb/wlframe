#!/bin/bash
# 测试 VA-API JPEG DMA-BUF 功能的脚本

set -e

echo "=== VA-API JPEG DMA-BUF 测试 ==="
echo

# 检查 VA-API 支持
echo "1. 检查 VA-API 支持..."
if command -v vainfo &> /dev/null; then
    echo "✓ vainfo 已安装"
    vainfo | grep -i jpeg || echo "⚠ 警告: 未检测到 JPEG 支持"
else
    echo "⚠ 警告: vainfo 未安装"
fi
echo

# 编译项目
echo "2. 编译项目..."
if [ ! -d "build" ]; then
    echo "初始化 meson 构建目录..."
    meson setup build
fi

echo "编译..."
meson compile -C build
echo "✓ 编译完成"
echo

# 检查可执行文件
DEMO_BIN="build/examples/vaapi_jpeg_demo"
if [ ! -f "$DEMO_BIN" ]; then
    echo "✗ 错误: 未找到 $DEMO_BIN"
    echo "请确保编译成功"
    exit 1
fi
echo "✓ 找到可执行文件: $DEMO_BIN"
echo

# 准备测试图片
echo "3. 准备测试图片..."
TEST_DIR="build/jpeg_test"
mkdir -p "$TEST_DIR"

# 如果没有测试图片，创建一个简单的 JPEG
TEST_INPUT="$TEST_DIR/test_input.jpg"
if [ ! -f "$TEST_INPUT" ]; then
    echo "创建测试图片..."
    # 使用 ImageMagick 创建测试图片（如果可用）
    if command -v convert &> /dev/null; then
        convert -size 640x480 xc:blue -pointsize 40 -fill white \
            -gravity center -annotate +0+0 "VA-API JPEG Test" \
            "$TEST_INPUT"
        echo "✓ 测试图片已创建: $TEST_INPUT"
    else
        echo "⚠ 警告: ImageMagick 未安装，无法自动创建测试图片"
        echo "   请手动放置一个 JPEG 文件到: $TEST_INPUT"
        exit 1
    fi
else
    echo "✓ 使用现有测试图片: $TEST_INPUT"
fi
echo

# 运行测试
echo "4. 运行 VA-API JPEG 测试..."
TEST_OUTPUT="$TEST_DIR/test_output.jpg"

echo "命令: $DEMO_BIN -i $TEST_INPUT -o $TEST_OUTPUT -q 90"
if "$DEMO_BIN" -i "$TEST_INPUT" -o "$TEST_OUTPUT" -q 90; then
    echo "✓ 测试成功"

    # 检查输出文件
    if [ -f "$TEST_OUTPUT" ]; then
        echo "✓ 输出文件已生成: $TEST_OUTPUT"

        # 显示文件大小
        INPUT_SIZE=$(stat -c%s "$TEST_INPUT" 2>/dev/null || stat -f%z "$TEST_INPUT")
        OUTPUT_SIZE=$(stat -c%s "$TEST_OUTPUT" 2>/dev/null || stat -f%z "$TEST_OUTPUT")
        echo "  输入大小: $INPUT_SIZE 字节"
        echo "  输出大小: $OUTPUT_SIZE 字节"

        # 使用 identify 显示图片信息（如果可用）
        if command -v identify &> /dev/null; then
            echo
            echo "输入图片信息:"
            identify "$TEST_INPUT"
            echo
            echo "输出图片信息:"
            identify "$TEST_OUTPUT"
        fi
    else
        echo "✗ 错误: 输出文件未生成"
        exit 1
    fi
else
    echo "✗ 测试失败"
    exit 1
fi
echo

# 测试不同质量参数
echo "5. 测试不同质量参数..."
for q in 50 70 85 95; do
    output="$TEST_DIR/test_quality_$q.jpg"
    echo "  质量 $q: $output"
    "$DEMO_BIN" -i "$TEST_INPUT" -o "$output" -q "$q" > /dev/null 2>&1

    if [ -f "$output" ]; then
        size=$(stat -c%s "$output" 2>/dev/null || stat -f%z "$output")
        echo "    ✓ 大小: $size 字节"
    else
        echo "    ✗ 失败"
    fi
done
echo

echo "=== 测试完成 ==="
echo
echo "测试文件位于: $TEST_DIR"
echo "可以使用图片查看器打开输出文件进行检查"
