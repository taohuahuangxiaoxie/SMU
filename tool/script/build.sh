#!/bin/bash
# 编译脚本
# 支持部分编译（指定项目）和整体编译（所有项目）

set -e  # 遇到错误立即退出

# 加载工具函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/utils.sh"

# 默认值
ARCH="amd64"
PROJECTS=""
BUILD_TYPE="Release"
CLEAN_BUILD=false

# 解析参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -a|--arch)
            ARCH="$2"
            shift 2
            ;;
        -p|--project)
            PROJECTS="$2"
            shift 2
            ;;
        -b|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --all)
            PROJECTS=""
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        -h|--help)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  -a, --arch ARCH          指定架构 (amd64|arm64)，默认: amd64"
            echo "  -p, --project PROJECTS   指定项目，多个项目用空格分隔，例如: 'wcdb_test zstd_test'"
            echo "                           不指定则编译所有项目"
            echo "  -b, --build-type TYPE    构建类型 (Debug|Release)，默认: Release"
            echo "  --all                    编译所有项目（默认行为）"
            echo "  --clean                  清理构建目录后重新编译"
            echo "  -h, --help               显示帮助信息"
            exit 0
            ;;
        *)
            log_error "未知参数: $1"
            echo "使用 -h 或 --help 查看帮助信息"
            exit 1
            ;;
    esac
done

# 验证架构
validate_arch "$ARCH" || exit 1

# 获取根目录
ROOT_DIR=$(get_root_dir)
log_info "项目根目录: $ROOT_DIR"

# 确定要编译的项目列表
if [ -z "$PROJECTS" ]; then
    # 编译所有项目
    PROJECTS=$(get_test_projects)
    log_info "编译所有项目: $PROJECTS"
else
    # 验证指定的项目
    for project in $PROJECTS; do
        validate_project "$project" || exit 1
    done
    log_info "编译指定项目: $PROJECTS"
fi

# 检查必要的命令
check_command "cmake" || exit 1
check_command "make" || exit 1

# 检查架构库目录是否存在
ARCH_LIB_DIR="$ROOT_DIR/lib/$ARCH"
if [ -d "$ARCH_LIB_DIR" ]; then
    ARCH_LIBS=$(find "$ARCH_LIB_DIR" -type f \( -name "*.so*" -o -name "*.a" \) 2>/dev/null)
    if [ -n "$ARCH_LIBS" ]; then
        log_info "使用架构库目录: $ARCH_LIB_DIR"
    else
        log_warning "架构目录 $ARCH_LIB_DIR 中没有找到库文件"
    fi
else
    log_warning "架构目录 $ARCH_LIB_DIR 不存在"
fi

# 创建顶层构建目录
TOP_BUILD_DIR="$ROOT_DIR/build-$ARCH"
if [ "$CLEAN_BUILD" = true ]; then
    log_info "清理构建目录: $TOP_BUILD_DIR"
    rm -rf "$TOP_BUILD_DIR"
fi
mkdir -p "$TOP_BUILD_DIR"

# 进入构建目录
cd "$TOP_BUILD_DIR"

# 配置CMake
log_info "配置CMake (架构: $ARCH, 构建类型: $BUILD_TYPE)..."
cmake "$ROOT_DIR" \
    -DARCH="$ARCH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    || exit_on_error "CMake配置失败"

# 编译
if [ -n "$PROJECTS" ]; then
    # 编译指定项目
    for project in $PROJECTS; do
        log_info "编译项目: $project"
        cmake --build . --target "$project" -j$(nproc 2>/dev/null || echo 4) \
            || exit_on_error "编译项目 $project 失败"
        log_success "项目 $project 编译成功"
    done
else
    # 编译所有项目
    log_info "编译所有项目..."
    cmake --build . -j$(nproc 2>/dev/null || echo 4) \
        || exit_on_error "编译失败"
    log_success "所有项目编译成功"
fi

log_success "编译完成！"
log_info "构建目录: $TOP_BUILD_DIR"

