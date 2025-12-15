#!/bin/bash
# 工具函数库

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

# 检查命令是否存在
check_command() {
    if ! command -v "$1" &> /dev/null; then
        log_error "命令 '$1' 未找到，请先安装"
        return 1
    fi
    return 0
}

# 检查目录是否存在
check_directory() {
    if [ ! -d "$1" ]; then
        log_error "目录不存在: $1"
        return 1
    fi
    return 0
}

# 检查文件是否存在
check_file() {
    if [ ! -f "$1" ]; then
        log_error "文件不存在: $1"
        return 1
    fi
    return 0
}

# 错误退出
exit_on_error() {
    if [ $? -ne 0 ]; then
        log_error "$1"
        exit 1
    fi
}

# 获取项目根目录（脚本所在目录的上级目录的上级目录）
get_root_dir() {
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    echo "$(cd "$script_dir/../.." && pwd)"
}

# 获取所有测试项目列表
get_test_projects() {
    local root_dir=$(get_root_dir)
    local test_dir="$root_dir/src/21-test_demo"
    
    if [ ! -d "$test_dir" ]; then
        log_error "测试目录不存在: $test_dir"
        return 1
    fi
    
    # 查找所有包含CMakeLists.txt的子目录
    local projects=()
    for dir in "$test_dir"/*; do
        if [ -d "$dir" ] && [ -f "$dir/CMakeLists.txt" ]; then
            projects+=("$(basename "$dir")")
        fi
    done
    
    echo "${projects[@]}"
}

# 验证项目名称
validate_project() {
    local project=$1
    local root_dir=$(get_root_dir)
    local project_dir="$root_dir/src/21-test_demo/$project"
    
    if [ ! -d "$project_dir" ]; then
        log_error "项目目录不存在: $project_dir"
        return 1
    fi
    
    if [ ! -f "$project_dir/CMakeLists.txt" ]; then
        log_error "项目CMakeLists.txt不存在: $project_dir/CMakeLists.txt"
        return 1
    fi
    
    return 0
}

# 验证架构
validate_arch() {
    local arch=$1
    if [ "$arch" != "amd64" ] && [ "$arch" != "arm64" ]; then
        log_error "无效的架构: $arch，必须是 'amd64' 或 'arm64'"
        return 1
    fi
    return 0
}

# 获取构建目录
get_build_dir() {
    local root_dir=$(get_root_dir)
    local project=$1
    local arch=$2
    echo "$root_dir/src/21-test_demo/$project/build-$arch"
}

# 获取可执行文件路径
get_executable_path() {
    local root_dir=$(get_root_dir)
    local project=$1
    local arch=$2
    
    # 顶层构建目录（使用顶层CMakeLists.txt时）
    local top_build_dir="$root_dir/build-$arch"
    if [ -f "$top_build_dir/bin-$arch/$project" ]; then
        echo "$top_build_dir/bin-$arch/$project"
        return 0
    fi
    
    # 项目构建目录（旧格式或独立编译）
    local build_dir=$(get_build_dir "$project" "$arch")
    if [ -f "$build_dir/$project" ]; then
        echo "$build_dir/$project"
        return 0
    fi
    
    # 如果都找不到，返回错误
    return 1
}

