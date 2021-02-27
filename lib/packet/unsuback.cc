#include "unsuback.h"
#include "codec.h"
#include "packet.h"
#include "properties.h"

namespace packet {
  UnsubAckEncoder::UnsubAckEncoder(UnsubAckPacket sp) : unsubackPkt(sp) {}

  std::vector<uint8_t> UnsubAckEncoder::encode() const {
    uint32_t propertySize = this->propertySize();

    // calculate the remaining length
    // 2 = packet ID
    uint32_t remainingLength =
        2 + propertySize + EncodedVarUint32::size(propertySize) +
        uint32_t(this->unsubackPkt.second.reasonCodes.size());

    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));

    enc.write(static_cast<uint8_t>(
        static_cast<uint32_t>(ControlPacket::Type::UNSUBACK) << 4));
    enc.writeVarUint32(remainingLength);

    enc.write(this->unsubackPkt.first);
    this->encodeProperties(enc, propertySize);
    for (const auto rc : this->unsubackPkt.second.reasonCodes) {
      enc.write(static_cast<uint8_t>(rc));
    }

    return enc.getBuffer();
  }

  uint32_t UnsubAckEncoder::propertySize() const {
    if (!this->unsubackPkt.second.properties) {
      return 0;
    }

    const mqtt::UnsubAck::Properties& props =
        *this->unsubackPkt.second.properties;
    uint32_t propertySize = 0;
    propertySize += Property::size(props.reasonString);

    return propertySize;
  }

  void UnsubAckEncoder::encodeProperties(Encoder& enc,
                                         uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->unsubackPkt.second.properties) {
      const mqtt::UnsubAck::Properties& props =
          *this->unsubackPkt.second.properties;
      Property::encode(enc, Property::ID::ReasonStringID, props.reasonString);
    }
  }

  UnsubAckPacket UnsubAckDecoder::decode(std::vector<uint8_t> buffer) {
    uint32_t remainingLen = uint32_t(buffer.size());
    Decoder dec(buffer);
    return UnsubAckDecoder::decode(dec, remainingLen);
  }

  UnsubAckPacket UnsubAckDecoder::decode(Decoder& dec, uint32_t remainingLen) {
    uint16_t packetID = dec.read<uint16_t>();
    mqtt::UnsubAck sa;
    auto result = UnsubAckDecoder::decodeProperties(dec);
    sa.properties = result.first;
    remainingLen -= (2 + result.second);

    const std::vector<uint8_t> reasonCodes =
        dec.readBinaryDataNoLen(remainingLen);
    sa.reasonCodes.resize(reasonCodes.size());
    for (size_t i = 0; i < reasonCodes.size(); ++i) {
      sa.reasonCodes[i] =
          static_cast<mqtt::UnsubAck::ReasonCode>(reasonCodes[i]);
    }

    return {packetID, sa};
  }

  std::pair<std::shared_ptr<mqtt::UnsubAck::Properties>, uint32_t>
  UnsubAckDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    uint32_t consumed = EncodedVarUint32::size(propertySize) + propertySize;

    std::shared_ptr<mqtt::UnsubAck::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::UnsubAck::Properties>();
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
        throwInvalidPropertyID(id, "UNSUBACK");
      }
    }
    return {props, consumed};
  }

} // namespace packet
