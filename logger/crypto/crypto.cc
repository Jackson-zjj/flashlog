#include "crypto.h"

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

/// @brief 生成密钥对
std::tuple<std::string, std::string> GenECDHKey() {
    CryptoPP::AutoSeededRandomPool random;
    // ECHD算法封装类初始化，基于secp256r1椭圆曲线(ECP)
    CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());
    // 密钥存储容器初始化，SecByteBlock是CryptoPP的安全字节块
    CryptoPP::SecByteBlock pri_key(dh.PrivateKeyLength());
    CryptoPP::SecByteBlock pub_key(dh.PublicKeyLength());
    // 生成密钥对
    dh.GenerateKeyPair(random, pri_key, pub_key);
    return std::make_tuple(std::string(reinterpret_cast<const char*>(pri_key.data()), pri_key.size()),
                            std::string(reinterpret_cast<const char*>(pub_key.data()), pub_key.size()));
}

/// @brief 生成共享密钥
std::string GenECDHSharedKey(const std::string& client_pri, const std::string& server_pub) {
    CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());
    CryptoPP::SecByteBlock shared_key(dh.AgreedValueLength());
    CryptoPP::SecByteBlock pri_key(reinterpret_cast<const byte*>(client_pri.data()), client_pri.size());
    CryptoPP::SecByteBlock pub_key(reinterpret_cast<const byte*>(server_pub.data()), server_pub.size());
    if (!dh.Agree(shared_key, pri_key, pub_key)) {
        throw std::runtime_error("Failed to generate shared key");
    }
    return std::string(reinterpret_cast<const char*>(shared_key.data()), shared_key.size());
}

/// @brief 密钥二转十六
std::string BinaryKeyToHex(const std::string& binary_key) {
    std::string hex_key;
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(hex_key));
    encoder.Put(reinterpret_cast<const byte*>(binary_key.data()), binary_key.size());
    encoder.MessageEnd();
    return hex_key;
}

/// @brief 密钥十六转二
std::string HexKeyToBinary(const std::string& hex_key) {
    std::string binary_key;
    CryptoPP::HexDecoder decoder;
    decoder.Attach(new CryptoPP::StringSink(binary_key));
    decoder.Put(reinterpret_cast<const byte*>(hex_key.data()), hex_key.size());
    decoder.MessageEnd();
    return binary_key;
}
    
} // namespace crypto
} // namespace logger
