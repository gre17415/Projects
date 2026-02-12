#pragma once

#include "peer.h"
#include "torrent_file.h"
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <cpr/cpr.h>
#include <iostream>
#include <memory>

class AllData {
public:
    virtual std::string GetOrdHash() = 0;
    virtual ~AllData() = default;
};

class Integer : public AllData {
public:
    explicit Integer(size_t x) : data_(x) {}

    std::string GetOrdHash() override {
        return "i" + std::to_string(data_) + "e";
    }

    size_t& Get() { return data_; }

private:
    size_t data_;
};

class String : public AllData {
public:
    explicit String(const std::string& str) : data_(str) {}

    std::string GetOrdHash() override {
        return std::to_string(data_.size()) + ":" + data_;
    }

    std::string& Get() { return data_; }

private:
    std::string data_;
};

class List : public AllData {
public:
    explicit List(const std::list<std::shared_ptr<AllData>>& list) : data_(list) {}

    std::string GetOrdHash() override {
        std::string ans = "l";
        for (const auto& item : data_)
            ans += item->GetOrdHash();
        ans += "e";
        return ans;
    }

    std::list<std::shared_ptr<AllData>>& Get() { return data_; }

private:
    std::list<std::shared_ptr<AllData>> data_;
};

class Map : public AllData {
public:
    explicit Map(const std::map<std::string, std::shared_ptr<AllData>>& map) : data_(map) {}

    std::string GetOrdHash() override {
        std::string ans = "d";
        for (const auto& [key, val] : data_) {
            ans += std::to_string(key.size()) + ":" + key;
            ans += val->GetOrdHash();
        }
        ans += "e";
        return ans;
    }

    std::map<std::string, std::shared_ptr<AllData>>& Get() { return data_; }

    std::shared_ptr<AllData> get(const std::string& key) {
        auto it = data_.find(key);
        return (it != data_.end()) ? it->second : nullptr;
    }

private:
    std::map<std::string, std::shared_ptr<AllData>> data_;
};

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

class TorrentTracker {
public:
    explicit TorrentTracker(const std::string& url) : url_(url) {}

    void UpdatePeers(const TorrentFile& tf, const std::string& peerId, int port) {
        cpr::Response res = cpr::Get(
            cpr::Url{url_},
            cpr::Parameters{
                {"info_hash", tf.infoHash},
                {"peer_id", peerId},
                {"port", std::to_string(port)},
                {"uploaded", "0"},
                {"downloaded", "0"},
                {"left", std::to_string(tf.length)},
                {"compact", "1"}
            },
            cpr::Timeout{20000}
        );

        if (res.status_code != 200) {
            std::cerr << "Tracker error: HTTP " << res.status_code << std::endl;
            return;
        }

        std::istringstream responseStream(res.text);
        char firstChar;
        responseStream.get(firstChar);
        auto root = std::dynamic_pointer_cast<Map>(Parse(responseStream, firstChar));
        if (!root) {
            std::cerr << "Failed to parse tracker response" << std::endl;
            return;
        }

        auto peersEntry = root->get("peers");
        if (!peersEntry) {
            std::cerr << "No 'peers' field in tracker response" << std::endl;
            return;
        }

        std::string peersData = std::dynamic_pointer_cast<String>(peersEntry)->Get();
        peers_ = ParsePeers(peersData);
    }

    const std::vector<Peer>& GetPeers() const {
        return peers_;
    }

private:
    std::string url_;
    std::vector<Peer> peers_;
};

TorrentFile LoadTorrentFile(const std::string& filename) {
    TorrentFile tf;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    char firstChar;
    file.get(firstChar);
    auto root = std::dynamic_pointer_cast<Map>(Parse(file, firstChar));
    if (!root) {
        throw std::runtime_error("Invalid torrent file: root is not a dictionary");
    }

    tf.announce = std::dynamic_pointer_cast<String>(root->Get().at("announce"))->Get();
    tf.comment  = std::dynamic_pointer_cast<String>(root->Get().at("comment"))->Get();

    auto info = std::dynamic_pointer_cast<Map>(root->Get().at("info"));
    tf.pieceLength = std::dynamic_pointer_cast<Integer>(info->Get().at("piece length"))->Get();
    tf.length      = std::dynamic_pointer_cast<Integer>(info->Get().at("length"))->Get();

    auto pieces = std::dynamic_pointer_cast<String>(info->Get().at("pieces"));
    const std::string& piecesData = pieces->Get();
    for (size_t i = 0; i < piecesData.size(); i += 20) {
        tf.pieceHashes.push_back(piecesData.substr(i, 20));
    }

    std::string bencodedInfo = info->GetOrdHash();
    unsigned char hash[20];
    SHA1(reinterpret_cast<const unsigned char*>(bencodedInfo.c_str()),
         bencodedInfo.size(), hash);
    tf.infoHash.assign(reinterpret_cast<char*>(hash), 20);

    return tf;
}