## 闪存日志


### 项目描述
一个高性能、可扩展的日志组件，专为高并发、高可靠性场景设计。提供格式化、序列化、压缩、加密、mmap、日志消费等功能模块。

### 核心特性
#### 多功能
+ 支持日志分级
+ 具备日志分片、淘汰功能

#### 高性能
+ 基于mmap内存映射的日志写入（零拷贝），大幅降低磁盘 IO 开销，同时也能减少日志丢失率（防崩溃）；
+ 异步日志调度机制，避免阻塞业务线程；

#### 安全与可靠
+ 日志加密：满足敏感场景的日志安全需求；

#### 跨平台兼容
+ 基于 CMake 构建，适配 Linux/Windows/macOS 等主流操作系统；




### 项目核心目录结构
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
+ 导表+编译构建的自动化脚本：script/build_mac.py
+ 日志写入使用示例：example/logger_example.cc
+ 日志解析工具：decode/decode.cc

使用步骤：
1. python3 执行自动化脚本 script/build_mac.py（当前只有mac系统进行了适配）
2. 运行“日志写入示例”可执行文件（bin/logger_example）
3. 修改日志解析工具路径参数（decode/decode.cc；line 110），或改用控制台输入参数的方式
4. 再次执行自动化脚本
5. 运行“日志解析工具”可执行文件（bin/decoder）
6. 查看日志：路径（test/log.txt）