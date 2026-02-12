#include "piece_storage.h"
#include <iostream>

PieceStorage::PieceStorage(const TorrentFile& tf) {
    size_t totalSize = tf.length;
    size_t pieceLength = tf.pieceLength;
    size_t numPieces = (totalSize + pieceLength - 1) / pieceLength;

    for (size_t i = 0; i < numPieces; ++i) {
        size_t length = pieceLength;
        if (i == numPieces - 1)
            length = totalSize - i * pieceLength;
        remainPieces_.push(std::make_shared<Piece>(i, length, tf.pieceHashes[i]));
    }
}

PiecePtr PieceStorage::GetNextPieceToDownload() {
    if (remainPieces_.empty())
        return nullptr;
    auto piece = remainPieces_.front();
    remainPieces_.pop();
    return piece;
}

void PieceStorage::AddPiece(PiecePtr& piece) {
    remainPieces_.push(piece);
}

void PieceStorage::PieceProcessed(const PiecePtr& piece) {
    if (!piece->HashMatches()) {
        std::cerr << "Piece " << piece->GetIndex() << " hash mismatch, resetting" << std::endl;
        piece->Reset();
        remainPieces_.push(piece);
        return;
    }
    SavePieceToDisk(piece);
    // Очищаем очередь, так как требуется только одна успешная часть
    std::queue<PiecePtr> empty;
    std::swap(remainPieces_, empty);
}

bool PieceStorage::QueueIsEmpty() const {
    return remainPieces_.empty();
}

size_t PieceStorage::TotalPiecesCount() const {
    return remainPieces_.size();
}

void PieceStorage::SavePieceToDisk(PiecePtr piece) {
    // Будет переопределено в тестирующей системе
}