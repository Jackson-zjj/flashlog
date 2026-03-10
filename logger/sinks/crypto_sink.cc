#include "crypto_sink.h"

#include "internal_log.h"
#include "utils/file_util.h"
#include "utils/timer_count.h"
#include "formatter/default_formatter.h"
#include "crypto/aes_crypto.h"

namespace logger {

constexpr char kFileSuffix[] = ".log";
constexpr size_t kDafaultCapacity = 512 * 1024;  // 512KB

CryptoSink::CryptoSink(detail::Conf conf) : conf_(std::move(conf)){
    INFO("CryptoSink: file_dir={}, file_name={}, single_size={}",
           conf_.file_dir.string(), conf_.file_name, conf_.single_size.count());

    if (!std::filesystem::exists(conf_.file_dir)) {
        std::filesystem::create_directories(conf_.file_dir);
    }

    cache_.Reserve(kDafaultCapacity);
    formatter_ = std::make_unique<DefaultFormatter>();
    auto keys = crypto::GenECDHKey();
    client_pri_key_ = std::get<0>(keys);
    client_pub_key_ = std::get<1>(keys);
    auto server_pub_key = crypto::HexKeyToBinary(conf_.server_pub_key);
    auto shared_key = crypto::GenECDHSharedKey(client_pri_key_, server_pub_key);
    auto crypto_iv = crypto::AESCrypto::GenerateIV();
    crypto_ = std::make_unique<crypto::AESCrypto>(shared_key, crypto_iv);
}

CryptoSink::~CryptoSink() {
    if (ofs_.is_open()) {
        ofs_.flush();
        ofs_.close();
    }
}

void CryptoSink::Log(const LogMsg& msg) {
    static thread_local MemoryBuf format_buf;
    format_buf.clear();
    formatter_->Format(msg, &format_buf);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        encrypt_buf_.clear();
        encrypt_buf_.reserve(format_buf.size());
        crypto_->Encrypt(format_buf.data(), format_buf.size(), encrypt_buf_);
        if (encrypt_buf_.empty()) {
            ERROR("EffectiveSink::Log: encrypt failed");
            return;
        }

        WriteToCache_(encrypt_buf_.data(), encrypt_buf_.size());
    }

    if (NeedWriteToFile_()) {
        CacheToFile_();
    }
}

void CryptoSink::SetFormatter(std::unique_ptr<Formatter> formatter) {
    formatter_ = std::move(formatter);
}

void CryptoSink::Flush() {
    TIMER_COUNT("CryptoSink::Flush");
    CacheToFile_();
}

void CryptoSink::WriteToCache_(const void* data, size_t size) {
    detail::ItemHeader item_header;
    item_header.size = size;
    cache_.Push(&item_header, sizeof(item_header));
    cache_.Push(data, size);
}

bool CryptoSink::NeedWriteToFile_() {
    std::lock_guard<std::mutex> lock(mtx_);
    return cache_.GetRatio() > 0.8;
}

void CryptoSink::CacheToFile_() {
    std::lock_guard<std::mutex> lock(mtx_);
    TIMER_COUNT("CryptoSink::CacheToFile_");
    
    detail::ChunkHeader chunk_header;
    chunk_header.size = cache_.Size();

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
    ofs_.write(reinterpret_cast<char*>(cache_.Data()), chunk_header.size);

    cache_.Clear();
}

static std::filesystem::path GetDataTimePath(std::filesystem::path& file_dir, std::string& file_name) {
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    auto time = fmt::format("{:%Y%m%d%H%M%S}", now_sec);
    return file_dir / (file_name + "_" + time);
}

std::filesystem::path CryptoSink::GetFilePath_() {
    constexpr char kSinkFlag[] = "_crypto";
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