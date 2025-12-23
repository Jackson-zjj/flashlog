#pragma once

#include "crypto.h"

namespace logger {
namespace crypto {

class AESCrypto final : public Crypto{
public:
    AESCrypto(std::string key);

    ~AESCrypto() override = default;

    static std::string GenerateKey();

    static std::string GenerateIV();

    void Encrypto(const void* data, size_t size, std::string& output) override;

    std::string Decrypto(const void* data, size_t size) override;

private:
    std::string key_;
    std::string iv_;

};  // class AESCrypto
    
} // namespace crypto
} // namespace logger

