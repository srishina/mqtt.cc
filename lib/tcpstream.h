#pragma once

#include <mqtt/stream.h>
#include <string>

struct sockaddr_in;

namespace mqttutils {
  class TCPStream : public mqtt::Stream {
  public:
    TCPStream(std::string hostAddr, int port);

  private:
    int open() override final;
    void close() override final;
    std::pair<std::vector<uint8_t>, int> readBytes(size_t len) override final;
    std::pair<size_t, int>
    writeBytes(const std::vector<uint8_t>& data) override final;
    bool isValid() const override final;

    int getAddrInfo(sockaddr_in* addrIn);

  private:
    int sockfd;
    std::string hostName;
    int port;
  };
} // namespace mqttutils
