#!/bin/bash
# 主流程脚本：编译 -> 测试 -> 发布
# 流程：编译amd64 -> 测试 -> 测试通过后编译arm64和amd64

set -e  # 遇到错误立即退出

# 加载工具函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/utils.sh"

# 默认值
PROJECTS=""
BUILD_TYPE="Release"
SKIP_FINAL_BUILD=false  # 是否跳过最后的amd64编译

# 解析参数
while [[ $# -gt 0 ]]; do
    case $1 in
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
        --skip-final-build)
            SKIP_FINAL_BUILD=true
            shift
            ;;
        -h|--help)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  -p, --project PROJECTS   指定项目，多个项目用空格分隔"
            echo "                           不指定则处理所有项目"
            echo "  -b, --build-type TYPE    构建类型 (Debug|Release)，默认: Release"
            echo "  --all                    处理所有项目（默认行为）"
            echo "  --skip-final-build       跳过最后的amd64编译步骤"
            echo "  -h, --help               显示帮助信息"
            echo ""
            echo "流程说明:"
            echo "  1. 编译 amd64 版本"
            echo "  2. 测试 amd64 版本"
            echo "  3. 如果测试通过，编译 arm64 版本"
            echo "  4. 如果测试通过，再次编译 amd64 版本（确保一致性）"
            exit 0
            ;;
        *)
            log_error "未知参数: $1"
            echo "使用 -h 或 --help 查看帮助信息"
            exit 1
            ;;
    esac
done

# 获取根目录
ROOT_DIR=$(get_root_dir)
log_info "=========================================="
log_info "开始完整构建和测试流程"
log_info "项目根目录: $ROOT_DIR"
log_info "=========================================="

# 确定要处理的项目列表
if [ -z "$PROJECTS" ]; then
    PROJECTS=$(get_test_projects)
    log_info "处理所有项目: $PROJECTS"
else
    for project in $PROJECTS; do
        validate_project "$project" || exit 1
    done
    log_info "处理指定项目: $PROJECTS"
fi

# 构建项目参数（用于传递给build.sh和test.sh）
# 注意：由于需要处理多个项目，我们直接传递参数而不是构建字符串

# ==========================================
# 阶段1: 编译 amd64 版本
# ==========================================
log_info ""
log_info "=========================================="
log_info "阶段1: 编译 AMD64 版本"
log_info "=========================================="
if [ -n "$PROJECTS" ]; then
    bash "$SCRIPT_DIR/build.sh" -a amd64 -p "$PROJECTS" -b "$BUILD_TYPE" || {
        log_error "阶段1失败: AMD64编译失败"
        exit 1
    }
else
    bash "$SCRIPT_DIR/build.sh" -a amd64 --all -b "$BUILD_TYPE" || {
        log_error "阶段1失败: AMD64编译失败"
        exit 1
    }
fi
log_success "阶段1完成: AMD64编译成功"

# ==========================================
# 阶段2: 测试 amd64 版本
# ==========================================
log_info ""
log_info "=========================================="
log_info "阶段2: 测试 AMD64 版本"
log_info "=========================================="
if [ -n "$PROJECTS" ]; then
    bash "$SCRIPT_DIR/test.sh" -a amd64 -p "$PROJECTS" || {
        log_error "阶段2失败: AMD64测试失败"
        log_error "流程终止，不会继续编译ARM64版本"
        exit 1
    }
else
    bash "$SCRIPT_DIR/test.sh" -a amd64 --all || {
        log_error "阶段2失败: AMD64测试失败"
        log_error "流程终止，不会继续编译ARM64版本"
        exit 1
    }
fi
log_success "阶段2完成: AMD64测试通过"

# ==========================================
# 阶段3: 编译 arm64 版本
# ==========================================
log_info ""
log_info "=========================================="
log_info "阶段3: 编译 ARM64 版本"
log_info "=========================================="
if [ -n "$PROJECTS" ]; then
    bash "$SCRIPT_DIR/build.sh" -a arm64 -p "$PROJECTS" -b "$BUILD_TYPE" || {
        log_error "阶段3失败: ARM64编译失败"
        exit 1
    }
else
    bash "$SCRIPT_DIR/build.sh" -a arm64 --all -b "$BUILD_TYPE" || {
        log_error "阶段3失败: ARM64编译失败"
        exit 1
    }
fi
log_success "阶段3完成: ARM64编译成功"

# ==========================================
# 阶段4: 再次编译 amd64 版本（确保一致性）
# ==========================================
if [ "$SKIP_FINAL_BUILD" = false ]; then
    log_info ""
    log_info "=========================================="
    log_info "阶段4: 再次编译 AMD64 版本（确保一致性）"
    log_info "=========================================="
    if [ -n "$PROJECTS" ]; then
        bash "$SCRIPT_DIR/build.sh" -a amd64 -p "$PROJECTS" -b "$BUILD_TYPE" || {
            log_error "阶段4失败: AMD64重新编译失败"
            exit 1
        }
    else
        bash "$SCRIPT_DIR/build.sh" -a amd64 --all -b "$BUILD_TYPE" || {
            log_error "阶段4失败: AMD64重新编译失败"
            exit 1
        }
    fi
    log_success "阶段4完成: AMD64重新编译成功"
else
    log_info ""
    log_info "跳过阶段4: 最终AMD64编译（用户指定）"
fi

# ==========================================
# 完成
# ==========================================
log_info ""
log_info "=========================================="
log_success "所有阶段完成！"
log_info "=========================================="
log_info "构建输出:"
if [ -n "$PROJECTS" ]; then
    PROJECT_LIST="$PROJECTS"
else
    PROJECT_LIST=$(get_test_projects)
fi

for project in $PROJECT_LIST; do
    amd64_path=$(get_executable_path "$project" amd64 2>/dev/null || echo "未找到")
    arm64_path=$(get_executable_path "$project" arm64 2>/dev/null || echo "未找到")
    log_info "  - $project (AMD64): $amd64_path"
    log_info "  - $project (ARM64): $arm64_path"
done

