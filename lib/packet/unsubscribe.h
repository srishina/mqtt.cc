#pragma once
#include <mqtt/noncopyable.h>
#include <mqtt/unsubscribe.h>

namespace packet {
  class Encoder;
  class Decoder;

  using UnsubscribePacket = std::pair<uint16_t, mqtt::Unsubscribe>;

  class UnsubscribeEncoder : public mqtt::noncopyable {
  public:
    explicit UnsubscribeEncoder(UnsubscribePacket sp);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertyLen) const;

  private:
    UnsubscribePacket unsubscribePkt;
  };

  class UnsubscribeDecoder {
  public:
    static UnsubscribePacket decode(std::vector<uint8_t> buffer);
    static UnsubscribePacket decode(Decoder& dec, uint32_t remainingLen);

  private:
    static std::pair<std::shared_ptr<mqtt::Unsubscribe::Properties>, uint32_t>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
