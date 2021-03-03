#pragma once

#include <vector>

namespace mqtt {

  // Stream represents a bi-directonal stream that the MQTT client uses.
  // The implementation of the Stream is responsible for
  // initialization of the stream(tcp, ws etc...) with the broker.
  // WebsocketStream, TCPStream is provided as part of the library, other
  // streams can be written by the implementations
  // todo. add timeout to the APIs
  class Stream {
  public:
    virtual int open() = 0;
    virtual void close() = 0;
    virtual std::pair<std::vector<uint8_t>, int> readBytes(size_t len) = 0;
    virtual std::pair<size_t, int>
    writeBytes(const std::vector<uint8_t>& data) = 0;
    virtual bool isValid() const = 0;

    virtual ~Stream();
  };

} // namespace mqtt
