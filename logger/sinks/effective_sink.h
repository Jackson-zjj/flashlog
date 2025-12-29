#pragma once

#include <mutex>
#include <atomic>
#include <filesystem>

#include "mmap/mmap_aux.h"
#include "compress/compress.h"
#include "crypto/crypto.h"
#include "scheduler/scheduler.h"
#include "sink.h"
#include "space.h"

namespace logger {

namespace detail {
struct ItemHeader {
    static constexpr uint32_t kMagic = 0xbe5fba11;
    uint32_t magic;
    uint32_t size;

    ItemHeader() : magic(kMagic), size(0) {}
};

struct ChunkHeader {
    static constexpr uint64_t kMagic = 0xdeadbeefdada1100;
    uint64_t magic;
    uint64_t size;
    char pub_key[128];

    ChunkHeader() : magic(kMagic), size(0) {}
};

} // namespace detail
    

/// @brief 高性能日志接收器
class EffectiveSink final : public LogSink {
public:
    struct Conf {
        std::filesystem::path file_dir;
        std::string file_name;
        std::string server_pub_key;
        std::chrono::minutes interval{5};
        space::MB single_size{4};   // 单个日志文件大小
        space::MB total_size{100};  // 总大小
    };

    explicit EffectiveSink(Conf conf);

    ~EffectiveSink() override = default;

    void Log(const LogMsg& msg) override;

    void SetFormatter(std::unique_ptr<Formatter> formatter) override;

    void Flush() override;

private:
    bool NeedWriteToFile_();

    void WriteToCache_(const void* data, size_t size);

    void SwapCache_();

    void PrepareToFile_();

    void CacheToFile_();

    std::filesystem::path GetFilePath_();

    void EliminateFiles_();

    Conf conf_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::filesystem::path file_path_;
    std::string client_pub_key_;
    std::string client_pri_key_;
    std::string compress_buf_;
    std::string encrypt_buf_;
    ExecutorTag executor_tag_{"SingleChannelDisk"};

    std::unique_ptr<Formatter> formatter_;
    std::unique_ptr<compress::Compression> compress_;
    std::unique_ptr<crypto::Crypto> crypto_;
    std::unique_ptr<MMapAux> master_mmap_;
    std::unique_ptr<MMapAux> slave_mmap_;
    std::atomic<bool> is_slave_free_{true};
    
};  // class EffectiveSink

} // namespace logger
