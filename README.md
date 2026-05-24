# 🌌 SmartOS-Simulation | 操作系统核心机制与智能大模型终端原型

![Language](https://img.shields.io/badge/Language-C%20%7C%20Python-blue)
![Platform](https://img.shields.io/badge/Platform-Ubuntu%20Linux-orange)
![Build](https://img.shields.io/badge/Build-Makefile-success)
![AI-Powered](https://img.shields.io/badge/AI_Powered-Ollama_Qwen-purple)

本项目为 **2026年《操作系统》课程设计** 综合实践项目。项目采用“基础必做 + 自由扩展”两级体系，在纯 C 语言 Linux 原生环境下，不仅重现了操作系统的四大核心底层机制，更创新性地引入了 **AI 大模型驱动的智能 Shell 原型**，实现了从“命令行驱动”到“自然语言意图驱动”的系统交互跨越。

---

## ✨ 核心功能模块 (Features)

### 🧱 基础架构层 (70% 必做模块)
1. **处理机调度 (CPU Scheduling)**
   - 实现了 FCFS (先来先服务)、SJF (短作业优先) 等经典调度算法。
   - 支持多进程参数模拟，精确计算周转时间与带权周转时间。
2. **内存管理 (Memory Management)**
   - 实现了动态分区分配与页面置换机制的底层模拟。
3. **并发与同步控制 (Concurrency Control)**
   - 基于 `pthread` 多线程库与互斥锁/信号量机制。
   - 彻底解决死锁与数据竞争，实现典型的哲学家进餐等并发模型。
4. **文件系统模拟 (File System)**
   - 模拟磁盘空间分配、空闲空间管理以及文件的基础操作逻辑。

### 🧠 创新拓展层 (30% 自由扩展：智能 OS 原型)
- 🤖 **意图驱动解释器 (SmartShell)**：基于 C 语言 `fork` 与 `execvp` 构建自定义 Shell 控制台，支持标准 Linux 命令。
- 🔗 **LLM 进程通信桥梁**：通过 `popen` 与 Python 桥接层通信，将自然语言请求直接发送至本地局域网的大语言模型（默认适配 Ollama + Qwen2.5:7b）。
- 🛡️ **安全沙箱防注入机制**：当 AI 将意图转换为高危系统命令时，系统主动拦截并触发 `[y/n]` 授权确认，彻底杜绝大模型幻觉摧毁文件系统。
- 🛡️ **底层信号截获**：通过捕获 `SIGINT` 信号，使 SmartShell 免疫用户的 `Ctrl+C` 误杀，具备真正的操作系统守护进程级鲁棒性。

---

## 📂 工程目录结构

```text
OS_assignment/
├── include/           # 头文件目录 (.h)
├── src/               # C语言核心源码 (.c)
│   └── smart_shell.c  # 智能控制台与进程管理主循环
├── scripts/           # 辅助脚本目录
│   └── llm_bridge.py  # 连接本地大模型的通信桥梁
├── docs/              # 项目文档与实验报告
├── build/             # 编译生成的二进制文件 (被Git忽略)
├── Makefile           # GNU Make 自动化构建脚本
└── README.md          # 项目说明文档
```

---

## 🛠️ 环境依赖与部署

### 1. 基础环境
- 操作系统：**Ubuntu Linux** (原生环境推荐)
- 编译器链：`gcc`, `make`
- 脚本环境：`python3`

### 2. AI 引擎配置 (本地运行)
本机智能 Shell 强依赖于本地部署的大模型引擎。请在终端执行以下命令安装 Ollama 并拉取模型：
```bash
# 1. 安装 Ollama 引擎
curl -fsSL https://ollama.com/install.sh | sh

# 2. 下载并后台运行 Qwen2.5 模型 (约占 4.7G 显存)
ollama run qwen2.5:7b
```
*(注：待模型下载完毕并出现 `>>>` 提示符后，输入 `/bye` 退出对话即可，服务将在后台保持静默运行。)*

---

## 🚀 编译与一键运行

项目内置了标准化的 `Makefile`，无需输入冗长的编译参数，请在项目根目录下执行：

```bash
# 清理历史构建产物，并重新编译链接所有模块
make clean && make

# 启动 SmartOS 智能控制台
./build/os_simulation
```

进入控制台后，您不仅可以输入常规的 `ls -l`、`pwd` 等系统命令，更可以通过 **`@` 符号唤醒 AI**：
> `SmartOS > @帮我找出当前目录下占用空间最大的3个文件`

---

## 📸 运行效果演示

*(请评审老师参考下方系统运行截图，展示了 AI 自然语言解析与安全沙箱拦截机制)*

> **[在此处拖入你的系统运行截图]**

---
**📝 License**: MIT License
