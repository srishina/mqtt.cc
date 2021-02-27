#include "suback.h"
#include "codec.h"
#include "packet.h"
#include "properties.h"
#include <mqtt/suback.h>

namespace packet {
  SubAckEncoder::SubAckEncoder(SubAckPacket sp) : subackPkt(sp) {}

  std::vector<uint8_t> SubAckEncoder::encode() const {
    uint32_t propertySize = this->propertySize();

    // calculate the remaining length
    // 2 = packet ID
    uint32_t remainingLength =
        2 + propertySize + EncodedVarUint32::size(propertySize) +
        uint32_t(this->subackPkt.second.reasonCodes.size());

    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));

    enc.write(static_cast<uint8_t>(
        static_cast<uint32_t>(ControlPacket::Type::SUBACK) << 4));
    enc.writeVarUint32(remainingLength);

    enc.write(this->subackPkt.first);
    this->encodeProperties(enc, propertySize);
    for (const auto rc : this->subackPkt.second.reasonCodes) {
      enc.write(static_cast<uint8_t>(rc));
    }

    return enc.getBuffer();
  }

  uint32_t SubAckEncoder::propertySize() const {
    if (!this->subackPkt.second.properties) {
      return 0;
    }

    const mqtt::SubAck::Properties& props = *this->subackPkt.second.properties;
    uint32_t propertySize = 0;
    propertySize += Property::size(props.reasonString);

    return propertySize;
  }

  void SubAckEncoder::encodeProperties(Encoder& enc,
                                       uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->subackPkt.second.properties) {
      const mqtt::SubAck::Properties& props =
          *this->subackPkt.second.properties;
      Property::encode(enc, Property::ID::ReasonStringID, props.reasonString);
    }
  }

  SubAckPacket SubAckDecoder::decode(std::vector<uint8_t> buffer) {
    uint32_t remainingLen = uint32_t(buffer.size());
    Decoder dec(buffer);
    return SubAckDecoder::decode(dec, remainingLen);
  }

  SubAckPacket SubAckDecoder::decode(Decoder& dec, uint32_t remainingLen) {
    uint16_t packetID = dec.read<uint16_t>();
    mqtt::SubAck sa;
    auto result = SubAckDecoder::decodeProperties(dec);
    sa.properties = result.first;
    remainingLen -= (2 + result.second);

    const std::vector<uint8_t> reasonCodes =
        dec.readBinaryDataNoLen(remainingLen);
    sa.reasonCodes.resize(reasonCodes.size());
    for (size_t i = 0; i < reasonCodes.size(); ++i) {
      sa.reasonCodes[i] = static_cast<mqtt::SubAck::ReasonCode>(reasonCodes[i]);
    }

    return {packetID, sa};
  }

  std::pair<std::shared_ptr<mqtt::SubAck::Properties>, uint32_t>
  SubAckDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    uint32_t consumed = EncodedVarUint32::size(propertySize) + propertySize;

    std::shared_ptr<mqtt::SubAck::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::SubAck::Properties>();
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
    return {props, consumed};
  }

} // namespace packet
