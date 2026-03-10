#pragma once

#include "sink.h"

#include <mutex>
#include <filesystem>
#include <fstream>

#include "space.h"
#include "mmap/mmap_aux.h"

namespace logger {

// mmap输出器：mmap缓冲区->同步持久化（探究mmap与缓冲区的性能差异）
class MMapSink : public LogSink {
public:

    explicit MMapSink(detail::Conf conf);

    ~MMapSink() override;

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
    std::unique_ptr<MMapAux> mmap_;
};  // class MMapSink

}// namespace logger

