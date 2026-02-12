#include "piece_storage.h"
#include <iostream>

PieceStorage::PieceStorage(const TorrentFile& tf, const std::filesystem::path& outputDirectory)
    : tf_(tf)
    , outputFile_(outputDirectory / tf.name, std::ios::out | std::ios::binary) {
    outputFile_.seekp(tf.length - 1);
    outputFile_.write("\0", 1);
    outputFile_.flush();

    size_t totalPieces = (tf.length + tf.pieceLength - 1) / tf.pieceLength;
    for (size_t i = 0; i < totalPieces; ++i) {
        size_t length = tf.pieceLength;
        if (i == totalPieces - 1)
            length = tf.length - i * tf.pieceLength;
        remainPieces_.push(std::make_shared<Piece>(i, length, tf.pieceHashes[i]));
    }
}

PiecePtr PieceStorage::GetNextPieceToDownload() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (remainPieces_.empty())
        return nullptr;
    auto piece = remainPieces_.front();
    remainPieces_.pop();
    return piece;
}

void PieceStorage::PieceProcessed(const PiecePtr& piece) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!piece->HashMatches()) {
        std::cerr << "Piece " << piece->GetIndex() << " hash mismatch, resetting" << std::endl;
        piece->Reset();
        remainPieces_.push(piece);
        return;
    }
    SavePieceToDisk(piece);
}

bool PieceStorage::QueueIsEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return remainPieces_.empty();
}

size_t PieceStorage::PiecesSavedToDiscCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return piecesSavedToDiscIndices_.size();
}

size_t PieceStorage::TotalPiecesCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return (tf_.length + tf_.pieceLength - 1) / tf_.pieceLength;
}

void PieceStorage::CloseOutputFile() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (outputFile_.is_open())
        outputFile_.close();
}

const std::vector<size_t>& PieceStorage::GetPiecesSavedToDiscIndices() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return piecesSavedToDiscIndices_;
}

size_t PieceStorage::PiecesInProgressCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return piecesSavedToDiscIndices_.size();
}

void PieceStorage::AddPiece(const PiecePtr& piece) {
    std::lock_guard<std::mutex> lock(mutex_);
    remainPieces_.push(piece);
}

void PieceStorage::SavePieceToDisk(const PiecePtr& piece) {
    if (!outputFile_.is_open())
        throw std::runtime_error("Output file is not open");
    piecesSavedToDiscIndices_.push_back(piece->GetIndex());
    std::streampos pos = piece->GetIndex() * tf_.pieceLength;
    outputFile_.seekp(pos);
    std::string data = piece->GetData();
    outputFile_.write(data.data(), data.size());
    outputFile_.flush();
}