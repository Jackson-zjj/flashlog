# FlashLog —— 闪存日志

<p align="center">
  <b>一个高性能、可扩展的 C++ 日志组件，专为高并发、高可靠性场景设计</b>
</p>

---

## 📖 项目简介

FlashLog 是一个现代 C++20 日志库，提供从日志写入到日志解析的完整解决方案。核心设计理念是**高性能**、**安全可靠**和**跨平台兼容**。

通过 mmap 内存映射实现零拷贝写入，结合异步调度、压缩、加密等功能模块，FlashLog 能够在保证日志不丢失的前提下，最大限度地降低对业务线程的性能影响。

---

## ✨ 核心特性

### 🚀 高性能
- **mmap 内存映射写入**：基于零拷贝技术，大幅降低磁盘 IO 开销，同时防止进程崩溃导致日志丢失
- **异步日志调度**：内置线程池 + 任务调度器，避免阻塞业务线程
- **双缓冲机制**：master/slave 双 mmap 缓冲区交替写入，实现无锁高吞吐
- **流式压缩**：支持 zstd/zlib 流式压缩，显著减少磁盘占用

### 🔒 安全与可靠
- **ECDH 密钥协商 + AES 加密**：基于椭圆曲线 Diffie-Hellman 密钥交换，每个日志 chunk 独立加密
- **崩溃恢复**：mmap 映射确保即使进程异常退出，已写入内存的日志数据也不会丢失
- **日志分片与淘汰**：自动按大小分片，超出总容量时自动淘汰旧文件

### 📋 多功能
- **日志分级**：支持 Trace / Debug / Info / Warn / Error / Critical / Off 七个级别
- **编译期级别过滤**：通过宏定义在编译期裁剪不需要的日志级别，零运行时开销
- **多种 Sink 输出**：控制台、文件、mmap、protobuf 序列化、压缩、加密、异步等多种输出方式
- **格式化输出**：基于 fmt 库的高性能格式化，支持自定义格式模板
- **Protobuf 序列化**：结构化日志存储，便于后续分析处理

### 🌍 跨平台兼容
- 基于 CMake 构建，适配 **Linux / Windows / macOS** 三大主流操作系统
- 使用 vcpkg 管理第三方依赖，一键安装

---

## 🏗️ 架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                      用户接口层                               │
│   LOG_INFO / LOG_DEBUG / LOG_ERROR ... (宏接口)              │
│   LogHandle / VariadicLogHandle (处理器)                     │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│                      Sink 输出层                              │
│  ┌──────────┐ ┌──────────┐ ┌────────────┐ ┌─────────────┐  │
│  │ConsoleSink│ │SimpleSink│ │ MMapSink   │ │ProtobufSink │  │
│  └──────────┘ └──────────┘ └────────────┘ └─────────────┘  │
│  ┌────────────┐ ┌──────────┐ ┌──────────┐ ┌─────────────┐  │
│  │CompressSink│ │CryptoSink│ │ AsyncSink│ │EffectiveSink│  │
│  └────────────┘ └──────────┘ └──────────┘ └─────────────┘  │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│                      功能模块层                               │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌───────────────┐  │
│  │ Compress │ │  Crypto  │ │   MMap   │ │  Scheduler    │  │
│  │zstd/zlib │ │ECDH+AES  │ │内存映射   │ │线程池+定时器   │  │
│  └──────────┘ └──────────┘ └──────────┘ └───────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

---

## 📁 项目目录结构

```
flashlog/
├── CMakeLists.txt              # 顶层构建配置
├── CMakePresets.json            # CMake 预设配置
├── vcpkg.json                  # vcpkg 依赖声明
├── vcpkg-configuration.json    # vcpkg 注册表配置
│
├── logger/                     # 核心日志库
│   ├── logger.h                # 便捷宏接口（LOG_INFO/LOG_DEBUG 等）
│   ├── log_common.h            # 通用定义（日志级别/源码位置）
│   ├── log_msg.h               # 日志消息结构体
│   ├── internal_log.h          # 内部调试日志
│   ├── scope_guard.h           # RAII 资源管理
│   ├── space.h                 # 存储容量类（B/KB/MB/GB/TB）
│   ├── compress/               # 压缩模块
│   │   ├── compress.h          #   抽象接口
│   │   ├── zstd_compress.*     #   Zstandard 实现
│   │   └── zlib_compress.*     #   Zlib 实现
│   ├── crypto/                 # 加密模块
│   │   ├── crypto.*            #   ECDH 密钥协商
│   │   └── aes_crypto.*        #   AES-CBC 加解密
│   ├── formatter/              # 日志格式化器
│   │   ├── formatter.h         #   抽象接口
│   │   ├── default_formatter.* #   默认格式化器
│   │   └── effective_formatter.* # Protobuf 格式化器
│   ├── handler/                # 日志处理器
│   │   ├── log_handle.*        #   基础处理器（多 Sink 分发）
│   │   ├── log_variadic_handle.h # 可变参数处理器
│   │   └── log_factory.h       #   全局单例工厂
│   ├── mmap/                   # 内存映射模块
│   │   ├── mmap_aux.*          #   mmap 辅助类
│   │   ├── mmap_aux_linux.cc   #   Linux 平台实现
│   │   ├── mmap_aux_win.cc     #   Windows 平台实现
│   │   └── memory_buffer.*     #   内存缓冲区
│   ├── scheduler/              # 异步任务调度器
│   │   ├── scheduler.*         #   调度器（含定时器）
│   │   └── thread_pool.*       #   线程池
│   ├── sinks/                  # 日志输出端
│   │   ├── sink.h              #   Sink 抽象基类
│   │   ├── console_sink.*      #   控制台输出
│   │   ├── simple_sink.*       #   简单文件输出
│   │   ├── mmap_sink.*         #   mmap 文件输出
│   │   ├── protobuf_sink.*     #   Protobuf 序列化输出
│   │   ├── compress_sink.*     #   压缩输出
│   │   ├── crypto_sink.*       #   加密输出
│   │   ├── async_sink.*        #   异步输出
│   │   └── effective_sink.*    #   综合高性能输出（推荐）
│   ├── proto/                  # Protobuf 定义
│   │   ├── effective_msg.proto #   日志消息协议
│   │   └── person.proto        #   示例协议
│   └── utils/                  # 工具类
│       ├── file_util.*         #   文件操作
│       ├── sys_util.*          #   系统信息（跨平台）
│       └── timer_count.h       #   计时器
│
├── decode/                     # 日志解码工具
│   ├── decode.cc               # 解码主程序
│   ├── decode_formatter.*      # 解码格式化器
│   └── CMakeLists.txt
│
├── example/                    # 使用示例
│   ├── logger_example.cc       # 日志写入示例
│   ├── crypto_example.cc       # 加密模块示例
│   ├── zstd_example.cc         # 压缩模块示例
│   └── CMakeLists.txt
│
├── src/                        # 单元测试
│   ├── gtest_mmap.cc           # mmap 测试
│   ├── gtest_space.cc          # space 容量类测试
│   ├── gtest_third_party.cc    # 第三方库测试
│   └── gtest_threadpool.cc     # 线程池测试
│
└── script/                     # 构建脚本
    ├── build_mac.py            # macOS 构建脚本
    ├── build_linux.py          # Linux 构建脚本
    ├── build_windows.py        # Windows 构建脚本
    └── bin/                    # protoc 编译器（各平台）
        ├── protoc_mac
        ├── protoc_linux
        └── protoc_win.exe
```

---

## 🔧 环境要求

| 依赖项 | 最低版本 | 说明 |
|--------|---------|------|
| C++ 编译器 | C++20 | GCC 10+ / Clang 12+ / MSVC 2022+ |
| CMake | 3.10+ | 构建系统 |
| vcpkg | 最新版 | 包管理器 |
| Python | 3.x | 构建脚本 |

### 第三方库依赖（通过 vcpkg 自动管理）

| 库名 | 用途 |
|------|------|
| **fmt** | 高性能格式化 |
| **zstd** | Zstandard 压缩算法 |
| **zlib** | Zlib 压缩算法 |
| **cryptopp** | 加密算法（AES/ECDH） |
| **protobuf** | 结构化序列化 |
| **gtest** | 单元测试框架 |

---

## 🚀 快速开始

### 1. 安装 vcpkg

```bash
# 克隆 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Linux/macOS
./bootstrap-vcpkg.sh

# Windows
.\bootstrap-vcpkg.bat

# 设置环境变量
export VCPKG_ROOT=/path/to/vcpkg
```

### 2. 构建项目

#### macOS

```bash
python3 script/build_mac.py --config Debug
```

可选参数：
- `--config Debug|Release`：编译模式（默认 Debug）
- `--test`：启用单元测试
- `--clang`：生成 compile_commands.json（用于 clang-tidy）

#### Linux

```bash
python3 script/build_linux.py --test
```

#### Windows

```bash
python script/build_windows.py --test
```

> Windows 构建需要 Visual Studio 2022（v143 工具集）。

### 3. 运行示例

```bash
# 运行日志写入示例
./bin/logger_example

# 运行单元测试
./bin/flashlog
```

---

## 📝 使用指南

### 基本用法 —— 控制台输出

```cpp
#include "sinks/console_sink.h"
#include "handler/log_variadic_handle.h"
#include "logger.h"

int main() {
    // 创建控制台 Sink
    auto sink = std::make_shared<logger::ConsoleSink>();
    auto handler = std::make_shared<logger::VariadicLogHandle>(sink);

    // 注册为全局日志处理器
    LOGGER_INIT(handler);

    // 使用全局宏接口写日志
    LOG_TRACE("这是 trace 日志");
    LOG_DEBUG("调试信息: x = {}", 42);
    LOG_INFO("程序启动成功");
    LOG_WARN("警告: 内存使用率 {}%", 85);
    LOG_ERROR("错误: 文件 {} 不存在", "config.json");
    LOG_CRITICAL("致命错误: 系统崩溃");

    return 0;
}
```

### 高性能用法 —— EffectiveSink（推荐）

`EffectiveSink` 是功能最完整的 Sink，集成了 mmap + 压缩 + 加密 + 异步 + 分片淘汰：

```cpp
#include "sinks/effective_sink.h"
#include "handler/log_handle.h"
#include "logger.h"

int main() {
    // 配置参数
    logger::detail::Conf conf;
    conf.file_dir = "/path/to/log/dir";       // 日志输出目录
    conf.file_name = "myapp";                  // 日志文件名前缀
    conf.single_size = logger::space::MB(4);   // 单文件最大 4MB
    conf.total_size = logger::space::MB(100);  // 总容量上限 100MB
    conf.interval = std::chrono::minutes(5);   // 刷盘间隔
    conf.server_pub_key = "04...";             // 服务端 ECDH 公钥（Hex）

    // 创建 EffectiveSink
    auto sink = std::make_shared<logger::EffectiveSink>(conf);
    logger::LogHandle handle({sink});

    // 写入日志
    handle.Log(logger::LogLevel::kInfo, logger::LogSourceLoc(), "Hello FlashLog!");

    // 刷盘
    handle.Flush();

    return 0;
}
```

### 多 Sink 组合

```cpp
#include "sinks/console_sink.h"
#include "sinks/effective_sink.h"
#include "handler/log_handle.h"

int main() {
    auto console_sink = std::make_shared<logger::ConsoleSink>();
    auto file_sink = std::make_shared<logger::EffectiveSink>(conf);

    // 同时输出到控制台和文件
    logger::LogHandle handle({console_sink, file_sink});
    handle.Log(logger::LogLevel::kInfo, logger::LogSourceLoc(), "双输出日志");

    return 0;
}
```

### 异步任务调度器

FlashLog 内置了功能完善的任务调度器，支持延迟任务和重复任务：

```cpp
#include "scheduler/scheduler.h"

// 创建执行器
auto tag = NEW_STRAND_EXECUTOR(1001);

// 提交普通任务
POST_TASK(tag, [] { /* 异步执行的任务 */ });

// 提交重复任务（每 5 秒执行一次，共执行 10 次）
POST_REPEATED_TASK(tag, std::chrono::seconds(5), 10, [] {
    // 定时执行的任务
});

// 等待所有任务完成
WAIT_TASKS_COMPLETED(tag);
```

---

## 🔐 日志加密与解密

### 加密流程

FlashLog 采用 **ECDH + AES-CBC** 混合加密方案：

1. 客户端生成临时 ECDH 密钥对
2. 使用客户端私钥 + 服务端公钥协商出共享密钥（Shared Key）
3. 共享密钥作为 AES-CBC 的加密密钥
4. 每个 Chunk 使用独立的 IV 和密钥对

### 解密日志

使用 `decode` 工具解密并解析加密日志：

```bash
# 命令行方式
./bin/decoder <加密日志文件路径> <服务端私钥Hex> <输出文件路径>

# 示例
./bin/decoder ./test/myapp_20260101120000.log \
    FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E \
    ./test/output.txt
```

### 解码格式模板

解码器支持自定义输出格式，模板占位符：

| 占位符 | 含义 |
|--------|------|
| `%l` | 日志级别 |
| `%D` | 日期 |
| `%S` | 时间戳 |
| `%p` | 进程 ID |
| `%t` | 线程 ID |
| `%F` | 文件名 |
| `%f` | 函数名 |
| `%#` | 行号 |
| `%v` | 日志内容 |

示例格式：`[%l][%D:%S][%p:%t][%F:%f:%#]%v`

---

## 🧩 Sink 类型说明

| Sink | 说明 | 适用场景 |
|------|------|---------|
| `ConsoleSink` | 控制台输出 | 开发调试 |
| `SimpleSink` | 简单文件写入 | 轻量级日志 |
| `MMapSink` | mmap 内存映射写入 | 高性能、防崩溃 |
| `ProtobufSink` | Protobuf 序列化写入 | 结构化日志 |
| `CompressSink` | 压缩写入 | 节省磁盘空间 |
| `CryptoSink` | 加密写入 | 敏感数据保护 |
| `AsyncSink` | 异步写入 | 降低主线程开销 |
| `EffectiveSink` | 综合方案（mmap+压缩+加密+异步+分片） | **生产环境推荐** |

---

## ⚙️ 配置参数

`logger::detail::Conf` 结构体字段说明：

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `file_dir` | `filesystem::path` | - | 日志文件输出目录 |
| `file_name` | `string` | - | 日志文件名前缀 |
| `server_pub_key` | `string` | - | 服务端 ECDH 公钥（Hex 格式） |
| `interval` | `chrono::minutes` | 5 | 日志刷盘间隔 |
| `single_size` | `space::MB` | 4 | 单个日志文件最大大小 |
| `total_size` | `space::MB` | 100 | 日志文件总容量上限 |

---

## 🧪 单元测试

项目使用 Google Test 框架，测试文件位于 `src/` 目录：

```bash
# 构建并运行测试
python3 script/build_mac.py --config Debug --test
./bin/flashlog
```

测试覆盖：
- `gtest_mmap.cc` —— mmap 内存映射功能
- `gtest_space.cc` —— 存储容量类运算
- `gtest_third_party.cc` —— 第三方库集成
- `gtest_threadpool.cc` —— 线程池调度

---

## 📊 性能基准

在 `logger_example.cc` 中的基准测试：
- **测试条件**：50 万条日志，每条 2000 字节
- **Sink 类型**：EffectiveSink（mmap + zstd 压缩 + AES 加密 + 异步写入）
- **测试环境**：macOS / Apple Silicon

> 具体性能数据取决于硬件环境，可运行 `bin/logger_example` 查看实际耗时。

---

## 📄 日志文件格式

### 文件结构

```
┌─────────────────────────────────────┐
│           Chunk Header              │
│  magic(8B) | size(8B) | iv(32B)     │
│  pub_key(128B)                      │
├─────────────────────────────────────┤
│           Item Header               │
│  magic(4B) | size(4B)               │
├─────────────────────────────────────┤
│     Encrypted + Compressed Data     │
│  (AES-CBC 加密的 zstd 压缩数据)      │
├─────────────────────────────────────┤
│           Item Header               │
│           ...                       │
├─────────────────────────────────────┤
│           Chunk Header              │
│           ...                       │
└─────────────────────────────────────┘
```

- **Chunk**：一个加密单元，包含独立的 IV 和客户端公钥
- **Item**：一条日志记录，经过 Protobuf 序列化 → zstd 压缩 → AES 加密

---

## 📜 License

本项目仅供学习和内部使用。
