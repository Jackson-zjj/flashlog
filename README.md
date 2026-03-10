## 闪存日志


### 项目描述
一个高性能、可扩展的日志组件，专为高并发、高可靠性场景设计。提供格式化、压缩、加密、mmap、日志消费等功能模块。

### 核心特性
#### 高性能
+ 基于mmap内存映射的日志写入（零拷贝），大幅降低磁盘 IO 开销；
+ 异步日志调度机制，避免阻塞业务线程；
+ 内置内存池管理，减少高频日志场景下的内存分配损耗。

#### 安全与可靠
+ 日志加密：满足敏感场景的日志安全需求；

#### 跨平台兼容
+ 基于 CMake 构建，适配 Linux/Windows/macOS 等主流操作系统；




### 目录结构
```
logger文件夹

├── logger.h                // 便捷接口
├── internal_log.h          // 内部日志组件
├── log_common.h            // 通用定义（日志级别/错误码/宏）
├── log_msg.h               // 日志消息结构体
├── scope_guard.h           // RAII资源管理（自动释放资源）
├── space.h                 // 存储容量类（KB/MB/GB）
├── compress/               // 压缩模块
├── crypto/                 // 加密模块
├── formatter/              // 日志格式化器
├── handler/                // 日志处理器
├── mmap/                   // mmap模块
├── proto/                  // 序列化模块
├── scheduler/              // 异步任务调度器
├── sinks/                  // 日志输出模块（文件/控制台/网络等）
├── utils/                  // 工具类（时间/线程/文件操作）
└── CMakeLists.txt          // 项目构建配置
```


### 使用方法
+ 日志写入使用示例：example/logger_example.cc
+ 日志解析工具：decode/decode.cc