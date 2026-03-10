#include "mmap_sink.h"

#include "internal_log.h"
#include "utils/file_util.h"
#include "utils/timer_count.h"
#include "formatter/default_formatter.h"

namespace logger {

constexpr char kFileSuffix[] = ".log";

MMapSink::MMapSink(detail::Conf conf) : conf_(std::move(conf)) {
    INFO("MMapSink: file_dir={}, file_name={}, single_size={}",
           conf_.file_dir.string(), conf_.file_name, conf_.single_size.count());

    if (!std::filesystem::exists(conf_.file_dir)) {
        std::filesystem::create_directories(conf_.file_dir);
    }
    
    formatter_ = std::make_unique<DefaultFormatter>();
    mmap_ = std::make_unique<MMapAux>(conf_.file_dir / "mmap");
    
    if (!mmap_) {
        throw std::runtime_error("MMapSink::MMapSink create mmap failed!");
    }
    if (!mmap_->Empty()) {
        CacheToFile_();
    }
}

MMapSink::~MMapSink() {
    if (ofs_.is_open()) {
        ofs_.flush();
        ofs_.close();
    }
}

void MMapSink::Log(const LogMsg& msg) {
    static thread_local MemoryBuf format_buf;
    format_buf.clear();
    formatter_->Format(msg, &format_buf);

    WriteToCache_(format_buf.data(), format_buf.size());

    if (NeedWriteToFile_()) {
        CacheToFile_();
    }
}

void MMapSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
    formatter_ = std::move(formatter);
}

void MMapSink::Flush() {
    TIMER_COUNT("MMapSink::Flush");
    CacheToFile_();
}

void MMapSink::WriteToCache_(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mtx_);
    detail::ItemHeader item_header;
    item_header.size = size;
    mmap_->Push(&item_header, sizeof(item_header));
    mmap_->Push(data, size);
}

bool MMapSink::NeedWriteToFile_() {
    std::lock_guard<std::mutex> lock(mtx_);
    return mmap_->GetRatio() > 0.8;
}

void MMapSink::CacheToFile_() {
    std::lock_guard<std::mutex> lock(mtx_);
    TIMER_COUNT("MMapSink::CacheToFile_");
    
    detail::ChunkHeader chunk_header;
    chunk_header.size = mmap_->Size();
    
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
    ofs_.write(reinterpret_cast<char*>(mmap_->Data()), chunk_header.size);

    mmap_->Clear();
}

static std::filesystem::path GetDataTimePath(std::filesystem::path& file_dir, std::string& file_name) {
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    auto time = fmt::format("{:%Y%m%d%H%M%S}", now_sec);
    return file_dir / (file_name + "_" + time);
}

std::filesystem::path MMapSink::GetFilePath_() {
    if (file_path_.empty()) {
        file_path_ = GetDataTimePath(conf_.file_dir, conf_.file_name).string() + "_mmap" + kFileSuffix;
    } else {
        auto file_size = fs::GetFileSize(file_path_);
        auto single_size = space::space_cast<space::B>(conf_.single_size);
        if (file_size > single_size.count()) {
            auto data_time_path = GetDataTimePath(conf_.file_dir, conf_.file_name);
            auto new_file_path = data_time_path.string() + "_mmap" + kFileSuffix;
            if (std::filesystem::exists(new_file_path)) {
                int index = 0;
                for (const auto& file : std::filesystem::directory_iterator(conf_.file_dir)) {
                    if (file.path().filename().string().find(data_time_path.filename().string()) != std::string::npos) {
                        ++index;
                    }
                }
                new_file_path = data_time_path.string() + "_mmap" + "_" + std::to_string(index) + kFileSuffix;
            }
            file_path_ = new_file_path;
        }
    }
    // file_path_.replace_extension(kFileSuffix);
    return file_path_;
}

}