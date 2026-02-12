#pragma once

#include "torrent_file.h"
#include "piece.h"
#include <queue>
#include <vector>
#include <mutex>
#include <fstream>
#include <filesystem>

class PieceStorage {
public:
    PieceStorage(const TorrentFile& tf, const std::filesystem::path& outputDirectory);

    PiecePtr GetNextPieceToDownload();
    void PieceProcessed(const PiecePtr& piece);
    bool QueueIsEmpty() const;
    size_t PiecesSavedToDiscCount() const;
    size_t TotalPiecesCount() const;
    void CloseOutputFile();
    const std::vector<size_t>& GetPiecesSavedToDiscIndices() const;
    size_t PiecesInProgressCount() const;
    void AddPiece(const PiecePtr& piece);

protected:
    std::queue<PiecePtr> remainPieces_;
    mutable std::mutex mutex_;
    std::ofstream outputFile_;
    const TorrentFile& tf_;
    std::vector<size_t> piecesSavedToDiscIndices_;

    virtual void SavePieceToDisk(const PiecePtr& piece);
};