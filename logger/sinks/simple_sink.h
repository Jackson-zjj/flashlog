#pragma once

#include "sink.h"

#include <mutex>
#include <filesystem>
#include <fstream>

#include "space.h"
#include "mmap/memory_buffer.h"

namespace logger {

// 简易输出器：缓冲区->同步持久化（最基础功能）
class SimpleSink : public LogSink {
public:
    explicit SimpleSink(detail::Conf conf);

    ~SimpleSink() override;

    void Log(const LogMsg& log_msg) override;

    void SetFormatter(std::unique_ptr<Formatter> formatter) override;

    void Flush() override;

private:
    void WriteToCache_(const void* data, size_t size);

    bool NeedWriteToFile_();

    void CacheToFile_();

    std::filesystem::path GetFilePath_();

    std::mutex mtx_;
    std::condition_variable cv_;
    std::ofstream ofs_;

    detail::Conf conf_;
    std::filesystem::path file_path_;
    std::unique_ptr<Formatter> formatter_;
    MemoryBuffer cache_;
};  // class SimpleSink

}// namespace logger