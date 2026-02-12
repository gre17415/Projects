#include "byte_tools.h"
#include "peer_connect.h"
#include "message.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <cassert>

using namespace std::chrono_literals;

PeerPiecesAvailability::PeerPiecesAvailability() {}

PeerPiecesAvailability::PeerPiecesAvailability(std::string bitfield) : bitfield_(std::move(bitfield)) {}

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
    size_t size = 0;
    for (unsigned char c : bitfield_) {
        size += __builtin_popcount(c);
    }
    return size;
}

PeerConnect::PeerConnect(const Peer& peer, const TorrentFile& tf, std::string selfPeerId)
    : tf_(tf),
      socket_(peer.ip, peer.port, std::chrono::seconds(1), std::chrono::seconds(1)),
      selfPeerId_(std::move(selfPeerId)),
      terminated_(false),
      choked_(true)
{}

void PeerConnect::Run() {
    while (!terminated_) {
        if (EstablishConnection()) {
            std::cout << "Connection established to peer" << std::endl;
            MainLoop();
        } else {
            std::cerr << "Cannot establish connection to peer" << std::endl;
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
    if (response[0] != '\x13' || response.substr(1, 19) != "BitTorrent protocol") {
        throw std::runtime_error("Handshake failed: invalid protocol");
    }
    if (response.substr(28, 20) != tf_.infoHash) {
        throw std::runtime_error("Handshake failed: info hash mismatch");
    }
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
        std::cerr << "Failed to establish connection with peer " << socket_.GetIp() << ":"
                  << socket_.GetPort() << " -- " << e.what() << std::endl;
        return false;
    }
}

void PeerConnect::ReceiveBitfield() {
    std::string msg = socket_.ReceiveData();
    if (msg.empty()) {
        throw std::runtime_error("Empty message received");
    }

    MessageId id = static_cast<MessageId>(msg[0]);
    if (id == MessageId::Unchoke) {
        choked_ = false;
        std::cout << "Received unchoke" << std::endl;
    } else if (id == MessageId::BitField) {
        std::string bitfield = msg.substr(1);
        piecesAvailability_ = PeerPiecesAvailability(bitfield);
        std::cout << "Received bitfield, available pieces: " << piecesAvailability_.Size() << std::endl;
    } else {
        throw std::runtime_error("Expected bitfield or unchoke, got " + std::to_string(static_cast<int>(id)));
    }
}

void PeerConnect::SendInterested() {
    Message msg = Message::Init(MessageId::Interested, "");
    socket_.SendData(msg.ToString());
    std::cout << "Sent interested" << std::endl;
}

void PeerConnect::Terminate() {
    std::cerr << "Terminating connection" << std::endl;
    terminated_ = true;
}

void PeerConnect::MainLoop() {
    // Будет переопределено в тестирующей системе
}