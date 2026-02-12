#include "tcp_connect.h"
#include "byte_tools.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <limits>
#include <utility>

TcpConnect::TcpConnect(std::string ip, int port,
                       std::chrono::milliseconds connectTimeout,
                       std::chrono::milliseconds readTimeout)
    : mode(0),
      ip_(std::move(ip)),
      port_(port),
      connectTimeout_(connectTimeout),
      readTimeout_(readTimeout) {}

TcpConnect::~TcpConnect() {
    if (mode == 2) return;
    CloseConnection();
}

// Константы и структуры оставлены для справки
// AF_INET - IPv4
// SOCK_STREAM - TCP
// F_GETFL - получить флаги файлового дескриптора
// O_NONBLOCK - неблокирующий режим
// POLLOUT - готовность к записи

// Структуры:
/*
    // sockaddr_in - структура для адреса IPv4
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // sockaddr - общая абстрактная структура адреса
    sockaddr genericAddress;
    genericAddress.sa_family = AF_INET;

    // pollfd - структура для poll()
    pollfd pollFileDescriptor;
    pollFileDescriptor.fd = 0;
    pollFileDescriptor.events = POLLIN;
    pollFileDescriptor.revents = 0;
*/

void TcpConnect::EstablishConnection() {
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = inet_addr(ip_.c_str());

    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0)
        throw std::runtime_error("Failed to create socket");

    int flags = fcntl(sock_, F_GETFL, 0);
    fcntl(sock_, F_SETFL, flags | O_NONBLOCK);

    int ret = connect(sock_, (struct sockaddr*)&address, sizeof(address));
    if (ret == 0) {
        flags = fcntl(sock_, F_GETFL, 0);
        fcntl(sock_, F_SETFL, flags & ~O_NONBLOCK);
        return;
    }

    struct pollfd pfd;
    pfd.fd = sock_;
    pfd.events = POLLOUT;

    ret = poll(&pfd, 1, connectTimeout_.count());
    if (ret == 0)
        throw std::runtime_error("Connection timed out");
    else if (ret < 0)
        throw std::runtime_error("Error during connection");

    flags = fcntl(sock_, F_GETFL, 0);
    fcntl(sock_, F_SETFL, flags & ~O_NONBLOCK);
}

void TcpConnect::SendData(const std::string& data) const {
    int sent = send(sock_, data.data(), data.size(), 0);
    if (sent < 0)
        throw std::runtime_error("Send failed");
    std::cout << "Sent " << sent << " bytes" << std::endl;
}


/*
1. SOL_SOCKET:
   - уровень сокета для общих параметров
2. SO_RCVTIMEO:
   - время ожидания приёма данных
   - устанавливается через setsockopt()
3. timeval:
   - структура для представления времени (сек/мксек)
*/
std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    pollfd fds = {sock_, POLLIN, 0};
    int ret = poll(&fds, 1, readTimeout_.count());
    if (ret <= 0)
        throw std::runtime_error("Receive timeout or error");

    if (bufferSize > 0) {
        std::string data(bufferSize, 0);
        char* ptr = &data[0];
        size_t remaining = bufferSize;
        while (remaining > 0) {
            ssize_t n = recv(sock_, ptr, remaining, 0);
            if (n <= 0 && errno != EAGAIN)
                throw std::runtime_error("Receive failed");
            if (n == 0)
                return "";
            ptr += n;
            remaining -= n;
        }
        return data;
    } else {
        char lenBuf[4];
        ssize_t n = recv(sock_, lenBuf, sizeof(lenBuf), 0);
        if (n <= 0 && errno != EAGAIN)
            throw std::runtime_error("Failed to read message length");
        int msgLen = BytesToInt(std::string_view(lenBuf, n));
        std::cout << "Expecting " << msgLen << " bytes" << std::endl;
        return ReceiveData(msgLen);
    }
}

void TcpConnect::CloseConnection() {
    close(sock_);
    mode = 2;
}

const std::string& TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}