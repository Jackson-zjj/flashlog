#pragma once

#include "crypto.h"

namespace logger {
namespace crypto {

class AESCrypto final : public Crypto{
public:
    AESCrypto(std::string key);

    ~AESCrypto() override = default;

    static std::string GenerateKey();


    void Encrypt(const void* data, size_t size, std::string& output) override;

    std::string Decrypt(const void* data, size_t size) override;

private:
    std::string GenerateIV_();

    std::string key_;
    std::string iv_;

};  // class AESCrypto
    
} // namespace crypto
} // namespace logger

