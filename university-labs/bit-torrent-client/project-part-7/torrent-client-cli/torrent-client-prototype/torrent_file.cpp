#include "torrent_file.h"
#include "bencode.h"
#include <openssl/sha.h>
#include <fstream>

TorrentFile LoadTorrentFile(const std::string& filename) {
    TorrentFile tf;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open torrent file: " + filename);

    char firstChar;
    file.get(firstChar);
    auto root = std::dynamic_pointer_cast<Map>(Bencode::Parse(file, firstChar));

    tf.announce = std::dynamic_pointer_cast<String>(root->Get().at("announce"))->Get();
    tf.comment  = std::dynamic_pointer_cast<String>(root->Get().at("comment"))->Get();

    auto info = std::dynamic_pointer_cast<Map>(root->Get().at("info"));
    tf.name = std::dynamic_pointer_cast<String>(info->Get().at("name"))->Get();
    tf.pieceLength = std::dynamic_pointer_cast<Integer>(info->Get().at("piece length"))->Get();
    tf.length      = std::dynamic_pointer_cast<Integer>(info->Get().at("length"))->Get();

    auto pieces = std::dynamic_pointer_cast<String>(info->Get().at("pieces"));
    const std::string& piecesData = pieces->Get();
    for (size_t i = 0; i < piecesData.size(); i += 20)
        tf.pieceHashes.push_back(piecesData.substr(i, 20));

    std::string bencodedInfo = info->GetOrdHash();
    unsigned char hash[20];
    SHA1(reinterpret_cast<const unsigned char*>(bencodedInfo.c_str()),
         bencodedInfo.size(), hash);
    tf.infoHash.assign(reinterpret_cast<char*>(hash), 20);

    return tf;
}