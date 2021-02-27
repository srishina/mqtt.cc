#include "publishresponse.h"
#include "codec.h"
#include "properties.h"
#include <cstdint>

namespace packet {
  PublishResponseEncoder::PublishResponseEncoder(PublishResponsePacket sp)
      : publishRespPkt(sp) {}

  std::vector<uint8_t> PublishResponseEncoder::encode() const {
    uint32_t propertySize = this->propertySize();
    const mqtt::PublishResponse& resp = this->publishRespPkt.response;

    // calculate the remaining length
    uint32_t remainingLength = 2; // packet id
    // The Reason Code or Property Length or both can be omitted
    // if the Reason Code is 0x00 (Success) and there are no Properties, then
    // both can be omitted and the remaining length is 2 If the reason code is
    // not 0x00 and there are no properties then the remaining length is 3 If
    // the reason code is  0x00 and there are  properties then the remaining
    // length is greather than 3
    if (propertySize != 0) {
      remainingLength +=
          uint32_t(1 + propertySize + EncodedVarUint32::size(propertySize));
    } else if (resp.reasonCode != mqtt::PublishResponse::ReasonCode::Success) {
      remainingLength += 1;
    }

    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));
    uint8_t byte0 = 0;
    ControlPacket::Type t = this->publishRespPkt.type;
    if (t == ControlPacket::Type::PUBREL) {
      byte0 = 0x62; // 01100010
    } else {
      byte0 = static_cast<uint8_t>(static_cast<uint32_t>(t) << 4);
    }
    enc.write(byte0);
    enc.writeVarUint32(remainingLength);

    enc.write(this->publishRespPkt.packetID);
    if (remainingLength > 2) {
      // encode the reason code
      enc.write(static_cast<uint8_t>(resp.reasonCode));
      if (remainingLength > 3) {
        this->encodeProperties(enc, propertySize);
      }
    }

    return enc.getBuffer();
  }

  uint32_t PublishResponseEncoder::propertySize() const {
    if (!this->publishRespPkt.response.properties) {
      return 0;
    }

    const mqtt::PublishResponse::Properties& props =
        *this->publishRespPkt.response.properties;
    uint32_t propertySize = 0;
    propertySize += Property::size(props.reasonString);

    return propertySize;
  }

  void PublishResponseEncoder::encodeProperties(Encoder& enc,
                                                uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->publishRespPkt.response.properties) {
      const mqtt::PublishResponse::Properties& props =
          *this->publishRespPkt.response.properties;
      Property::encode(enc, Property::ID::ReasonStringID, props.reasonString);
    }
  }

  PublishResponsePacket
  PublishResponseDecoder::decode(std::vector<uint8_t> buffer,
                                 ControlPacket::Type t) {
    uint32_t remainingLen = uint32_t(buffer.size());
    Decoder dec(buffer);
    return PublishResponseDecoder::decode(dec, t, remainingLen);
  }

  PublishResponsePacket PublishResponseDecoder::decode(Decoder& dec,
                                                       ControlPacket::Type t,
                                                       uint32_t remainingLen) {
    PublishResponsePacket resp;
    resp.type = t;
    resp.packetID = dec.read<uint16_t>();
    if (remainingLen > 2) {
      resp.response.reasonCode =
          static_cast<mqtt::PublishResponse::ReasonCode>(dec.read<uint8_t>());
      if (remainingLen > 3) {
        resp.response.properties =
            PublishResponseDecoder::decodeProperties(dec);
      }
    }

    return resp;
  }

  std::shared_ptr<mqtt::PublishResponse::Properties>
  PublishResponseDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    std::shared_ptr<mqtt::PublishResponse::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::PublishResponse::Properties>();
    }

    while (propertySize > 0) {
      uint32_t val = dec.read<uint32_t, true>();
      propertySize -= EncodedVarUint32::size(val);
      Property::ID id = static_cast<Property::ID>(val);
      switch (id) {
      case Property::ID::ReasonStringID:
        propertySize -= Property::decode(dec, id, props->reasonString);
        break;
      default:
        throwInvalidPropertyID(id, "SUBACK");
      }
    }
    return props;
  }
} // namespace packet
