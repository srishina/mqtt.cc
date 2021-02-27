#pragma once

#include <mqtt/connect.h>
#include <mqtt/noncopyable.h>
#include <vector>

namespace packet {
  class Encoder;
  class Decoder;

  class ConnectEncoder : public mqtt::noncopyable {
  public:
    explicit ConnectEncoder(const mqtt::Connect& c);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertyLen) const;

  private:
    const mqtt::Connect& connect;
  };

  class ConnectDecoder {
  public:
    static mqtt::Connect decode(std::vector<uint8_t> buffer);
    static mqtt::Connect decode(Decoder& dec);

  private:
    static std::shared_ptr<mqtt::Connect::Properties>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
