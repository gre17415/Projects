#include "bencode.h"
#include <utility>
#include <openssl/sha.h>

namespace Bencode {

    std::shared_ptr<AllData> Parse(std::istream& input, char firstChar) {
        char current;
        input.get(current);

        if (firstChar == 'l') {
            std::list<std::shared_ptr<AllData>> list;
            while (current != 'e') {
                list.push_back(Parse(input, current));
                input.get(current);
            }
            return std::make_shared<List>(list);
        }
        else if (firstChar == 'i') {
            std::string numStr;
            while (current != 'e') {
                numStr += current;
                input.get(current);
            }
            size_t value = 0;
            for (char ch : numStr)
                value = value * 10 + (ch - '0');
            return std::make_shared<Integer>(value);
        }
        else if (firstChar == 'd') {
            std::map<std::string, std::shared_ptr<AllData>> dict;
            std::string key;
            while (current != 'e') {
                if (key.empty()) {
                    auto strObj = std::dynamic_pointer_cast<String>(Parse(input, current));
                    key = strObj->Get();
                } else {
                    dict[key] = Parse(input, current);
                    key.clear();
                }
                input.get(current);
            }
            return std::make_shared<Map>(dict);
        }
        else {
            // string: <length>:<data>
            size_t len = firstChar - '0';
            while (current != ':') {
                len = len * 10 + (current - '0');
                input.get(current);
            }
            std::string str;
            for (size_t i = 0; i < len; ++i) {
                input.get(current);
                str += current;
            }
            return std::make_shared<String>(str);
        }
    }

    std::vector<Peer> ParsePeers(const std::string& data) {
        std::vector<Peer> result;
        for (size_t i = 0; i + 6 <= data.size(); i += 6) {
            std::string ip;
            for (int j = 0; j < 4; ++j) {
                ip += std::to_string(static_cast<unsigned char>(data[i + j]));
                if (j < 3) ip += '.';
            }
            uint16_t port = (static_cast<unsigned char>(data[i + 4]) << 8) |
                             static_cast<unsigned char>(data[i + 5]);
            result.push_back({ip, port});
        }
        return result;
    }

    std::string GetHash(Map& map) {
        const std::string key = "info";
        if (!map.IsFind(key))
            throw std::runtime_error("Bencode error: info section not found");
        std::string bencoded = map.get(key)->GetOrdHash();
        unsigned char hash[20];
        SHA1(reinterpret_cast<const unsigned char*>(bencoded.c_str()),
             bencoded.size(), hash);
        return std::string(reinterpret_cast<char*>(hash), 20);
    }
}