# SMU项目文档

## 文档索引

### 构建和测试

- [自动化编译测试流程](./build_and_test_automation.md) - 完整的编译、测试自动化流程文档

## 快速开始

### 编译和测试

1. **通过Cursor Tasks（推荐）**：
   - 按 `Ctrl+Shift+P`
   - 输入 "Tasks: Run Task"
   - 选择需要的任务

2. **在容器内执行**：
   ```bash
   docker exec -it -w /workspace/github/SMU cpp_run bash
   bash tool/script/build.sh -a amd64 --all
   bash tool/script/test.sh -a amd64 --all
   ```

详细说明请参考：[自动化编译测试流程文档](./build_and_test_automation.md)

## 项目结构

```
SMU/
├── doc/                    # 项目文档
├── tool/                   # 工具脚本和配置
│   ├── cmake/             # CMake配置文件
│   └── script/            # 自动化脚本
├── src/                   # 源代码
│   ├── 10-include/        # 头文件
│   └── 21-test_demo/      # 测试项目
└── lib/                   # 库文件
```

## 联系方式

如有问题或建议，请参考相关文档或联系项目维护者。

