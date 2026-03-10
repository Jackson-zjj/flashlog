#include "compress_sink.h"

#include "internal_log.h"
#include "utils/file_util.h"
#include "utils/timer_count.h"
#include "formatter/default_formatter.h"
#include "compress/zstd_compress.h"

namespace logger {

constexpr char kFileSuffix[] = ".log";
constexpr size_t kDafaultCapacity = 512 * 1024;  // 512KB

CompressSink::CompressSink(detail::Conf conf) : conf_(std::move(conf)){
    INFO("CompressSink: file_dir={}, file_name={}, single_size={}",
           conf_.file_dir.string(), conf_.file_name, conf_.single_size.count());

    if (!std::filesystem::exists(conf_.file_dir)) {
        std::filesystem::create_directories(conf_.file_dir);
    }

    cache_.Reserve(kDafaultCapacity);
    formatter_ = std::make_unique<DefaultFormatter>();
    compress_ = std::make_unique<compress::ZstdCompress>();
    compress_->ResetStream();
}

CompressSink::~CompressSink() {
    if (ofs_.is_open()) {
        ofs_.flush();
        ofs_.close();
    }
}

void CompressSink::Log(const LogMsg& msg) {
    static thread_local MemoryBuf format_buf;
    format_buf.clear();
    formatter_->Format(msg, &format_buf);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        compress_buf_.reserve(compress_->CompressBound(format_buf.size()));
        size_t compress_size = compress_->Compress(format_buf.data(), format_buf.size(), compress_buf_.data(), compress_buf_.capacity());
        if (compress_size == 0) {
            ERROR("CompressSink::Log: compress failed");
            return;
        }
    }

    WriteToCache_(compress_buf_.data(), compress_buf_.size());
    if (NeedWriteToFile_()) {
        CacheToFile_();
    }
}

void CompressSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
    formatter_ = std::move(formatter);
}

void CompressSink::Flush() {
    TIMER_COUNT("CompressSink::Flush");
    CacheToFile_();
}

void CompressSink::WriteToCache_(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mtx_);
    detail::ItemHeader item_header;
    item_header.size = size;
    cache_.Push(&item_header, sizeof(item_header));
    cache_.Push(data, size);
}

bool CompressSink::NeedWriteToFile_() {
    std::lock_guard<std::mutex> lock(mtx_);
    return cache_.GetRatio() > 0.8;
}

void CompressSink::CacheToFile_() {
    std::lock_guard<std::mutex> lock(mtx_);
    TIMER_COUNT("CompressSink::CacheToFile_");
    
    detail::ChunkHeader chunk_header;
    chunk_header.size = cache_.Size();
    
    auto cache_path = file_path_;
    auto file_path = GetFilePath_();
    if (!ofs_.is_open()) {
        ofs_.open(file_path, std::ios::binary | std::ios::app);
    } else {
        if (cache_path != file_path) {
            ofs_.close();
            ofs_.open(file_path, std::ios::binary | std::ios::app);
        }
    }
    ofs_.write(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
    ofs_.write(reinterpret_cast<char*>(cache_.Data()), chunk_header.size);

    cache_.Clear();
    compress_->ResetStream();
}

static std::filesystem::path GetDataTimePath(std::filesystem::path& file_dir, std::string& file_name) {
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    auto time = fmt::format("{:%Y%m%d%H%M%S}", now_sec);
    return file_dir / (file_name + "_" + time);
}

std::filesystem::path CompressSink::GetFilePath_() {
    constexpr char kSinkFlag[] = "_compress";
    if (file_path_.empty()) {
        file_path_ = GetDataTimePath(conf_.file_dir, conf_.file_name).string() + kSinkFlag + kFileSuffix;
    } else {
        auto file_size = fs::GetFileSize(file_path_);
        auto single_size = space::space_cast<space::B>(conf_.single_size);
        if (file_size > single_size.count()) {
            auto data_time_path = GetDataTimePath(conf_.file_dir, conf_.file_name);
            auto new_file_path = data_time_path.string() + kSinkFlag + kFileSuffix;
            if (std::filesystem::exists(new_file_path)) {
                int index = 0;
                for (const auto& file : std::filesystem::directory_iterator(conf_.file_dir)) {
                    if (file.path().filename().string().find(data_time_path.filename().string()) != std::string::npos) {
                        ++index;
                    }
                }
                new_file_path = data_time_path.string() + kSinkFlag + "_" + std::to_string(index) + kFileSuffix;
            }
            file_path_ = new_file_path;
        }
    }
    // file_path_.replace_extension(kFileSuffix);
    return file_path_;
}

}