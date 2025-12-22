#pragma once

#include "compress.h"

#include "zstd.h"

namespace logger {
namespace compress {
    
class ZstdCompress final : public Compression {
public:
    ZstdCompress();

    ~ZstdCompress() override;

    size_t CompressBound(size_t input_size) override;

    size_t Compress(const void* input, size_t input_size, void* output, size_t output_size) override;

    std::string Decompress(const void* data, size_t size) override;

    void ResetStream() override;

private:
    void ResetDecompressStream_();

    // 需要通过接口获取和释放流对象
    ZSTD_CCtx* cctx_;   // 压缩流对象
    ZSTD_DCtx* dctx_;   // 解压流对象
};  // class ZstdCompress

} // namespace compress
} // namespace logger

