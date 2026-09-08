#include "core/Crypto.hpp"
#include <windows.h> 
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/buffer.h>
#include <iostream>

namespace dnflogin::core {

// === Isolated Win32 Resource Bridge ===
bool Crypto::LoadEmbeddedKey(const char*& pOutData, int& outSize) {
    HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCEA(101), "PEM_KEY");
    if (!hRes) {
        std::cerr << "❌ Launcher Critical Resource Asset Missing (Err: " << GetLastError() << ")\n";
        return false;
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return false;

    pOutData = static_cast<const char*>(LockResource(hData));
    outSize = static_cast<int>(SizeofResource(NULL, hRes));

    return (pOutData != nullptr && outSize > 0);
}

// 接收 Ticket 生成的 46 字节原始数据包
std::string Crypto::EncryptAndEncode(const std::vector<unsigned char>& rawPacket, const std::string& /* privateKeyPath */) {
    if (rawPacket.size() != 46) {
        std::cerr << "❌ Error: Packet size must be exactly 46 bytes.\n";
        return "";
    }

    const char* keyData = nullptr;
    int keySize = 0;

    if (!LoadEmbeddedKey(keyData, keySize)) {
        return "";
    }

    BIO* bio = BIO_new_mem_buf(keyData, keySize);
    if (!bio) return "";

    RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (!rsa) {
        std::cerr << "❌ OpenSSL failed to parse embedded key buffer structure.\n";
        return "";
    }


    std::vector<unsigned char> encryptedData(RSA_size(rsa), 0x00);
    

    int encryptedLen = RSA_private_encrypt(
        static_cast<int>(rawPacket.size()),
        rawPacket.data(),
        encryptedData.data(),
        rsa,
        RSA_PKCS1_PADDING 
    );

    RSA_free(rsa);
    if (encryptedLen == -1) {
        std::cerr << "❌ OpenSSL encryption failed. Check your private key format.\n";
        return "";
    }

    return Base64Encode(encryptedData.data(), encryptedLen);
}

std::string Crypto::Base64Encode(const unsigned char* data, int len) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); 
    b64 = BIO_push(b64, mem);

    BIO_write(b64, data, len);
    (void)BIO_flush(b64);

    BUF_MEM* bptr = nullptr;
    BIO_get_mem_ptr(b64, &bptr);
    
    std::string out(bptr->data, bptr->length);
    BIO_free_all(b64);
    
    return out;
}

} // namespace dnflogin::core
