#include "packet.h"
#include <mqtt/noncopyable.h>
#include <mqtt/publishresponse.h>

namespace packet {
  class Encoder;
  class Decoder;

  struct PublishResponsePacket {
    ControlPacket::Type type;
    uint16_t packetID;
    mqtt::PublishResponse response;
  };

  class PublishResponseEncoder : public mqtt::noncopyable {
  public:
    explicit PublishResponseEncoder(PublishResponsePacket sp);
    std::vector<uint8_t> encode() const;

  private:
    uint32_t propertySize() const;
    void encodeProperties(Encoder& enc, uint32_t propertyLen) const;

  private:
    PublishResponsePacket publishRespPkt;
  };

  class PublishResponseDecoder {
  public:
    static PublishResponsePacket decode(std::vector<uint8_t> buffer,
                                        ControlPacket::Type t);
    static PublishResponsePacket decode(Decoder& dec, ControlPacket::Type t,
                                        uint32_t remainingLen);

  private:
    static std::shared_ptr<mqtt::PublishResponse::Properties>
    decodeProperties(Decoder& dec);
  };
} // namespace packet
