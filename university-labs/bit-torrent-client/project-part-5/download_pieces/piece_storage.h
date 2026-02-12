#pragma once

#include "torrent_file.h"
#include "piece.h"
#include <queue>
#include <string>
#include <unordered_set>
#include <mutex>

class PieceStorage {
public:
    explicit PieceStorage(const TorrentFile& tf);

    PiecePtr GetNextPieceToDownload();
    void PieceProcessed(const PiecePtr& piece);
    bool QueueIsEmpty() const;
    void AddPiece(PiecePtr& piece);
    size_t TotalPiecesCount() const;

protected:
    std::queue<PiecePtr> remainPieces_;
    virtual void SavePieceToDisk(PiecePtr piece);
};