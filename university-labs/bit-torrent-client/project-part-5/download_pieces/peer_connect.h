#pragma once

#include "tcp_connect.h"
#include "piece.h"
#include "peer.h"
#include "torrent_file.h"
#include "piece_storage.h"

//Информация о доступности частей файла у пира
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

// Соединение с одним пиром

class PeerConnect {
public:
    PeerConnect(const Peer& peer, const TorrentFile& tf,
                std::string selfPeerId, PieceStorage& pieceStorage);

    void Run();
    void Terminate();

private:
    const TorrentFile& tf_;
    TcpConnect socket_;
    const std::string selfPeerId_;
    std::string peerId_;
    PeerPiecesAvailability piecesAvailability_;
    bool terminated_;
    bool choked_;
    PiecePtr pieceInProgress_;
    PieceStorage& pieceStorage_;
    bool pendingBlock_;

    void PerformHandshake();
    bool EstablishConnection();
    void ReceiveBitfield();
    void SendInterested();
    void RequestPiece();
    void MainLoop();
};