#include "torrent_tracker.h"
#include "bencode.h"
#include "byte_tools.h"
#include <cpr/cpr.h>
#include <iostream>
#include <sstream>

TorrentTracker::TorrentTracker(const std::string& url) : url_(url) {}

void TorrentTracker::UpdatePeers(const TorrentFile& tf, std::string peerId, int port) {
    cpr::Response res = cpr::Get(
        cpr::Url{url_},
        cpr::Parameters{
            {"info_hash", tf.infoHash},
            {"peer_id", peerId},
            {"port", std::to_string(port)},
            {"uploaded", "0"},
            {"downloaded", "0"},
            {"left", std::to_string(tf.length)},
            {"compact", "1"}
        },
        cpr::Timeout{20000}
    );

    if (res.status_code != 200) {
        std::cerr << "Tracker error: HTTP " << res.status_code << std::endl;
        return;
    }

    std::istringstream responseStream(res.text);
    char firstChar;
    responseStream.get(firstChar);
    auto root = std::dynamic_pointer_cast<Map>(Bencode::Parse(responseStream, firstChar));
    if (!root) {
        std::cerr << "Failed to parse tracker response" << std::endl;
        return;
    }

    auto peersEntry = root->get("peers");
    if (!peersEntry) {
        std::cerr << "No 'peers' field in tracker response" << std::endl;
        return;
    }

    std::string peersData = std::dynamic_pointer_cast<String>(peersEntry)->Get();
    peers_ = Bencode::ParsePeers(peersData);
}

const std::vector<Peer>& TorrentTracker::GetPeers() const {
    return peers_;
}