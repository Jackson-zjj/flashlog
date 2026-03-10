#pragma once

#include "sink.h"

#include <mutex>
#include <filesystem>
#include <fstream>

#include "space.h"
#include "mmap/memory_buffer.h"
#include "compress/compress.h"

namespace logger {

// 压缩输出器：缓冲区->压缩->同步持久化（探究压缩效率）
class CompressSink : public LogSink {
public:
    explicit CompressSink(detail::Conf conf);

    ~CompressSink() override;

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
    std::string compress_buf_;

    detail::Conf conf_;
    std::filesystem::path file_path_;
    std::unique_ptr<Formatter> formatter_;
    std::unique_ptr<compress::Compression> compress_;
    MemoryBuffer cache_;
};  // class CompressSink

}// namespace logger