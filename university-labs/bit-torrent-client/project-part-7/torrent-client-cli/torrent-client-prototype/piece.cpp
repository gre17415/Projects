#include "byte_tools.h"
#include "piece.h"
#include <iostream>
#include <algorithm>

namespace {
constexpr size_t BLOCK_SIZE = 1 << 14;
}

Piece::Piece(size_t index, size_t length, std::string hash)
    : index_(index)
    , length_(length)
    , hash_(std::move(hash))
    , blocks_((length + BLOCK_SIZE - 1) / BLOCK_SIZE) {
    size_t remaining = length;
    for (size_t i = 0; i < blocks_.size(); ++i) {
        blocks_[i].piece = index_;
        blocks_[i].offset = i * BLOCK_SIZE;
        blocks_[i].length = std::min<size_t>(BLOCK_SIZE, remaining);
        blocks_[i].status = Block::Missing;
        remaining -= blocks_[i].length;
    }
}

void Piece::Reset() {
    for (auto& block : blocks_) {
        block.data.clear();
        block.status = Block::Missing;
    }
}

const std::string& Piece::GetHash() const {
    return hash_;
}

size_t Piece::GetIndex() const {
    return index_;
}

std::string Piece::GetData() const {
    std::string data;
    for (const auto& block : blocks_)
        data += block.data;
    return data;
}

std::string Piece::GetDataHash() const {
    return CalculateSHA1(GetData());
}

bool Piece::HashMatches() const {
    return hash_ == GetDataHash();
}

void Piece::SaveBlock(size_t blockOffset, std::string data) {
    size_t blockIndex = blockOffset / BLOCK_SIZE;
    if (blockIndex < blocks_.size()) {
        blocks_[blockIndex].data = std::move(data);
        blocks_[blockIndex].status = Block::Retrieved;
    } else {
        throw std::runtime_error("Invalid block offset");
    }
}

Block* Piece::FirstMissingBlock() {
    for (auto& block : blocks_)
        if (block.status == Block::Missing)
            return &block;
    return nullptr;
}

bool Piece::AllBlocksRetrieved() const {
    for (const auto& block : blocks_)
        if (block.status != Block::Retrieved)
            return false;
    return true;
}