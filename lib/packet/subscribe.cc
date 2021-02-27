#include "subscribe.h"
#include "codec.h"
#include "properties.h"
#include <mqtt/subscribe.h>

namespace packet {

  SubscribeEncoder::SubscribeEncoder(SubscribePacket sp) : subscribePkt(sp) {}

  std::vector<uint8_t> SubscribeEncoder::encode() const {
    const uint8_t fhdr = 0x82; // 10000010
    uint32_t propertySize = this->propertySize();

    // calculate the remaining length
    // 2 = packet ID
    uint32_t remainingLength =
        2 + propertySize + EncodedVarUint32::size(propertySize);

    for (const auto& s : this->subscribePkt.second.subscriptions) {
      remainingLength += uint32_t(s.topicFilter.size() + 2 + 1);
    }

    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));
    enc.write(fhdr);
    enc.writeVarUint32(remainingLength);

    // write packet ID
    enc.write(this->subscribePkt.first);
    this->encodeProperties(enc, propertySize);

    for (const auto& s : this->subscribePkt.second.subscriptions) {
      enc.write(s.topicFilter);
      uint8_t b = 0;
      // write subscribe options
      b |= (s.qosLevel & 0x03);
      if (s.noLocal) {
        b |= 0x04;
      }
      if (s.retainAsPublished) {
        b |= 0x08;
      }

      b |= (s.retainHandling & 0x30);

      enc.write(b);
    }

    return enc.getBuffer();
  }

  uint32_t SubscribeEncoder::propertySize() const {
    if (!this->subscribePkt.second.properties) {
      return 0;
    }

    const mqtt::Subscribe::Properties& props =
        *this->subscribePkt.second.properties;
    uint32_t propertySize = 0;
    propertySize +=
        Property::size<decltype(props.subscriptionIdentifier), true>(
            props.subscriptionIdentifier);
    return propertySize;
  }

  void SubscribeEncoder::encodeProperties(Encoder& enc,
                                          uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->subscribePkt.second.properties) {
      const mqtt::Subscribe::Properties& props =
          *this->subscribePkt.second.properties;
      Property::encode<decltype(props.subscriptionIdentifier), true>(
          enc, Property::ID::SubscriptionIdentifierID,
          props.subscriptionIdentifier);
    }
  }

  SubscribePacket SubscribeDecoder::decode(std::vector<uint8_t> buffer) {
    uint32_t remainingLen = uint32_t(buffer.size());
    Decoder dec(buffer);
    return SubscribeDecoder::decode(dec, remainingLen);
  }

  SubscribePacket SubscribeDecoder::decode(Decoder& dec,
                                           uint32_t remainingLen) {
    uint16_t packetID = dec.read<uint16_t>();
    mqtt::Subscribe s;
    auto result = SubscribeDecoder::decodeProperties(dec);
    s.properties = result.first;
    remainingLen -= (2 + result.second);

    while (remainingLen > 0) {
      mqtt::Subscription sub;
      sub.topicFilter = dec.read<std::string>();
      uint8_t b = dec.read<uint8_t>();
      sub.qosLevel = (b & 0x03);
      sub.noLocal = ((b & 0x04) == 1);
      sub.retainAsPublished = ((b & 0x08) == 1);
      sub.retainHandling = (b & 0x30);
      s.subscriptions.emplace_back(sub);
      remainingLen -= uint32_t(sub.topicFilter.size() + 2 + 1);
    };

    return {packetID, s};
  }

  std::pair<std::shared_ptr<mqtt::Subscribe::Properties>, uint32_t>
  SubscribeDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    uint32_t consumed = EncodedVarUint32::size(propertySize) + propertySize;

    std::shared_ptr<mqtt::Subscribe::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::Subscribe::Properties>();
    }

    while (propertySize > 0) {
      uint32_t val = dec.read<uint32_t, true>();
      propertySize -= EncodedVarUint32::size(val);
      Property::ID id = static_cast<Property::ID>(val);
      switch (id) {
      case Property::ID::SubscriptionIdentifierID:
        propertySize -=
            Property::decode<decltype(props->subscriptionIdentifier), true>(
                dec, id, props->subscriptionIdentifier);
        break;
      default:
        throwInvalidPropertyID(id, "SUBSCRIBE");
      }
    }
    return {props, consumed};
  }

} // namespace packet
