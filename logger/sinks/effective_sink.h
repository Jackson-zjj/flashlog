#pragma once

#include <mutex>
#include <atomic>
#include <filesystem>
#include <fstream>

#include "mmap/mmap_aux.h"
#include "compress/compress.h"
#include "crypto/crypto.h"
#include "scheduler/scheduler.h"
#include "sink.h"
#include "space.h"

namespace logger {

/// @brief 高性能日志接收器
class EffectiveSink final : public LogSink {
public:

    explicit EffectiveSink(detail::Conf conf);

    ~EffectiveSink() override;

    void Log(const LogMsg& msg) override;

    void SetFormatter(std::unique_ptr<Formatter> formatter) override;

    void Flush() override;

private:
    bool NeedWriteToFile_();

    void WriteToCache_(const void* data, size_t size);

    void SwapCache_();

    void AsyncCacheToFile_();

    void CacheToFile_();

    std::filesystem::path GetFilePath_();

    void EliminateFiles_();

    detail::Conf conf_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::filesystem::path file_path_;
    std::ofstream ofs_;
    std::string client_pub_key_;
    std::string client_pri_key_;
    std::string compress_buf_;
    std::string encrypt_buf_;
    ExecutorTag executor_tag_{10086};

    std::unique_ptr<Formatter> formatter_;
    std::unique_ptr<compress::Compression> compress_;
    std::unique_ptr<crypto::Crypto> crypto_;
    std::unique_ptr<MMapAux> master_mmap_;
    std::unique_ptr<MMapAux> slave_mmap_;
    std::atomic<bool> is_slave_free_{true};
    
};  // class EffectiveSink

} // namespace logger
