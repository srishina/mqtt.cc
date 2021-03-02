#include "doctest/doctest.h"

#include "tcpstream.h"
#include <thread>

#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

namespace test {
  class EchoServer {

  public:
    static const int port = 3000;

    EchoServer();
    void start();
    void stop();
    void run();

  private:
    std::thread t;
    int sockfd;
  };

  EchoServer::EchoServer() : sockfd(-1) {}

  void EchoServer::start() {
    sockaddr_in servaddr = {};

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(EchoServer::port);

    if (bind(fd, reinterpret_cast<sockaddr*>(&servaddr), sizeof(servaddr)) !=
        0) {
      return;
    }

    if (listen(fd, 5) != 0) {
      return;
    }

    this->sockfd = fd;

    this->t = std::thread(&EchoServer::run, this);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  void EchoServer::stop() {
    if (sockfd != -1) {
      close(sockfd);
      t.join();
      sockfd = -1;
    }
  }

  void EchoServer::run() {
    // we accept only one connection
    sockaddr_in clientaddr = {};
    socklen_t clientaddrlen = sizeof(clientaddr);
    int newsock = accept(this->sockfd, reinterpret_cast<sockaddr*>(&clientaddr),
                         &clientaddrlen);
    if (newsock < 0) {
      return;
    }
    std::vector<uint8_t> buffer(128);
    for (;;) {
      ssize_t bytesRead = recv(newsock, buffer.data(), 128, 0);
      if (bytesRead == 0 || (bytesRead == -1 && errno != EINTR)) {
        close(newsock);
        return;
      }
      ssize_t bytesWritten =
          send(newsock, buffer.data(), static_cast<size_t>(bytesRead), 0);
      if (bytesWritten != bytesRead) {
        close(newsock);
        return;
      }
    }
  }

} // namespace test

TEST_CASE("TCP Stream simple connect/send/recv/close") {
  test::EchoServer svr;
  svr.start();

  std::unique_ptr<mqtt::Stream> stream =
      std::make_unique<mqttutils::TCPStream>("localhost", 3000);
  CHECK(stream->open() == 0);
  CHECK(stream->isValid());
  std::vector<uint8_t> buffer = {'A', 'B', 'C', 'D', 'E'};
  auto writeResult = stream->writeBytes(buffer);
  CHECK(writeResult.second == 0);
  CHECK(writeResult.first == buffer.size());
  auto readResult = stream->readBytes(buffer.size());
  CHECK(readResult.second == 0);
  CHECK(readResult.first == buffer);
  stream->close();

  svr.stop();
}
