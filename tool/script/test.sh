#!/bin/bash
# 测试脚本
# 支持部分测试（指定项目）和整体测试（所有项目）

set -e  # 遇到错误立即退出

# 加载工具函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/utils.sh"

# 默认值
ARCH="amd64"
PROJECTS=""
TIMEOUT=300  # 默认超时5分钟

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
        --all)
            PROJECTS=""
            shift
            ;;
        --timeout)
            TIMEOUT="$2"
            shift 2
            ;;
        -h|--help)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  -a, --arch ARCH          指定架构 (amd64|arm64)，默认: amd64"
            echo "  -p, --project PROJECTS   指定项目，多个项目用空格分隔"
            echo "                           不指定则测试所有项目"
            echo "  --all                    测试所有项目（默认行为）"
            echo "  --timeout SECONDS        测试超时时间（秒），默认: 300"
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

# 准备库文件：确保使用正确架构的库
log_info "准备库文件 (架构: $ARCH)..."
ARCH_LIB_DIR="$ROOT_DIR/lib/$ARCH"
MAIN_LIB_DIR="$ROOT_DIR/lib"

if [ -d "$ARCH_LIB_DIR" ]; then
    # 检查架构目录是否有库文件
    ARCH_LIBS=$(find "$ARCH_LIB_DIR" -type f \( -name "*.so*" -o -name "*.a" \) 2>/dev/null)
    if [ -n "$ARCH_LIBS" ]; then
        log_info "从 $ARCH_LIB_DIR 拷贝库文件到 $MAIN_LIB_DIR"
        # 拷贝所有库文件（.so, .so.*, .a）
        find "$ARCH_LIB_DIR" -type f \( -name "*.so*" -o -name "*.a" \) -exec cp -f {} "$MAIN_LIB_DIR/" \;
        log_success "库文件准备完成"
    else
        log_warning "架构目录 $ARCH_LIB_DIR 中没有找到库文件，使用现有库文件"
    fi
else
    log_warning "架构目录 $ARCH_LIB_DIR 不存在，使用现有库文件"
fi

# 确定要测试的项目列表
if [ -z "$PROJECTS" ]; then
    # 测试所有项目
    PROJECTS=$(get_test_projects)
    log_info "测试所有项目: $PROJECTS"
else
    # 验证指定的项目
    for project in $PROJECTS; do
        validate_project "$project" || exit 1
    done
    log_info "测试指定项目: $PROJECTS"
fi

# 检查timeout命令（用于超时控制）
if ! command -v timeout &> /dev/null; then
    log_warning "timeout 命令未找到，将不使用超时控制"
    USE_TIMEOUT=false
else
    USE_TIMEOUT=true
fi

# 测试结果统计
PASSED=0
FAILED=0
FAILED_PROJECTS=()

# 运行测试
for project in $PROJECTS; do
    log_info "=========================================="
    log_info "测试项目: $project (架构: $ARCH)"
    log_info "=========================================="
    
    # 获取可执行文件路径
    EXECUTABLE=$(get_executable_path "$project" "$ARCH")
    
    if [ $? -ne 0 ] || [ ! -f "$EXECUTABLE" ]; then
        log_error "可执行文件不存在: $project"
        log_error "请先编译项目: bash tool/script/build.sh -p $project -a $ARCH"
        FAILED=$((FAILED + 1))
        FAILED_PROJECTS+=("$project")
        continue
    fi
    
    log_info "可执行文件: $EXECUTABLE"
    
    # 创建日志目录（在可执行文件所在目录或项目构建目录）
    EXECUTABLE_DIR=$(dirname "$EXECUTABLE")
    LOG_DIR="$EXECUTABLE_DIR/logs"
    mkdir -p "$LOG_DIR"
    
    # 进入可执行文件所在目录运行测试（确保相对路径正确，如logs目录）
    cd "$EXECUTABLE_DIR"
    
    # 运行测试（带超时控制）
    log_info "开始运行测试..."
    TEST_START_TIME=$(date +%s)
    
    if [ "$USE_TIMEOUT" = true ]; then
        if timeout "$TIMEOUT" "$EXECUTABLE"; then
            TEST_EXIT_CODE=0
        else
            TEST_EXIT_CODE=$?
            if [ $TEST_EXIT_CODE -eq 124 ]; then
                log_error "测试超时（超过 ${TIMEOUT} 秒）"
            fi
        fi
    else
        "$EXECUTABLE"
        TEST_EXIT_CODE=$?
    fi
    
    TEST_END_TIME=$(date +%s)
    TEST_DURATION=$((TEST_END_TIME - TEST_START_TIME))
    
    # 检查测试结果
    if [ $TEST_EXIT_CODE -eq 0 ]; then
        log_success "项目 $project 测试通过 (耗时: ${TEST_DURATION}秒)"
        PASSED=$((PASSED + 1))
    else
        log_error "项目 $project 测试失败 (退出码: $TEST_EXIT_CODE, 耗时: ${TEST_DURATION}秒)"
        FAILED=$((FAILED + 1))
        FAILED_PROJECTS+=("$project")
    fi
    
    echo ""
done

# 输出测试摘要
log_info "=========================================="
log_info "测试摘要"
log_info "=========================================="
log_info "通过: $PASSED"
if [ $FAILED -gt 0 ]; then
    log_error "失败: $FAILED"
    log_error "失败的项目: ${FAILED_PROJECTS[*]}"
    exit 1
else
    log_success "失败: $FAILED"
    log_success "所有测试通过！"
    exit 0
fi

