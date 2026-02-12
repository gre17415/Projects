#pragma once

#include "tcp_connect.h"
#include "piece.h"
#include "peer.h"
#include "torrent_file.h"
#include "piece_storage.h"
#include <atomic>

class PeerPiecesAvailability {
public:
    PeerPiecesAvailability();
    explicit PeerPiecesAvailability(std::string bitfield);

    bool IsPieceAvailable(size_t pieceIndex) const;
    void SetPieceAvailability(size_t pieceIndex);
    size_t Size() const;

private:
    std::string bitfield_;
};

class PeerConnect {
public:
    PeerConnect(const Peer& peer, const TorrentFile& tf,
                std::string selfPeerId, PieceStorage& pieceStorage);

    void Run();
    void Terminate();
    bool Failed() const;

private:
    const TorrentFile& tf_;
    TcpConnect socket_;
    const std::string selfPeerId_;
    std::string peerId_;
    PeerPiecesAvailability piecesAvailability_;
    std::atomic<bool> terminated_;
    bool choked_;
    PiecePtr pieceInProgress_;
    PieceStorage& pieceStorage_;
    bool pendingBlock_;
    bool failed_;

    void PerformHandshake();
    bool EstablishConnection();
    void ReceiveBitfield();
    void SendInterested();
    void RequestPiece();
    void MainLoop();
};