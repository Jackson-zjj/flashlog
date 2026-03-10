#pragma once

#include "sink.h"

#include <mutex>
#include <atomic>
#include <filesystem>
#include <fstream>

#include "space.h"
#include "mmap/memory_buffer.h"
#include "scheduler/scheduler.h"

namespace logger {

// 异步输出器：缓冲区->异步持久化（探究异步提高的写入效率）
class AsyncSink : public LogSink {
public:
    explicit AsyncSink(detail::Conf conf);

    ~AsyncSink() override;

    void Log(const LogMsg& log_msg) override;

    void SetFormatter(std::unique_ptr<Formatter> formatter) override;

    void Flush() override;

private:
    void WriteToCache_(const void* data, size_t size);

    bool NeedWriteToFile_();

    void SwapCache_();

    void AsyncCacheToFile_();

    void CacheToFile_();

    std::filesystem::path GetFilePath_();

    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> is_slave_free_{true};
    std::ofstream ofs_;

    detail::Conf conf_;
    ExecutorTag executor_tag_{10000};
    std::filesystem::path file_path_;
    std::unique_ptr<Formatter> formatter_;
    std::unique_ptr<MemoryBuffer> master_cache_;
    std::unique_ptr<MemoryBuffer> slave_cache_;
};  // class AsyncSink

}// namespace logger