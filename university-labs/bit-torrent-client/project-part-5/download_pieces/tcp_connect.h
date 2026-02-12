#pragma once

#include <string>
#include <chrono>

class TcpConnect {
public:
    TcpConnect(std::string ip, int port,
               std::chrono::milliseconds connectTimeout,
               std::chrono::milliseconds readTimeout);
    ~TcpConnect();

    void EstablishConnection();
    void SendData(const std::string& data) const;
    std::string ReceiveData(size_t bufferSize = 0) const;
    void CloseConnection();

    const std::string& GetIp() const;
    int GetPort() const;

private:
    int mode; // 0 - not used, 1 - in use, 2 - closed
    const std::string ip_;
    const int port_;
    std::chrono::milliseconds connectTimeout_, readTimeout_;
    int sock_;
};