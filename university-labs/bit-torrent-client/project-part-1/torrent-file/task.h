#pragma once

#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <memory>

struct TorrentFile 
{
    std::string announce;
    std::string comment;
    std::vector<std::string> pieceHashes;
    size_t pieceLength;
    size_t length;
    std::string name;
    std::string infoHash;
};

class AllData 
{
public:
    virtual std::string GetOrdHash() = 0;
    virtual ~AllData() = default;
};

class Integer : public AllData 
{
public:
    Integer(size_t x) : data_(x) {}

    std::string GetOrdHash() override 
    {
        return "i" + std::to_string(data_) + "e";
    }

    size_t& Get() { return data_; }

private:
    size_t data_;
};

class String : public AllData 
{
public:
    String(std::string& str) : data_(str) {}

    std::string GetOrdHash() override 
    {
        return std::to_string(data_.size()) + ":" + data_;
    }

    std::string& Get() { return data_; }

private:
    std::string data_;
};

class List : public AllData 
{
public:
    List(std::list<std::shared_ptr<AllData>> list) : data_(list) {}

    std::string GetOrdHash() override 
    {
        std::string ans = "l";
        for (auto& item : data_)
            ans += item->GetOrdHash();
        ans += "e";
        return ans;
    }

    std::list<std::shared_ptr<AllData>>& Get() { return data_; }

private:
    std::list<std::shared_ptr<AllData>> data_;
};

class Map : public AllData 
{
public:
    Map(std::map<std::string, std::shared_ptr<AllData>>& map) : data_(map) {}

    std::string GetOrdHash() override 
    {
        std::string ans = "d";
        for (auto& [key, val] : data_) 
        {
            ans += std::to_string(key.size()) + ":" + key;
            ans += val->GetOrdHash();
        }
        ans += "e";
        return ans;
    }

    std::map<std::string, std::shared_ptr<AllData>>& Get() { return data_; }

private:
    std::map<std::string, std::shared_ptr<AllData>> data_;
};

std::shared_ptr<AllData> Parse(std::ifstream& input, char firstChar) 
{
    char current;
    input.get(current);

    if (firstChar == 'l') 
    {
        std::list<std::shared_ptr<AllData>> list;
        while (current != 'e') 
        {
            list.push_back(Parse(input, current));
            input.get(current);
        }
        return std::make_shared<List>(list);
    }
    else if (firstChar == 'i') 
    {
        std::string numStr;
        while (current != 'e') 
        {
            numStr += current;
            input.get(current);
        }
        size_t value = 0;
        for (char ch : numStr)
            value = value * 10 + (ch - '0');
        return std::make_shared<Integer>(value);
    }
    else if (firstChar == 'd') 
    {
        std::map<std::string, std::shared_ptr<AllData>> dict;
        std::string key;
        while (current != 'e') 
        {
            if (key.empty())
                key = std::dynamic_pointer_cast<String>(Parse(input, current))->Get();
            else 
            {
                dict[key] = Parse(input, current);
                key.clear();
            }
            input.get(current);
        }
        return std::make_shared<Map>(dict);
    }
    else
    {
        size_t len = firstChar - '0';
        while (current != ':') 
        {
            len = len * 10 + (current - '0');
            input.get(current);
        }
        std::string str;
        for (size_t i = 0; i < len; ++i) 
        {
            input.get(current);
            str += current;
        }
        return std::make_shared<String>(str);
    }
}

TorrentFile LoadTorrentFile(const std::string& filename) 
{
    TorrentFile tf;
    char firstChar;
    std::ifstream file(filename);
    file.get(firstChar);
    auto root = std::dynamic_pointer_cast<Map>(Parse(file, firstChar));

    tf.announce = std::dynamic_pointer_cast<String>(root->Get().at("announce"))->Get();
    tf.comment  = std::dynamic_pointer_cast<String>(root->Get().at("comment"))->Get();

    auto info = std::dynamic_pointer_cast<Map>(root->Get().at("info"));
    tf.pieceLength = std::dynamic_pointer_cast<Integer>(info->Get().at("piece length"))->Get();
    tf.length      = std::dynamic_pointer_cast<Integer>(info->Get().at("length"))->Get();

    auto pieces = std::dynamic_pointer_cast<String>(info->Get().at("pieces"));
    const std::string& piecesData = pieces->Get();
    for (size_t i = 0; i < piecesData.size(); i += 20)
        tf.pieceHashes.push_back(piecesData.substr(i, 20));

    std::string bencodedInfo = info->GetOrdHash();
    unsigned char hash[20];
    SHA1(reinterpret_cast<const unsigned char*>(bencodedInfo.c_str()),
         bencodedInfo.size(), hash);
    tf.infoHash.assign(reinterpret_cast<char*>(hash), 20);

    return tf;
}