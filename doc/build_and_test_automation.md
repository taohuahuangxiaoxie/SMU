# 自动化编译测试流程文档

## 概述

本文档描述了SMU项目的自动化编译测试流程实现。该流程支持在Docker容器中进行跨平台编译（AMD64和ARM64），并提供完整的编译、测试和发布自动化。

### 主要特性

- ✅ **部分编译支持**：可以只编译指定的测试项目，加快开发迭代速度
- ✅ **整体编译支持**：支持编译所有测试项目，确保完整性
- ✅ **多架构支持**：支持AMD64和ARM64架构的编译和测试
- ✅ **自动化流程**：一键完成编译→测试→发布的完整流程
- ✅ **Cursor集成**：通过`.cursor/tasks.json`集成到IDE，方便使用
- ✅ **智能库管理**：自动根据架构切换库文件，统一链接路径

## 项目结构

```
SMU/
├── .cursor/
│   └── tasks.json                    # Cursor任务配置（IDE集成）
├── CMakeLists.txt                    # 顶层CMakeLists，统一管理所有测试项目
├── tool/
│   ├── cmake/
│   │   ├── toolchain_arm64.cmake     # ARM64交叉编译工具链配置
│   │   └── common.cmake              # 通用CMake配置（架构选择、路径设置）
│   └── script/
│       ├── utils.sh                  # 工具函数库（日志、验证、路径处理）
│       ├── build.sh                  # 编译脚本（支持部分/整体编译）
│       ├── test.sh                   # 测试脚本（支持部分/整体测试）
│       └── build_and_test.sh         # 主流程脚本（完整流程）
└── src/21-test_demo/
    ├── wcdb_test/CMakeLists.txt      # wcdb_test项目配置（已修改支持架构选择）
    └── zstd_test/CMakeLists.txt      # zstd_test项目配置（已修改支持架构选择）
```

## 文件说明

### CMake配置文件

#### `CMakeLists.txt` (顶层)
- **功能**：统一管理所有测试项目
- **内容**：
  - 设置C++标准（C++14）
  - 包含通用配置（`tool/cmake/common.cmake`）
  - 添加所有测试项目子目录

#### `tool/cmake/common.cmake`
- **功能**：通用CMake配置，支持架构选择和路径配置
- **关键配置**：
  - 架构选择：通过`ARCH`变量（amd64/arm64）
  - 库目录：统一使用`lib/`目录（库文件在编译前由脚本拷贝）
  - 头文件目录：`src/10-include`
  - 构建类型：Debug/Release
  - 交叉编译：ARM64时自动加载`toolchain_arm64.cmake`

#### `tool/cmake/toolchain_arm64.cmake`
- **功能**：ARM64交叉编译工具链配置
- **内容**：
  - 设置交叉编译器：`aarch64-linux-gnu-gcc` / `aarch64-linux-gnu-g++`
  - 配置系统类型：Linux aarch64

### 脚本文件

#### `tool/script/utils.sh`
- **功能**：工具函数库
- **提供的函数**：
  - `log_info/log_success/log_warning/log_error`：日志输出
  - `check_command/check_directory/check_file`：验证函数
  - `exit_on_error`：错误处理
  - `get_root_dir`：获取项目根目录
  - `get_test_projects`：获取所有测试项目列表
  - `validate_project/validate_arch`：验证函数
  - `get_build_dir/get_executable_path`：路径处理函数

#### `tool/script/build.sh`
- **功能**：编译脚本，支持部分编译和整体编译
- **参数**：
  - `-a, --arch ARCH`：指定架构（amd64/arm64），默认：amd64
  - `-p, --project PROJECTS`：指定项目，多个项目用空格分隔
  - `-b, --build-type TYPE`：构建类型（Debug/Release），默认：Release
  - `--all`：编译所有项目（默认行为）
  - `--clean`：清理构建目录后重新编译
  - `-h, --help`：显示帮助信息
- **工作流程**：
  1. 准备库文件：将对应架构的库从`lib/{arch}/`拷贝到`lib/`
  2. 创建构建目录：`build-{arch}/`
  3. 配置CMake
  4. 编译指定项目或所有项目

#### `tool/script/test.sh`
- **功能**：测试脚本，支持部分测试和整体测试
- **参数**：
  - `-a, --arch ARCH`：指定架构（amd64/arm64），默认：amd64
  - `-p, --project PROJECTS`：指定项目，多个项目用空格分隔
  - `--all`：测试所有项目（默认行为）
  - `--timeout SECONDS`：测试超时时间（秒），默认：300
  - `-h, --help`：显示帮助信息
- **工作流程**：
  1. 准备库文件：确保使用正确架构的库
  2. 查找可执行文件
  3. 运行测试（支持超时控制）
  4. 统计测试结果

#### `tool/script/build_and_test.sh`
- **功能**：主流程脚本，实现完整的编译→测试→发布流程
- **参数**：
  - `-p, --project PROJECTS`：指定项目，多个项目用空格分隔
  - `-b, --build-type TYPE`：构建类型（Debug/Release），默认：Release
  - `--all`：处理所有项目（默认行为）
  - `--skip-final-build`：跳过最后的amd64编译步骤
  - `-h, --help`：显示帮助信息
- **完整流程**：
  1. **阶段1**：编译AMD64版本
  2. **阶段2**：测试AMD64版本（如果失败，流程终止）
  3. **阶段3**：如果测试通过，编译ARM64版本
  4. **阶段4**：如果测试通过，再次编译AMD64版本（确保一致性）

### Cursor任务配置

#### `.cursor/tasks.json`
- **功能**：Cursor IDE任务配置，提供可视化任务执行
- **定义的任务**：
  - `Build and Test: Full Flow (All Projects)`：完整流程（所有项目）
  - `Build and Test: wcdb_test`：wcdb_test的完整流程
  - `Build and Test: zstd_test`：zstd_test的完整流程
  - `Build: All Projects (AMD64)`：编译所有项目（AMD64）
  - `Build: wcdb_test (AMD64)`：编译wcdb_test（AMD64）
  - `Build: zstd_test (AMD64)`：编译zstd_test（AMD64）
  - `Build: wcdb_test (ARM64)`：编译wcdb_test（ARM64）
  - `Build: zstd_test (ARM64)`：编译zstd_test（ARM64）
  - `Test: All Projects (AMD64)`：测试所有项目（AMD64）
  - `Test: wcdb_test (AMD64)`：测试wcdb_test（AMD64）
  - `Test: zstd_test (AMD64)`：测试zstd_test（AMD64）
  - `Clean: Build Directories`：清理所有构建目录

## 使用方法

### 方式1：通过Cursor Tasks（推荐）

1. 按 `Ctrl+Shift+P`（Windows/Linux）或 `Cmd+Shift+P`（Mac）
2. 输入 "Tasks: Run Task"
3. 选择需要的任务，例如：
   - `Build and Test: Full Flow (All Projects)` - 完整流程
   - `Build: wcdb_test (AMD64)` - 只编译wcdb_test
   - `Test: wcdb_test (AMD64)` - 只测试wcdb_test

### 方式2：在容器内手动执行脚本

```bash
# 进入容器
docker exec -it -w /workspace/github/SMU cpp_run bash

# 编译单个项目
bash tool/script/build.sh -p wcdb_test -a amd64

# 编译所有项目
bash tool/script/build.sh -a amd64 --all

# 测试单个项目
bash tool/script/test.sh -p wcdb_test -a amd64

# 完整流程（单个项目）
bash tool/script/build_and_test.sh -p wcdb_test

# 完整流程（所有项目）
bash tool/script/build_and_test.sh --all
```

### 方式3：通过Docker Exec直接执行

```bash
# 编译
docker exec -w /workspace/github/SMU cpp_run bash tool/script/build.sh -a amd64 --all

# 测试
docker exec -w /workspace/github/SMU cpp_run bash tool/script/test.sh -a amd64 --all

# 完整流程
docker exec -w /workspace/github/SMU cpp_run bash tool/script/build_and_test.sh --all
```

## 库文件管理机制

### 设计理念

为了简化链接逻辑，统一使用`lib/`目录进行链接。编译和测试时，脚本会自动将对应架构的库文件从`lib/{arch}/`目录拷贝到`lib/`目录。

### 目录结构

```
lib/
├── amd64/          # AMD64架构的库文件（可选）
├── arm64/          # ARM64架构的库文件
│   ├── libzstd.a
│   └── libzstd.so.1.5.7
├── libWCDB.so      # 当前使用的库文件（由脚本自动更新）
├── libzstd.a
└── libzstd.so.1.5.7
```

### 工作流程

1. **编译前**：`build.sh`会将`lib/{arch}/`目录下的库文件拷贝到`lib/`
2. **链接时**：CMake统一使用`lib/`目录进行链接
3. **测试前**：`test.sh`也会确保使用正确架构的库文件

### 优势

- ✅ 简化CMake配置：不需要根据架构选择不同的库路径
- ✅ 统一链接逻辑：所有架构使用相同的链接配置
- ✅ 自动切换：编译不同架构时自动更新库文件

## 构建输出

### 构建目录结构

```
build-amd64/                    # AMD64构建目录
├── bin-amd64/                  # 可执行文件输出目录
│   ├── wcdb_test
│   └── zstd_test
└── ...                         # CMake生成的文件

build-arm64/                    # ARM64构建目录
├── bin-arm64/                  # 可执行文件输出目录
│   ├── wcdb_test
│   └── zstd_test
└── ...                         # CMake生成的文件
```

### 可执行文件位置

- AMD64版本：`build-amd64/bin-amd64/{project_name}`
- ARM64版本：`build-arm64/bin-arm64/{project_name}`

## 配置说明

### 容器配置

- **容器名称**：`cpp_run`（可在`.cursor/tasks.json`中修改）
- **工作目录**：`/workspace/github/SMU`
- **要求**：容器内需要安装cmake、make、gcc/g++等构建工具

### 环境要求

- **CMake版本**：>= 3.10
- **C++标准**：C++14
- **编译器**：
  - AMD64：系统默认gcc/g++
  - ARM64：aarch64-linux-gnu-gcc/aarch64-linux-gnu-g++

### 架构支持

- **AMD64**：使用系统默认编译器
- **ARM64**：使用交叉编译工具链（`tool/cmake/toolchain_arm64.cmake`）

## 工作流程示例

### 场景1：日常开发（部分编译）

```bash
# 1. 修改了wcdb_test.cpp
# 2. 在Cursor中执行任务：Build: wcdb_test (AMD64)
# 3. 快速编译，只编译wcdb_test
# 4. 执行任务：Test: wcdb_test (AMD64)
# 5. 验证功能正常
```

### 场景2：发布前验证（完整流程）

```bash
# 1. 执行任务：Build and Test: Full Flow (All Projects)
# 2. 流程自动执行：
#    - 编译AMD64版本
#    - 测试AMD64版本
#    - 如果测试通过，编译ARM64版本
#    - 如果测试通过，再次编译AMD64版本
# 3. 确保所有项目在两个架构上都能正常编译和运行
```

### 场景3：交叉编译验证

```bash
# 1. 编译ARM64版本
bash tool/script/build.sh -a arm64 -p wcdb_test

# 2. 验证ARM64版本编译成功
ls build-arm64/bin-arm64/wcdb_test
```

## 注意事项

### 测试程序特性

- **wcdb_test**：长时间运行的程序，每10秒查询一次数据库，持续运行
- **zstd_test**：循环测试程序，持续运行

如果需要自动测试这些程序，建议：
- 设置较短的超时时间（如30秒）验证程序能正常启动
- 或者修改测试程序，添加命令行参数控制运行时间

### 库文件管理

- 库文件会在编译/测试前自动更新
- 如果`lib/{arch}/`目录不存在或为空，会使用现有的`lib/`目录中的库文件
- 建议在版本控制中忽略`lib/`目录下的库文件（只保留`lib/{arch}/`目录）

### 构建目录清理

- 使用`--clean`参数可以清理构建目录
- 或者使用Cursor任务：`Clean: Build Directories`

### 权限问题

- 脚本文件需要有执行权限
- 如果遇到权限问题，执行：`chmod +x tool/script/*.sh`

## 故障排除

### 问题1：找不到cmake命令

**症状**：`[ERROR] 命令 'cmake' 未找到，请先安装`

**解决方案**：
- 确认容器内已安装cmake：`docker exec cpp_run which cmake`
- 如果未安装，需要安装cmake（但根据实际情况，容器内应该已有cmake）

### 问题2：找不到库文件

**症状**：`找不到 zstd 库文件，查找路径: /workspace/github/SMU/lib`

**解决方案**：
- 检查`lib/{arch}/`目录是否存在库文件
- 确认库文件拷贝逻辑正常工作
- 检查库文件权限

### 问题3：交叉编译失败

**症状**：ARM64编译时找不到交叉编译器

**解决方案**：
- 确认容器内已安装ARM64交叉编译工具链
- 检查`tool/cmake/toolchain_arm64.cmake`配置是否正确
- 确认工具链路径：`which aarch64-linux-gnu-gcc`

### 问题4：测试程序长时间运行

**症状**：测试脚本一直等待，不退出

**解决方案**：
- 这是正常现象，因为测试程序是长时间运行的
- 可以设置较短的超时时间：`--timeout 30`
- 或者手动中断测试（Ctrl+C）

## 扩展说明

### 添加新的测试项目

1. 在`src/21-test_demo/`目录下创建新项目目录
2. 创建`CMakeLists.txt`，参考现有项目的配置
3. 在顶层`CMakeLists.txt`中添加`add_subdirectory`
4. 脚本会自动发现新项目

### 修改容器名称

如果Docker容器名称不是`cpp_run`，需要修改：
- `.cursor/tasks.json`中的所有`docker exec`命令
- 或者使用环境变量配置

### 添加新的架构支持

1. 在`tool/cmake/`目录下创建新的工具链文件（如`toolchain_riscv64.cmake`）
2. 修改`tool/cmake/common.cmake`，添加新架构的支持
3. 修改脚本中的架构验证逻辑

## 更新日志

- **2025-12-15**：初始版本
  - 实现部分编译和整体编译支持
  - 实现多架构编译支持（AMD64/ARM64）
  - 实现自动化编译测试流程
  - 集成Cursor Tasks
  - 实现智能库文件管理机制

## 相关文件

- 顶层CMakeLists：`CMakeLists.txt`
- CMake通用配置：`tool/cmake/common.cmake`
- 交叉编译工具链：`tool/cmake/toolchain_arm64.cmake`
- 编译脚本：`tool/script/build.sh`
- 测试脚本：`tool/script/test.sh`
- 主流程脚本：`tool/script/build_and_test.sh`
- Cursor任务配置：`.cursor/tasks.json`

