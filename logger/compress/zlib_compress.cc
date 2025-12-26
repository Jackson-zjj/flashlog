#include "zlib_compress.h"

namespace logger {
namespace compress {

static bool IsCompressed(const void* data, size_t size) {
    if (!data) {
        return false;
    }
    if (size < 2) {
        return false;
    }

    uint16_t magic = *(uint16_t*)data;
    if (magic == 0x9c78 || magic == 0xda78 || magic == 0x5e78 || magic == 0x0178) {
    return true;
    }

    return false;
}

size_t ZlibCompress::CompressBound(size_t input_size) {
    return compressBound(input_size);
}

size_t ZlibCompress::Compress(const void* input, size_t input_size, void* output, size_t output_size) {
    if (!input || !output) {
        return 0;
    }
    if (!deflate_stream_) {
        return 0;
    }
    
    deflate_stream_->next_in = (Bytef*)input;
    deflate_stream_->avail_in = static_cast<uInt>(input_size);
    deflate_stream_->next_out = (Bytef*)output;
    deflate_stream_->avail_out = static_cast<uInt>(output_size);

    int ret = Z_OK;
    do {
        ret = deflate(deflate_stream_.get(), Z_SYNC_FLUSH);
        if (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_STREAM_END) {
            return 0;
        }
    }while (ret == Z_BUF_ERROR);
    // Z_SYNC_FLUSH为同步刷新策略，适用于流式压缩场景，会将压缩过程信息也写入到输出缓冲区，使得下次压缩可以继续
    // 同步刷新时，处理结果不会是Z_STREAM_END。
    // Z_BUF_ERROR不代表致命错误，而是输出缓冲区不足，需要清空输出缓冲区后再继续
    // todo 这里有个疑问：按前面的逻辑，输出缓冲区一定比输入大，因此也就不会返回Z_BUF_ERROR的结果，这个等待后续实验
    size_t use_size = output_size - deflate_stream_->avail_out;
    return use_size;
}

std::string ZlibCompress::Decompress(const void* data, size_t size) {
    if (!data) {
        return "";
    }
    if (IsCompressed(data, size)) {
        ResetDecompressStream_();
    }
    if (!inflate_stream_) {
        return "";
    }

    inflate_stream_->next_in = (Bytef*)data;
    inflate_stream_->avail_in = static_cast<uInt>(size);

    std::string output;
    while (inflate_stream_->avail_in > 0) {
        char buffer[4096] = {0};
        inflate_stream_->next_out = (Bytef*)buffer;
        inflate_stream_->avail_out = sizeof(buffer);
        int ret = inflate(inflate_stream_.get(), Z_SYNC_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            return "";
        }
        output.append(buffer, sizeof(buffer) - inflate_stream_->avail_out);
    }
    
    return output;
}

void ZlibCompress::ResetStream() {
    deflate_stream_ = std::unique_ptr<z_stream, ZStreamDeflateDeleter>(new z_stream());
    deflate_stream_->zalloc = Z_NULL;
    deflate_stream_->zfree = Z_NULL;
    deflate_stream_->opaque = Z_NULL;
    int ret = deflateInit2(deflate_stream_.get(), Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        deflate_stream_.reset();
    }
}

void ZlibCompress::ResetDecompressStream_() {
    inflate_stream_ = std::unique_ptr<z_stream, ZStreamInflateDeleter>(new z_stream());
    inflate_stream_->zalloc = Z_NULL;
    inflate_stream_->zfree = Z_NULL;
    inflate_stream_->opaque = Z_NULL;
    inflate_stream_->avail_in = 0;
    inflate_stream_->next_in = Z_NULL;

    int ret = inflateInit2(inflate_stream_.get(), MAX_WBITS);
    if (ret != Z_OK) {
        inflate_stream_.reset();
    }
}

} // namespace compress
} // namespace logger
