#include "aes_crypto.h"

#include "cryptopp/aes.h"
#include "cryptopp/base64.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/oids.h"
#include "cryptopp/osrng.h"

namespace logger {
namespace crypto {

using CryptoPP::byte;

AESCrypto::AESCrypto(std::string key) : key_(key) {
    iv_ = GenerateIV_();
}

std::string AESCrypto::GenerateKey() {
    CryptoPP::AutoSeededRandomPool random;
    byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    random.GenerateBlock(key, sizeof(key));
    return BinaryKeyToHex(std::string(reinterpret_cast<const char*>(key), sizeof(key)));
}

/// @brief 生成初始化向量，用于保障加密安全性
std::string AESCrypto::GenerateIV_() {
    CryptoPP::AutoSeededRandomPool random;
    byte iv[CryptoPP::AES::BLOCKSIZE];
    random.GenerateBlock(iv, sizeof(iv));
    return BinaryKeyToHex(std::string(reinterpret_cast<const char*>(iv), sizeof(iv)));
}

void AESCrypto::Encrypt(const void* data, size_t size, std::string& output) {
    CryptoPP::AES::Encryption aes_encryption(reinterpret_cast<const byte*>(key_.data()), key_.size());
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbc_encryption(aes_encryption, reinterpret_cast<const byte*>(iv_.data()));

    CryptoPP::StreamTransformationFilter stf_encryptor(cbc_encryption, new CryptoPP::StringSink(output));
    stf_encryptor.Put(reinterpret_cast<const byte*>(data), size);
    stf_encryptor.MessageEnd();
}

std::string AESCrypto::Decrypt(const void* data, size_t size) {
    std::string decryptedtext;
    CryptoPP::AES::Decryption aes_decryption(reinterpret_cast<const byte*>(key_.data()), key_.size());
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbc_decryption(aes_decryption, reinterpret_cast<const byte*>(iv_.data()));

    CryptoPP::StreamTransformationFilter stf_decryptor(cbc_decryption, new CryptoPP::StringSink(decryptedtext));
    stf_decryptor.Put(reinterpret_cast<const byte*>(data), size);
    stf_decryptor.MessageEnd();
    return decryptedtext;
}
        
} // namespace crypto
} // namespace logger
