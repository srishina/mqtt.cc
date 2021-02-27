#include "unsubscribe.h"
#include "codec.h"
#include "properties.h"

namespace packet {
  UnsubscribeEncoder::UnsubscribeEncoder(UnsubscribePacket sp)
      : unsubscribePkt(sp) {}

  std::vector<uint8_t> UnsubscribeEncoder::encode() const {
    const uint8_t fhdr = 0xA2;
    uint32_t propertySize = this->propertySize();

    // calculate the remaining length
    // 2 = packet ID
    uint32_t remainingLength =
        uint32_t(2 + propertySize + EncodedVarUint32::size(propertySize));

    for (const auto& tf : this->unsubscribePkt.second.topicFilters) {
      remainingLength += uint32_t(tf.size() + 2);
    }
    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));
    enc.write(fhdr);
    enc.writeVarUint32(remainingLength);

    // write packet ID
    enc.write(this->unsubscribePkt.first);

    this->encodeProperties(enc, propertySize);
    for (const auto& tf : this->unsubscribePkt.second.topicFilters) {
      enc.write(tf);
    }

    return enc.getBuffer();
  }

  uint32_t UnsubscribeEncoder::propertySize() const {
    if (!this->unsubscribePkt.second.properties) {
      return 0;
    }

    uint32_t propertySize = 0;
    // todo.. implement
    return propertySize;
  }

  void UnsubscribeEncoder::encodeProperties(Encoder& enc,
                                            uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->unsubscribePkt.second.properties) {
      // todo.. implement
    }
  }

  UnsubscribePacket UnsubscribeDecoder::decode(std::vector<uint8_t> buffer) {
    uint32_t remainingLen = uint32_t(buffer.size());
    Decoder dec(buffer);
    return UnsubscribeDecoder::decode(dec, remainingLen);
  }

  UnsubscribePacket UnsubscribeDecoder::decode(Decoder& dec,
                                               uint32_t remainingLen) {
    uint16_t packetID = dec.read<uint16_t>();
    mqtt::Unsubscribe us;
    auto result = UnsubscribeDecoder::decodeProperties(dec);
    us.properties = result.first;
    remainingLen -= (2 + result.second);
    while (remainingLen > 0) {
      std::string tf = dec.read<std::string>();
      remainingLen -= uint32_t(tf.size() + 2);
      us.topicFilters.emplace_back(tf);
    }

    return {packetID, us};
  }

  std::pair<std::shared_ptr<mqtt::Unsubscribe::Properties>, uint32_t>
  UnsubscribeDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    uint32_t consumed = EncodedVarUint32::size(propertySize) + propertySize;

    std::shared_ptr<mqtt::Unsubscribe::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::Unsubscribe::Properties>();
    }

    while (propertySize > 0) {
      uint32_t val = dec.read<uint32_t, true>();
      propertySize -= EncodedVarUint32::size(val);
      Property::ID id = static_cast<Property::ID>(val);
      switch (id) {
      default:
        throwInvalidPropertyID(id, "UNSUBSCRIBE");
      }
    }
    return {props, consumed};
  }
} // namespace packet
