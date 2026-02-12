#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>

//Части файла скачиваются не за одно сообщение, а блоками размером 2^14 байт или меньше
struct Block {

    enum Status {
        Missing = 0,
        Pending,
        Retrieved,
    };

    uint32_t piece;  // id части файла, к которой относится данный блок
    uint32_t offset;  // смещение начала блока относительно начала части файла в байтах
    uint32_t length;  // длина блока в байтах
    Status status;  // статус загрузки данного блока
    std::string data;  // бинарные данные
};

//Часть скачиваемого файла
class Piece {
public:
    Piece(size_t index, size_t length, std::string hash);

    bool HashMatches() const;
    Block* FirstMissingBlock();
    size_t GetIndex() const;
    void SaveBlock(size_t blockOffset, std::string data);
    bool AllBlocksRetrieved() const;
    std::string GetData() const;
    std::string GetDataHash() const;
    const std::string& GetHash() const;
    void Reset();

private:
    const size_t index_;
    const size_t length_;
    const std::string hash_;
    std::vector<Block> blocks_;
};

using PiecePtr = std::shared_ptr<Piece>;