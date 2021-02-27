#pragma once

#include <mqtt/noncopyable.h>
#include <mqtt/publish.h>

namespace packet {
  class Encoder;
  class Decoder;

  using PublishPacket = std::pair<uint16_t, mqtt::Publish>;

  class PublishEncoder : public mqtt::noncopyable {
  public:
    explicit PublishEncoder(PublishPacket sp);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertySize) const;

  private:
    PublishPacket publishPkt;
  };

  class PublishDecoder {
  public:
    static PublishPacket decode(std::vector<uint8_t> buffer, uint8_t byte0);
    static PublishPacket decode(Decoder& dec, uint8_t byte0,
                                uint32_t remainingLen);

  private:
    static std::pair<std::shared_ptr<mqtt::Publish::Properties>, uint32_t>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
