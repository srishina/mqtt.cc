#include "tcpstream.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

namespace mqttutils {
  TCPStream::TCPStream(std::string hostNameA, int portA)
      : sockfd(-1), hostName(hostNameA), port(portA) {}

  int TCPStream::open() {
    if (this->isValid()) {
      return 0;
    }

    sockaddr_in hostSockAddr = {};
    int result = this->getAddrInfo(&hostSockAddr);
    if (result != 0) {
      return result;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      return errno;
    }

    if (::connect(fd, reinterpret_cast<const sockaddr*>(&hostSockAddr),
                  sizeof(sockaddr)) != 0) {
      return errno;
    }

    this->sockfd = fd;
    return 0;
  }

  void TCPStream::close() {
    if (this->sockfd != 0) {
      ::close(this->sockfd);
      this->sockfd = -1;
    }
  }

  // reads from socket untill it has read len bytes
  // Returns the  bytes read and an error if fewer bytes
  // are read
  std::pair<std::vector<uint8_t>, int> TCPStream::readBytes(size_t len) {
    std::vector<uint8_t> buffer(len);
    size_t totalBytesRead = 0;
    while (totalBytesRead < len) {
      ssize_t bytesRead = recv(this->sockfd, buffer.data() + totalBytesRead,
                               len - totalBytesRead, 0);
      if (bytesRead > 0) {
        totalBytesRead += static_cast<size_t>(bytesRead);
      } else if (bytesRead == 0 || (bytesRead == -1 && errno != EINTR)) {
        buffer.resize(totalBytesRead);
        return {buffer, errno};
      }
    }
    return {buffer, 0};
  }

  // Writes all data from buffer to socket. Returns the total number
  // of bytes written and the error code. In there an error while writing
  // then the errno and the total the total number of bytes written till
  // that point will be returned
  std::pair<size_t, int>
  TCPStream::writeBytes(const std::vector<uint8_t>& buffer) {
    size_t size = buffer.size();
    size_t totalBytesWritten = 0;
    while (totalBytesWritten < size) {
      ssize_t bytesWritten =
          send(this->sockfd, buffer.data() + totalBytesWritten,
               size - totalBytesWritten, 0);
      if (bytesWritten >= 0) {
        totalBytesWritten += static_cast<size_t>(bytesWritten);
      } else if (bytesWritten == -1 && errno != EINTR) {
        return {totalBytesWritten, errno};
      }
    }
    return {totalBytesWritten, 0};
  }

  bool TCPStream::isValid() const {
    return this->sockfd != -1;
  }

  int TCPStream::getAddrInfo(sockaddr_in* addrIn) {
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::vector<char> cstr(this->hostName.c_str(),
                           this->hostName.c_str() + this->hostName.size() + 1);
    addrinfo* resultList = nullptr;
    int result = getaddrinfo(cstr.data(), nullptr, &hints, &resultList);
    if (result != 0) {
      return result;
    }
    memcpy(addrIn, resultList->ai_addr, sizeof(sockaddr_in));
    addrIn->sin_port = htons(this->port);

    freeaddrinfo(resultList);

    return 0;
  }

} // namespace mqttutils
