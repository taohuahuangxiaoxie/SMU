[TOC]



## 0. 约定与前置条件

- 宿主机：Windows 10/11 或 macOS（Apple Silicon / Intel 均可）
- IDE：Cursor（当前主流版本）
- 构建系统：CMake
- 编译目标：
  - amd64（本机 / 服务器）
  - arm64（嵌入式设备，交叉编译）
- 编译环境：Docker Linux 容器（Ubuntu 20.04/22.04）'



## 1. 安装 Docker Desktop 并启动 C++ Linux 容器

### 1.1 安装 Docker Desktop

- [官网下载安装（Windows / macOS）](https://docs.docker.com/desktop/setup/install/windows-install/)

- 安装完成后：

  - 启动 Docker Desktop
  - 确认右下角 / 菜单栏 Docker 图标为 **Running**

- 安装完成后，可以通过docker desktop面板拉取和启动容器，也可以通过PowerShell指令操作，以下为shell指令操作示例。

  - 注意：docker国内的镜像源已被禁止，如果需要在线拉取镜像，需要配置vpn，建议打开系统代理，打开TUN模式。

    

### 1.2 拉取并启动 C++ 开发容器

注意：以下为如何从0开始搭建编译容器，实际使用可直接至1.2.2，使用已配置好的容器

#### 1.2.1 构建ubuntu编译容器

以 Ubuntu + 常用 C++ 工具链为例：

```shell
docker pull ubuntu:20.04
```

注意：当前SMU盒子使用的ubuntu版本有20.04和22.04，必须基于最低版本的20.04进行交叉编译，高版本编译的程序在低版本上会有libc的兼容问题，解决起来较麻烦。

启动容器（**重点：目录挂载**）：

```shell
docker run -itd --privileged -v E:\work:/workspace --name cpp_run ubuntu:20.04 bash
```

说明：

- `E:\work:`：宿主机代码目录（自行替换）
- `/workspace`：容器内统一工作目录

进入容器后安装基础环境：

```shell
apt update
apt install -y \
  cmake \
  gdb \
  clang \
  vim
```

安装交叉编译环境：此处暂略。

#### 1.2.2 安装启动编译容器

拿到已打包的容器文件：cpp_run_image_v1.tar，powershell进入文件所在目录，执行以下命令：

```shell
# 加载容器
docker load -i cpp_run_image_v1.tar 
#启动容器
docker run -itd --privileged -v E:\work:/workspace --name cpp_run cpp_run_image:v1 bash
```

说明：

- `E:\work:`：宿主机代码目录（自行替换）
- `/workspace`：容器内统一工作目录



## 2. Cursor 的C++开发配置

### 2.1 解除大模型限制

当前模型提供方的政策问题，导致使用cursor时，有以下限制：

- **大陆节点不可用**：
  - Claude
  - GPT-4 / GPT-5 系列
- 可用节点：
  - 台湾（首选）
  - 日本 / 新加坡（次选）

#### 操作建议

- VPN 打开后：
  - 节点选择：台湾
  - 打开系统代理与TUN模式
  - 浏览器确认 IP 区域（重要）
- 重新启动 Cursor（重要）
- 登录模型后确认：
  - Claude / GPT 不再报地区限制

### 2.2 安装c++编码插件 

​	cursor支持vscode的插件市场安装，但是由于微软政策因素，直接安装的c++插件是不可用的（搜索安装enable之后也没用，实际不生效），解决这个问题，有两种方法。

- 安装clangd插件：直接在cursor插件栏搜索安装cland插件
- [安装c++较早版本](https://blog.csdn.net/beiguodexueCSDN/article/details/147126571)

注意，clangd和C/C++插件冲突，安装其中一个即可。

### 2.3 C++工程创建

至此，前置条件已经部署完毕，接下来就可以用agent对话的形式来创建和编译工程，必须清楚的告知ai使用docker linux容器进行编译，并且告知使用docker exec进行交互，以及使用cpp_run容器。

- 对话示例：

```
当前工程是在win环境里开发，在linux环境下运行，我用docker跑了一个cpp_run的编译、测试容器，smu工程挂载目录是/workspace/github/SMU，容器里我已经配置好了arm-64的交叉编译环境，现在是cursor写完代码我自己进容器手动编译测试的。我期望你能完成自动化编译测试的流程并固化下来，需要注意：先编译amd64的版本，再进行调测，测试通过后编译arm64和amd64的两个版本。你需要将这个流程固化下来，cmake参考已有的文件，交叉编译用tool/cmake工具配置，如果需要写编译、测试脚本，则放在tool/script目录下。暂不考虑gdb调试，仅使用docker exec执行命令交互。
```

- 已有的工程参考地址：[SMU_OUTSIDE(此工程仅做测试)](https://github.com/taohuahuangxiaoxie/SMU)。
  - 可以直接拉取到本地，再基于次工程让ai编写测试代码。

- 后续正式工程，会直接将工程相关的编译、设计文件固化，开发者让ai建立index理解项目之后，就可以直接开发。
  - 不建议AI写cmake和交叉编译，出错了仅依赖ai无法解决。
  - cursor自带代码review模式，审阅后再进行合并。

