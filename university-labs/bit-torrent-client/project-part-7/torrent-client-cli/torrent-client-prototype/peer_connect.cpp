#include "byte_tools.h"
#include "peer_connect.h"
#include "message.h"
#include <iostream>

using namespace std::chrono_literals;

PeerPiecesAvailability::PeerPiecesAvailability() {}

PeerPiecesAvailability::PeerPiecesAvailability(std::string bitfield)
    : bitfield_(std::move(bitfield)) {}

bool PeerPiecesAvailability::IsPieceAvailable(size_t pieceIndex) const {
    size_t byteIndex = pieceIndex / 8;
    size_t bitIndex = 7 - pieceIndex % 8;
    return (bitfield_[byteIndex] & (1 << bitIndex)) != 0;
}

void PeerPiecesAvailability::SetPieceAvailability(size_t pieceIndex) {
    size_t byteIndex = pieceIndex / 8;
    size_t bitIndex = 7 - pieceIndex % 8;
    bitfield_[byteIndex] |= (1 << bitIndex);
}

size_t PeerPiecesAvailability::Size() const {
    size_t count = 0;
    for (unsigned char c : bitfield_)
        count += __builtin_popcount(c);
    return count;
}

PeerConnect::PeerConnect(const Peer& peer, const TorrentFile& tf,
                         std::string selfPeerId, PieceStorage& pieceStorage)
    : tf_(tf),
      socket_(peer.ip, peer.port, std::chrono::seconds(1), std::chrono::seconds(1)),
      selfPeerId_(std::move(selfPeerId)),
      terminated_(false),
      choked_(true),
      pieceStorage_(pieceStorage),
      pendingBlock_(false),
      failed_(false) {}

void PeerConnect::Run() {
    while (!terminated_) {
        if (EstablishConnection()) {
            std::cout << "Connection established to peer" << std::endl;
            MainLoop();
        } else {
            std::cerr << "Cannot establish connection to peer" << std::endl;
            failed_ = true;
            Terminate();
        }
    }
}

void PeerConnect::PerformHandshake() {
    socket_.EstablishConnection();

    std::string request;
    request += static_cast<char>(19);
    request += "BitTorrent protocol" + std::string(8, '\0') + tf_.infoHash + selfPeerId_;
    socket_.SendData(request);

    std::string response = socket_.ReceiveData(68);
    if (response[0] != '\x13' || response.substr(1, 19) != "BitTorrent protocol")
        throw std::runtime_error("Handshake failed: invalid protocol");
    if (response.substr(28, 20) != tf_.infoHash)
        throw std::runtime_error("Handshake failed: info hash mismatch");
    peerId_ = response.substr(48, 20);
    std::cout << "Successful handshake with peer " << peerId_ << std::endl;
}

bool PeerConnect::EstablishConnection() {
    try {
        PerformHandshake();
        ReceiveBitfield();
        SendInterested();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to establish connection with peer "
                  << socket_.GetIp() << ":" << socket_.GetPort()
                  << " -- " << e.what() << std::endl;
        return false;
    }
}

void PeerConnect::ReceiveBitfield() {
    std::string msg = socket_.ReceiveData();
    if (msg.empty())
        throw std::runtime_error("Empty message");

    MessageId id = static_cast<MessageId>(msg[0]);
    if (id == MessageId::Unchoke) {
        choked_ = false;
        std::cout << "Received unchoke" << std::endl;
    } else if (id == MessageId::BitField) {
        std::string bitfield = msg.substr(1);
        piecesAvailability_ = PeerPiecesAvailability(bitfield);
        std::cout << "Received bitfield, available pieces: "
                  << piecesAvailability_.Size() << std::endl;
    } else {
        throw std::runtime_error("Expected bitfield or unchoke, got "
                                 + std::to_string(static_cast<int>(id)));
    }
}

void PeerConnect::SendInterested() {
    Message msg = Message::Init(MessageId::Interested, "");
    socket_.SendData(msg.ToString());
    std::cout << "Sent interested" << std::endl;
}

void PeerConnect::RequestPiece() {
    if (pieceStorage_.QueueIsEmpty()) {
        std::cout << "All pieces have been downloaded" << std::endl;
        return;
    }

    if (!pieceInProgress_) {
        auto piece = pieceStorage_.GetNextPieceToDownload();
        while (piece && !piecesAvailability_.IsPieceAvailable(piece->GetIndex())) {
            pieceStorage_.AddPiece(piece);
            piece = pieceStorage_.GetNextPieceToDownload();
        }
        pieceInProgress_ = piece;
    }

    if (!pieceInProgress_)
        return;

    if (!pendingBlock_) {
        pendingBlock_ = true;
        Block* block = pieceInProgress_->FirstMissingBlock();
        if (block) {
            std::string request =
                IntToBytes(pieceInProgress_->GetIndex()) +
                IntToBytes(block->offset) +
                IntToBytes(block->length);
            Message msg = Message::Init(MessageId::Request, request);
            socket_.SendData(msg.ToString());
            std::cout << "Request piece " << pieceInProgress_->GetIndex()
                      << " block offset " << block->offset
                      << " length " << block->length << std::endl;
        } else {
            pieceInProgress_ = nullptr;
            pendingBlock_ = false;
        }
    }
}

void PeerConnect::MainLoop() {
    try {
        std::cout << "MainLoop started" << std::endl;
        while (!terminated_) {
            std::string data = socket_.ReceiveData();
            if (data.empty())
                continue;
            Message msg = Message::Parse(data);
            std::string payload = data.substr(1);

            switch (msg.id) {
            case MessageId::Have: {
                size_t pieceIndex = BytesToInt(payload.substr(0, 4));
                piecesAvailability_.SetPieceAvailability(pieceIndex);
                std::cout << "Have: piece " << pieceIndex << std::endl;
                break;
            }
            case MessageId::Piece: {
                size_t pieceIndex = BytesToInt(payload.substr(0, 4));
                size_t offset = BytesToInt(payload.substr(4, 4));
                std::string blockData = payload.substr(8);
                if (pieceInProgress_ && pieceInProgress_->GetIndex() == pieceIndex) {
                    pieceInProgress_->SaveBlock(offset, blockData);
                    if (pieceInProgress_->AllBlocksRetrieved()) {
                        pieceStorage_.PieceProcessed(pieceInProgress_);
                        pieceInProgress_ = nullptr;
                    }
                }
                pendingBlock_ = false;
                break;
            }
            case MessageId::Choke:
                std::cout << "Choke received" << std::endl;
                Terminate();
                break;
            case MessageId::Unchoke:
                std::cout << "Unchoke received" << std::endl;
                choked_ = false;
                break;
            default:
                std::cout << "Received message id="
                          << static_cast<int>(msg.id) << std::endl;
                break;
            }

            if (!choked_ && !pendingBlock_)
                RequestPiece();
        }
        std::cout << "MainLoop finished" << std::endl;
    } catch (...) {
        if (pieceInProgress_)
            pieceStorage_.AddPiece(pieceInProgress_);
        Terminate();
    }
}

void PeerConnect::Terminate() {
    std::cerr << "Terminating connection" << std::endl;
    terminated_ = true;
}

bool PeerConnect::Failed() const {
    return failed_;
}