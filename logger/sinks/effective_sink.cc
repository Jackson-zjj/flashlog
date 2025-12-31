#include "effective_sink.h"

#include <chrono>
#include <vector>

#include <fmt/core.h>
#include <fmt/chrono.h>

#include "formatter/effective_formatter.h"
#include "compress/zstd_compress.h"
#include "crypto/aes_crypto.h"
#include "utils/file_util.h"
#include "internal_log.h"
#include "utils/timer_count.h"
#include "utils/sys_util.h"

namespace logger {

constexpr char kFileSuffix[] = ".log";

EffectiveSink::EffectiveSink(Conf conf) : conf_(std::move(conf)) {
    INFO("EffectiveSink: file_dir={}, file_name={}, server_pub_key={}, interval={}, single_size={}, total_size={}",
           conf_.file_dir.string(), conf_.file_name, conf_.server_pub_key, conf_.interval.count(), conf_.single_size.count(),
           conf_.total_size.count());

    if (!std::filesystem::exists(conf_.file_dir)) {
        std::filesystem::create_directories(conf_.file_dir);
    }
    executor_tag_ = NEW_STRAND_EXECUTOR(executor_tag_);
    formatter_ = std::make_unique<EffectiveFormatter>();
    compress_ = std::make_unique<compress::ZstdCompress>();
    auto keys = crypto::GenECDHKey();
    client_pri_key_ = std::get<0>(keys);
    client_pub_key_ = std::get<1>(keys);
    INFO("EffectiveSink: client pub size {}", client_pub_key_.size());
    auto server_pub_key = crypto::HexKeyToBinary(conf_.server_pub_key);
    auto shared_key = crypto::GenECDHSharedKey(client_pri_key_, server_pub_key);
    auto crypto_iv = crypto::AESCrypto::GenerateIV();
    crypto_ = std::make_unique<crypto::AESCrypto>(shared_key, crypto_iv);
    master_mmap_ = std::make_unique<MMapAux>(conf_.file_dir / "master_mmap");
    slave_mmap_ = std::make_unique<MMapAux>(conf_.file_dir / "slave_mmap");
    if (!master_mmap_ || !slave_mmap_) {
        throw std::runtime_error("EffectiveSink::EffectiveSink create mmap failed!");
    }

    if (!slave_mmap_->Empty()) {
        is_slave_free_.store(false);
        PrepareToFile_();
    }
    if (!master_mmap_->Empty()) {
        WAIT_TASKS_COMPLETED(executor_tag_);
        SwapCache_();
        PrepareToFile_();
    }

    // 淘汰策略
    POST_REPEATED_TASK(executor_tag_, conf_.interval, -1, [this] {
        EliminateFiles_();
    });
}

EffectiveSink::~EffectiveSink() {
    if (ofs_.is_open()) {
        ofs_.flush();
        ofs_.close();
    }
}

void EffectiveSink::Log(const LogMsg& msg) {
    static thread_local MemoryBuf format_buf;
    formatter_->Format(msg, &format_buf);

    if (master_mmap_->Empty()) {
        compress_->ResetStream();
    }

    {
        std::lock_guard<std::mutex> lock(mtx_);
        compress_buf_.reserve(compress_->CompressBound(format_buf.size()));
        size_t compress_size = compress_->Compress(format_buf.data(), format_buf.size(), compress_buf_.data(), compress_buf_.capacity());
        if (compress_size == 0) {
            // todo
            ERROR("EffectiveSink::Log: compress failed");
            return;
        }

        encrypt_buf_.clear();
        encrypt_buf_.reserve(compress_size + 10);
        crypto_->Encrypt(compress_buf_.data(), compress_size, encrypt_buf_);
        if (encrypt_buf_.empty()) {
            ERROR("EffectiveSink::Log: encrypt failed");
            return;
        }

        WriteToCache_(encrypt_buf_.data(), encrypt_buf_.size());
    }


    if (NeedWriteToFile_()) {
        SwapCache_();
        PrepareToFile_();
    }
}

void EffectiveSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
    formatter_ = std::move(formatter);
}

void EffectiveSink::Flush() {
    TIMER_COUNT("Flush");
    PrepareToFile_();
    WAIT_TASKS_COMPLETED(executor_tag_);

    SwapCache_();
    PrepareToFile_();
    WAIT_TASKS_COMPLETED(executor_tag_);
}


bool EffectiveSink::NeedWriteToFile_() {
    std::lock_guard<std::mutex> lock(mtx_);
    return master_mmap_->GetRatio() > 0.8;
}

void EffectiveSink::WriteToCache_(const void* data, size_t size) {
    detail::ItemHeader item_header;
    item_header.size = size;
    master_mmap_->Push(&item_header, sizeof(item_header));
    master_mmap_->Push(data, size);
}

void EffectiveSink::SwapCache_() {
    if (is_slave_free_.load()) {
        is_slave_free_.store(false);
        std::lock_guard<std::mutex> lock(mtx_);
        std::swap(master_mmap_, slave_mmap_);
    }
}

void EffectiveSink::PrepareToFile_() {
    POST_TASK(executor_tag_, [this]{
        CacheToFile_();
    });
}

void EffectiveSink::CacheToFile_() {
    TIMER_COUNT("CacheToFile_");
    if (is_slave_free_.load()) {
        return;
    }

    if (slave_mmap_->Empty()) {
        is_slave_free_.store(true);
        return;
    }
    if (slave_mmap_->Size() > space::space_cast<space::B>(conf_.single_size).count() * 1.5) {
        fmt::print("too big! \n");
    }
    
    detail::ChunkHeader chunk_header;
    chunk_header.size = slave_mmap_->Size();
    memcpy(chunk_header.pub_key, client_pub_key_.data(), client_pub_key_.size());
    if (auto aes_crypto_ptr = dynamic_cast<crypto::AESCrypto*>(crypto_.get())) {
        auto crypto_iv = aes_crypto_ptr->GetIV();
        memcpy(chunk_header.iv, crypto_iv.data(), crypto_iv.size());
    }
    
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
    ofs_.write(reinterpret_cast<char*>(slave_mmap_->Data()), chunk_header.size);

    slave_mmap_->Clear();
    is_slave_free_.store(true);
}

static std::filesystem::path GetDataTimePath(std::filesystem::path& file_dir, std::string& file_name) {
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    auto time = fmt::format("{:%Y%m%d%H%M%S}", now_sec);
    return file_dir / (file_name + "_" + time);
}

std::filesystem::path EffectiveSink::GetFilePath_() {
    if (file_path_.empty()) {
        file_path_ = GetDataTimePath(conf_.file_dir, conf_.file_name).string() + kFileSuffix;
    } else {
        auto file_size = fs::GetFileSize(file_path_);
        auto single_size = space::space_cast<space::B>(conf_.single_size);
        if (file_size > single_size.count()) {
            auto data_time_path = GetDataTimePath(conf_.file_dir, conf_.file_name);
            auto new_file_path = data_time_path.string() + kFileSuffix;
            if (std::filesystem::exists(new_file_path)) {
                int index = 0;
                for (const auto& file : std::filesystem::directory_iterator(conf_.file_dir)) {
                    if (file.path().filename().string().find(data_time_path.filename().string()) != std::string::npos) {
                        ++index;
                    }
                }
                new_file_path = data_time_path.string() + "_" + std::to_string(index) + kFileSuffix;
            }
            file_path_ = new_file_path;
        }
    }
    // file_path_.replace_extension(kFileSuffix);
    return file_path_;
}

void EffectiveSink::EliminateFiles_() {
    INFO("EffectiveSink::EliminateFiles_: start");
    std::vector<std::filesystem::path> files;
    for (const auto& file : std::filesystem::directory_iterator(conf_.file_dir)) {
        if (file.path().extension() == kFileSuffix) {
            files.push_back(file.path());
        }
    }
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
    });

    size_t cur_total_size = 0;
    size_t total_size = space::space_cast<space::B>(conf_.total_size).count();
    for (const auto& file : files) {
        if (cur_total_size < total_size) {
            cur_total_size += fs::GetFileSize(file);
        } else {
            INFO("EffectiveSink::EliminateFiles_: remove file = {}", file.string());
            std::filesystem::remove(file);
        }
    }
}

}   // namspace logger