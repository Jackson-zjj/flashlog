#pragma once

#include "sink.h"

#include <mutex>
#include <filesystem>
#include <fstream>

#include "space.h"
#include "mmap/memory_buffer.h"

namespace logger {

// protobuf输出器：序列化->缓冲区->同步持久化（探究序列化提升的性能效率）
class ProtobufSink : public LogSink {
public:
    explicit ProtobufSink(detail::Conf conf);

    ~ProtobufSink() override;

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
};  // class ProtobufSink

}// namespace logger

