#pragma once

#include "compress.h"

#include "zlib.h"

namespace logger {
namespace compress {

/// @brief zlib压缩流删除器
struct ZStreamDeflateDeleter {
    void operator()(z_stream* stream) {
        if (stream) {
            deflateEnd(stream);
            delete stream;
        }
    }
};

/// @brief zlib解压流删除器
struct ZStreamInflateDeleter {
    void operator()(z_stream* stream) {
        inflateEnd(stream);
        delete stream;
    }
};

/// @brief zlib压缩类
class ZlibCompress final : public Compression {
public:
    ~ZlibCompress() override = default;

    size_t CompressBound(size_t input_size) override;

    size_t Compress(const void* input, size_t input_size, void* output, size_t output_size) override;

    std::string Decompress(const void* data, size_t size) override;

    void ResetStream() override;

private:
    void ResetDecompressStream_();

    std::unique_ptr<z_stream, ZStreamDeflateDeleter> deflate_stream_;  // 压缩流
    std::unique_ptr<z_stream, ZStreamInflateDeleter> inflate_stream_;  // 解压流

};  // class ZlibCompress
    
} // namespace compress
} // namespace logger
