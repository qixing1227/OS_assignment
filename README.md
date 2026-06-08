# 🌌 OS-Simulation | 操作系统核心机制实验原型

![Language](https://img.shields.io/badge/Language-C-blue)
![Platform](https://img.shields.io/badge/Platform-Ubuntu%20Linux-orange)
![Build](https://img.shields.io/badge/Build-Makefile-success)

本项目为 **《操作系统》课程设计** 综合实践项目。项目基于纯 C 语言并在 Linux 原生环境（如 Ubuntu / WSL）下开发，通过代码重现了操作系统的四大核心底层机制，并实现了一个具备基础交互与进程管理的定制化 Shell 控制台。

本项目旨在帮助学习者深入理解操作系统底层的调度、内存、并发以及文件系统等抽象概念，将其具象化为可执行、可交互的代码模型。

---

## ✨ 核心功能模块 (Features)

本项目主要涵盖以下五大子模块：

### 1. 🖥️ 定制化控制台 (Custom Shell)
- **进程管理与交互**：基于 C 语言 `fork` 与 `execvp` 系统调用构建的底层 Shell，支持执行如 `ls`、`pwd` 等标准的 Linux 命令。
- **系统守护特性**：通过捕获并处理 `SIGINT` 信号，使该 Shell 免疫用户的 `Ctrl+C` 误操作，模拟了操作系统级守护进程的鲁棒性。
- **模块集成**：提供统一的入口命令（如 `run_scheduler`, `run_memory`, `run_fs`, `run_concurrency`）来快捷调用其他底层机制实验模块。

### 2. ⏱️ 处理机调度 (CPU Scheduling)
- 实现了 **FCFS** (先来先服务)、**SJF** (短作业优先) 以及 **RR** (时间片轮转) 等经典调度算法。
- 支持多进程参数模拟（自定义到达时间、执行时间），精确计算并输出等待时间与周转时间，直观展示进程在就绪队列与 CPU 之间的调度过程。

### 3. 💾 内存管理 (Memory Management)
- **动态分区分配**：实现了内存的首次适应 (First Fit) 和最佳适应 (Best Fit) 算法，模拟内存块的申请、分割与回收合并。
- **页面置换机制**：实现了虚拟内存的 FIFO (先进先出) 与 LRU (最近最久未使用) 页面置换算法，可直观对比不同算法在同等访问序列下的缺页率。

### 4. 🔄 并发与同步控制 (Concurrency Control)
- 基于 `pthread` 多线程库与互斥锁/信号量（Mutex & Semaphore）机制，彻底解决并发场景下的死锁与数据竞争。
- 实现了三大经典进程同步模型：
  - **生产者-消费者问题**
  - **读者-写者问题** (读者优先)
  - **哲学家进餐问题** (打破循环等待策略)

### 5. 🗄️ 文件系统模拟 (File System)
- 模拟磁盘空间分配（FAT表）、空闲空间管理以及逻辑块映射。
- 支持基础的系统操作逻辑，包括创建文件、删除文件、写入文件、读取文件以及列出当前目录状态。

---

## 📂 工程目录结构

```text
OS_assignment/
├── include/           # 头文件目录 (.h)
├── src/               # C语言核心源码 (.c)
│   ├── main.c         # 程序入口
│   ├── smart_shell.c  # 控制台与进程管理主循环
│   ├── scheduler.c    # 处理机调度模块
│   ├── memory_manager.c # 内存管理模块
│   ├── sync_concurrency.c # 并发与同步模块
│   └── file_system.c  # 文件系统模块
├── docs/              # 项目文档与实验报告
├── build/             # 编译生成的二进制文件 (被Git忽略)
├── Makefile           # GNU Make 自动化构建脚本
└── README.md          # 项目说明文档
```

---

## 🛠️ 环境依赖与部署

本项目依赖于 Linux 的原生系统调用，因此需要在以下环境中运行：
- **操作系统**：Ubuntu Linux 或 Windows 下的 **WSL (Windows Subsystem for Linux)**
- **编译器链**：`gcc`, `make`

---

## 🚀 编译与运行

项目内置了标准化的 `Makefile`，您只需在项目根目录下执行以下命令：

```bash
# 1. 清除旧产物并重新编译项目
make clean && make

# 2. 启动 OS 控制台
./build/os_simulation
```

*(注：如果您使用的是 Windows，请打开终端，确保已安装 WSL，并运行 `wsl ./build/os_simulation` 即可无缝体验！)*

进入控制台后，您可以输入常规的 Linux 系统命令，或是输入项目专属指令（如 `run_scheduler`）开启核心机制模拟测试。

---

**📝 License**: MIT License
