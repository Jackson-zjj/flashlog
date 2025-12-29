#include "zstd_compress.h"

namespace logger {
namespace compress {

ZstdCompress::ZstdCompress() {
    cctx_ = ZSTD_createCStream();
    dctx_ = ZSTD_createDStream();

    ZSTD_CCtx_setParameter(cctx_, ZSTD_c_compressionLevel, 5);
}

ZstdCompress::~ZstdCompress() {
    if (cctx_) {
        ZSTD_freeCCtx(cctx_);
    }
    if (dctx_) {
        ZSTD_freeDCtx(dctx_);
    }
}

void ZstdCompress::ResetStream() {
    if (cctx_) {
        ZSTD_CCtx_reset(cctx_, ZSTD_reset_session_only);
    }
}

void ZstdCompress::ResetDecompressStream_() {
    if (dctx_) {
        ZSTD_DCtx_reset(dctx_, ZSTD_reset_session_only);
    }
}

size_t ZstdCompress::CompressBound(size_t input_size) {
  return ZSTD_compressBound(input_size);
}

static bool IsZSTDCompress(const void* data, size_t size) {
    if (!data) {
        return false;
    }
    if (size < 4) {
        return false;
    }

    const uint8_t kMagicNumberBigEndian[] = {0x28, 0xb5, 0x2f, 0xfd};
    const uint8_t kMagicNumberLittleEndian[] = {0xfd, 0x2f, 0xb5, 0x28};
    if (memcmp(data, kMagicNumberBigEndian, sizeof(kMagicNumberBigEndian)) == 0) {
        return true;
    }

    if (memcmp(data, kMagicNumberLittleEndian, sizeof(kMagicNumberLittleEndian)) == 0) {
        return true;
    }

    return false;
}

size_t ZstdCompress::Compress(const void* input, size_t input_size, void* output, size_t output_size) {
    if (!input || input_size == 0) {
        return 0;
    }
    ZSTD_inBuffer in_buffer = {input, input_size, 0};
    ZSTD_outBuffer out_buffer = {output, output_size, 0};

    size_t ret = ZSTD_compressStream2(cctx_, &out_buffer, &in_buffer, ZSTD_e_flush);
    if (ZSTD_isError(ret) != 0) {
        return 0;
    }
    return out_buffer.pos;
}

std::string ZstdCompress::Decompress(const void* data, size_t size) {
    if (!data || size == 0) {
        return "";
    }
    if (IsZSTDCompress(data, size)) {
        ResetDecompressStream_();
    }

    std::string output;
    output.reserve(1024 * 10);
    ZSTD_inBuffer in_buffer = {data, size, 0};
    ZSTD_outBuffer out_buffer = {reinterpret_cast<void*>(output.data()), output.capacity(), 0};
    
    size_t ret = ZSTD_decompressStream(dctx_, &out_buffer, &in_buffer);
    if (ZSTD_isError(ret) != 0) {
        return "";
    }
    output = std::string(reinterpret_cast<char*>(out_buffer.dst), out_buffer.pos);
    return output;
}
    
    
} // namespace compress
} // namespace logger

