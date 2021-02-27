#pragma once

#include <mqtt/noncopyable.h>
#include <mqtt/subscribe.h>
#include <vector>

namespace packet {
  class Encoder;
  class Decoder;

  using SubscribePacket = std::pair<uint16_t, mqtt::Subscribe>;

  class SubscribeEncoder : public mqtt::noncopyable {
  public:
    explicit SubscribeEncoder(SubscribePacket sp);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertyLen) const;

  private:
    SubscribePacket subscribePkt;
  };

  class SubscribeDecoder {
  public:
    static SubscribePacket decode(std::vector<uint8_t> buffer);
    static SubscribePacket decode(Decoder& dec, uint32_t remainingLen);

  private:
    static std::pair<std::shared_ptr<mqtt::Subscribe::Properties>, uint32_t>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
