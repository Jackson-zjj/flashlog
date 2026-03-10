#pragma once

#include <filesystem>

#include "log_msg.h"
#include "formatter/formatter.h"
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
    char iv[32];
    char pub_key[128];

    ChunkHeader() : magic(kMagic), size(0) {}
};

struct Conf {
    std::filesystem::path file_dir;
    std::string file_name;
    std::string server_pub_key;
    std::chrono::minutes interval{5};
    space::MB single_size{4};   // 单个日志文件大小
    space::MB total_size{100};  // 总大小
};

} // namespace detail

/// @brief 日志接收器
class LogSink {
public:
    LogSink() = default;

    LogSink(const LogSink& other) = delete;

    LogSink& operator=(const LogSink& other) = delete;

    virtual ~LogSink() = default;

    virtual void Log(const LogMsg&) = 0;

    virtual void SetFormatter(std::unique_ptr<Formatter> formatter) = 0;

    virtual void Flush() {}

};  // class sink

}   // namespace logger