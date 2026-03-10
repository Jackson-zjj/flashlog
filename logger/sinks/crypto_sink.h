#pragma once

#include "sink.h"

#include <mutex>
#include <filesystem>
#include <fstream>

#include "space.h"
#include "mmap/memory_buffer.h"
#include "crypto/crypto.h"

namespace logger {

// 加密输出器：缓冲区->加密->同步持久化（探究加密对写入效率的影响）
class CryptoSink : public LogSink {
public:
    explicit CryptoSink(detail::Conf conf);

    ~CryptoSink() override;

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
    std::string client_pub_key_;
    std::string client_pri_key_;
    std::string encrypt_buf_;

    detail::Conf conf_;
    std::filesystem::path file_path_;
    std::unique_ptr<Formatter> formatter_;
    std::unique_ptr<crypto::Crypto> crypto_;
    MemoryBuffer cache_;
};  // class CryptoSink

}// namespace logger