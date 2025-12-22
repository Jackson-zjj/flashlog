#pragma once

#include <string>
#include <tuple>

namespace logger {
namespace crypto {

std::tuple<std::string, std::string> GenECDHKey();

std::string GenECDHSharedKey(const std::string& client_pri, const std::string& server_pub);

std::string BinaryKeyToHex(const std::string& binary_key);

std::string HexKeyToBinary(const std::string& hex_key);

class Crypto {
public:
    virtual ~Crypto() = default;

    virtual void Encrypto(const void* data, size_t size, std::string& output) = 0;

    virtual std::string Decrypto(const void* data, size_t size) = 0;

};  // class Crypto
    
} // namespace crypto
} // namespace logger
