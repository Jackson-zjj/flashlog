#include <iostream>
#include <fstream>
#include <exception>
#include <memory>

#include "decode_formatter.h"
#include "logger/compress/zstd_compress.h"
#include "logger/crypto/aes_crypto.h"
#include "logger/sinks/effective_sink.h"

namespace {

using namespace logger;
using namespace logger::detail;

std::unique_ptr<DecodeFormatter> decode_formatter;
std::unique_ptr<logger::compress::Compression> decompress;

std::vector<char> ReadFile(const std::string& file_path) {
    std::ifstream ifs(file_path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("ReadFile open file failed");
    }

    ifs.seekg(0, std::ios::end);
    size_t file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::vector<char> data_buf(file_size);
    ifs.read(data_buf.data(), file_size);
    return data_buf;
}

void WriteToFile(const std::string& file_path, const std::string& data) {
    std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
    ofs.write(data.data(), data.size());
}

size_t DecodeItemData(const char* data, std::shared_ptr<crypto::Crypto> crypto, std::string& output_buf) {
    auto item_header = reinterpret_cast<const ItemHeader*>(data);
    if (item_header->magic != ItemHeader::kMagic) {
        throw std::runtime_error("DecodeItemData: itemHeader magic invalid");
    }

    auto item_data = data + sizeof(ItemHeader);
    std::string decrypted_data = crypto->Decrypt(item_data, item_header->size);
    std::string decompressed_data = decompress->Decompress(decrypted_data.data(), decrypted_data.size());
    EffectiveMsg msg;
    msg.ParseFromString(decompressed_data);
    std::string data_buf;
    decode_formatter->Format(msg, data_buf);
    output_buf.append(data_buf);
    return sizeof(ItemHeader) + item_header->size;
}

size_t DecodeChunkData(const char* file_data, std::string ser_pri_key, std::string& output_buf) {
    auto chunk_header = reinterpret_cast<const ChunkHeader*>(file_data);
    if (chunk_header->magic != ChunkHeader::kMagic) {
        throw std::runtime_error("DecodeChunkData: chunkHeader magic invalid");
    }

    auto chunk_data = file_data + sizeof(ChunkHeader);
    std::string crypto_iv = std::string(chunk_header->iv, 17);
    std::string cli_pub_key = std::string(chunk_header->pub_key, 65);
    std::string ser_pri_key_bin = crypto::HexKeyToBinary(ser_pri_key);
    std::string shared_key = crypto::GenECDHSharedKey(ser_pri_key_bin, cli_pub_key);
    auto crypto = std::make_shared<crypto::AESCrypto>(shared_key, crypto_iv);

    size_t offset = 0;
    while (offset < chunk_header->size) {
        size_t item_size = DecodeItemData(chunk_data + offset, crypto, output_buf);
        offset += item_size;
        output_buf.push_back('\n');
    }
    return sizeof(ChunkHeader) + chunk_header->size;
}

void DecodeFile(const std::string& file_path, const std::string& pri_key, const std::string& output_file_path) {
    auto file_data = ReadFile(file_path);
    if (file_data.size() < sizeof(ChunkHeader)) {
        throw std::runtime_error("DecodeFile: data is too small");
    }

    size_t offset = 0;
    size_t file_size = file_data.size();
    std::string output_buf;

    while (offset < file_size) {
        output_buf.clear();
        size_t chunk_size = DecodeChunkData(file_data.data() + offset, pri_key, output_buf);
        offset += chunk_size;
        WriteToFile(output_file_path, output_buf);
    }
}

} // namespace


int main(int argc, char* argv[]) {
#if false
    if (argc != 4) {
        std::cerr << "Usage: ./decode <file_path> <pri_key> <output_file>" << std::endl;
        return 1;
    }
    std::string input_file_path = argv[1];
    std::string pri_key = argv[2];
    std::string output_file_path = argv[3];
#else
    std::string input_file_path = "/Users/jackson/MyProject/flashlog/test/loggerdemo_20251231131616.log";
    std::string pri_key = "FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E";
    std::string output_file_path = "/Users/jackson/MyProject/flashlog/test/log.txt";
#endif
    
    try {
        decode_formatter = std::make_unique<DecodeFormatter>();
        decode_formatter->SetPattern("[%l][%D:%S][%p:%t][%F:%f:%#]%v");
        decompress = std::make_unique<logger::compress::ZstdCompress>();
        DecodeFile(input_file_path, pri_key, output_file_path);

    } catch (const std::exception& err) {
        std::cerr << "Decode failed: " << err.what() << std::endl;
        return 1;
    }
    return 0;
}