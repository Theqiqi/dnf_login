#pragma once
#include <string>
#include <vector>

namespace dnflogin::core {

class Crypto {
public:

    static std::string EncryptAndEncode(const std::vector<unsigned char>& rawPacket, const std::string& privateKeyPath = "");
    

    static std::string Base64Encode(const unsigned char* data, int len);

private:

    static bool LoadEmbeddedKey(const char*& pOutData, int& outSize);
};

} // namespace dnflogin::core
