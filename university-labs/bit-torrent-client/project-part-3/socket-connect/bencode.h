#pragma once

#include "peer.h"
#include <string>
#include <vector>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
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
        return it != data_.end() ? it->second : nullptr;
    }

    bool IsFind(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

private:
    std::map<std::string, std::shared_ptr<AllData>> data_;
};

namespace Bencode {
    std::vector<Peer> ParsePeers(const std::string& data);
    std::shared_ptr<AllData> Parse(std::istream& input, char firstChar);
    std::string GetHash(Map& map);
}