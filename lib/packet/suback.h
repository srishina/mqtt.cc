#pragma once

#include <mqtt/noncopyable.h>
#include <mqtt/suback.h>
#include <vector>

namespace packet {
  class Encoder;
  class Decoder;

  using SubAckPacket = std::pair<uint16_t, mqtt::SubAck>;

  class SubAckEncoder : public mqtt::noncopyable {
  public:
    explicit SubAckEncoder(SubAckPacket sp);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertyLen) const;

  private:
    SubAckPacket subackPkt;
  };

  class SubAckDecoder {
  public:
    static SubAckPacket decode(std::vector<uint8_t> buffer);
    static SubAckPacket decode(Decoder& dec, uint32_t remainingLen);

  private:
    static std::pair<std::shared_ptr<mqtt::SubAck::Properties>, uint32_t>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
