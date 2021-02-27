#pragma once

#pragma once

#include <mqtt/connack.h>
#include <mqtt/noncopyable.h>
#include <vector>

namespace packet {
  class Encoder;
  class Decoder;

  class ConnAckEncoder : public mqtt::noncopyable {
  public:
    explicit ConnAckEncoder(const mqtt::ConnAck& ca);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertyLen) const;

  private:
    const mqtt::ConnAck& connack;
  };

  class ConnAckDecoder {
  public:
    static mqtt::ConnAck decode(std::vector<uint8_t> buffer);
    static mqtt::ConnAck decode(Decoder& dec);

  private:
    static std::shared_ptr<mqtt::ConnAck::Properties>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
