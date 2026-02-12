#include "byte_tools.h"
#include <openssl/sha.h>
#include <vector>

int BytesToInt(std::string_view bytes) {
    int res = 0;
    for (size_t i = 0; i < 4 && i < bytes.size(); ++i) {
        res = (res << 8) | static_cast<unsigned char>(bytes[i]);
    }
    return res;
}

std::string IntToBytes(size_t x) {
    std::string res(4, '\0');
    for (int i = 3; i >= 0; --i) {
        res[i] = static_cast<char>(x & 0xFF);
        x >>= 8;
    }
    return res;
}

std::string CalculateSHA1(const std::string& msg) {
    std::string hash(20, '\0');
    SHA1(reinterpret_cast<const unsigned char*>(msg.data()),
         msg.size(),
         reinterpret_cast<unsigned char*>(&hash[0]));
    return hash;
}