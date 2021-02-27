#pragma once

#include <mqtt/noncopyable.h>
#include <mqtt/unsuback.h>
#include <vector>

namespace packet {
  class Encoder;
  class Decoder;

  using UnsubAckPacket = std::pair<uint16_t, mqtt::UnsubAck>;

  class UnsubAckEncoder : public mqtt::noncopyable {
  public:
    explicit UnsubAckEncoder(UnsubAckPacket sp);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertyLen) const;

  private:
    UnsubAckPacket unsubackPkt;
  };

  class UnsubAckDecoder {
  public:
    static UnsubAckPacket decode(std::vector<uint8_t> buffer);
    static UnsubAckPacket decode(Decoder& dec, uint32_t remainingLen);

  private:
    static std::pair<std::shared_ptr<mqtt::UnsubAck::Properties>, uint32_t>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
