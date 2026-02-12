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
#include <cstdint>

// Константы:
// AF_INET - IPv4
// SOCK_STREAM - поток данных с использованием протокола TCP
// F_GETFL - для функции fcntl - получение флагов состояния ФД
// O_NONBLOCK - устанавливает или снимает флаг блокировки ФД
// POLLOUT - готовность на запись в poll()

// Структуры:
/*
 // sockaddr_in - структура для представления сетевого адреса IPv4
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET; // Семейство адресов (IPv4)
    serverAddress.sin_port = htons(8080); // Номер порта (8080)
    serverAddress.sin_addr.s_addr = INADDR_ANY; // IP-адрес (любой)
    
    // sockaddr - общая абстрактная структура для адреса сокета
    sockaddr genericAddress;
    genericAddress.sa_family = AF_INET; // Семейство адресов
    
    // pollfd - структура для описания файловых дескрипторов для poll()
    pollfd pollFileDescriptor;
    pollFileDescriptor.fd = 0; // Файловый дескриптор
    pollFileDescriptor.events = POLLIN; // События для отслеживания (ввод)
    pollFileDescriptor.revents = 0; // Возвращаемые события
    
    // Вывод
    std::cout << "Server Address Family: " << serverAddress.sin_family << std::endl;
    std::cout << "Server Port: " << ntohs(serverAddress.sin_port) << std::endl;
    std::cout << "Server IP Address: " << inet_ntoa(serverAddress.sin_addr) << std::endl;
    
    std::cout << "Generic Address Family: " << genericAddress.sa_family << std::endl;
    
    std::cout << "Poll File Descriptor FD: " << pollFileDescriptor.fd << std::endl;
    std::cout << "Poll File Descriptor Events: " << pollFileDescriptor.events << std::endl;
*/

TcpConnect::TcpConnect(std::string ip, int port,
                       std::chrono::milliseconds connectTimeout,
                       std::chrono::milliseconds readTimeout)
    : mode(0),
      ip_(std::move(ip)),
      port_(port),
      connectTimeout_(connectTimeout),
      readTimeout_(readTimeout)
{}

TcpConnect::~TcpConnect() {
    if (mode == 2) return;
    CloseConnection();
}

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
    const char* ptr = data.data();
    int remaining = data.size();

    while (remaining > 0) {
        int sent = send(sock_, ptr, remaining, 0);
        if (sent < 0)
            throw std::runtime_error("Send failed");
        ptr += sent;
        remaining -= sent;
    }
}

std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    /*
    1. SOL_SOCKET:
       - Константа SOL_SOCKET используется для указания уровня сокета, на котором устанавливаются или получаются параметры.
       - Обычно эта константа используется вместе с функцией setsockopt() для установки параметров сокета или с функцией getsockopt() 
         для получения параметров сокета.
       - Уровень SOL_SOCKET обычно указывает на общие параметры сокета, которые применяются ко всем типам сокетов.

    2. SO_RCVTIMEO:
       - Константа SO_RCVTIMEO используется для установки времени ожидания приема данных на сокете.
       - При установке этого параметра с помощью функции setsockopt() можно задать время ожидания в миллисекундах для операций 
         приема данных на сокете.
       - Если на сокете не поступают данные в течение указанного времени, операция чтения будет завершена и вернется 
         соответствующий результат (обычно это значение -1 или другое обозначение ошибки).
    */

    if (bufferSize > 0) {
        std::string data(bufferSize, 0);
        char* ptr = &data[0];
        int remaining = bufferSize;
        while (remaining > 0) {
            struct timeval tv;
            tv.tv_sec = readTimeout_.count() / 1000;
            tv.tv_usec = (readTimeout_.count() % 1000) * 1000;
            if (setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO,
                           (const char*)&tv, sizeof(tv)) < 0)
                throw std::runtime_error("Failed to set receive timeout");

            int n = recv(sock_, ptr, remaining, 0);
            if (n < 0)
                throw std::runtime_error("Receive error");
            if (n == 0)
                return "";
            ptr += n;
            remaining -= n;
        }
        return data;
    } else {
        char lenBuf[4];
        struct timeval tv;
        tv.tv_sec = readTimeout_.count() / 1000;
        tv.tv_usec = (readTimeout_.count() % 1000) * 1000;
        if (setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO,
                       (const char*)&tv, sizeof(tv)) < 0)
            throw std::runtime_error("Failed to set receive timeout");

        int n = recv(sock_, lenBuf, 4, 0);
        if (n < 4)
            throw std::runtime_error("Failed to read message length");
        int msgLen = BytesToInt(std::string_view(lenBuf, 4));
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