#include "publish.h"
#include "packet.h"
#include "properties.h"

namespace packet {
  PublishEncoder::PublishEncoder(PublishPacket sp) : publishPkt(sp) {}

  std::vector<uint8_t> PublishEncoder::encode() const {
    const mqtt::Publish& p = this->publishPkt.second;

    uint32_t propertySize = this->propertySize();
    uint32_t remainingLength =
        propertySize + EncodedVarUint32::size(propertySize);
    remainingLength += uint32_t(p.topicName.size() + 2 + p.payload.size());
    if (p.qosLevel > 0) {
      remainingLength += 2;
    }

    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));
    uint8_t byte0 = static_cast<uint8_t>(
        static_cast<uint32_t>(ControlPacket::Type::PUBLISH) << 4);
    if (p.isDup) {
      byte0 |= (1 << 3);
    }
    byte0 |= (p.qosLevel << 1);
    if (p.hasRetain) {
      byte0 |= 1;
    }

    enc.write(byte0);
    enc.writeVarUint32(remainingLength);

    // topic name
    enc.write(p.topicName);
    if (p.qosLevel > 0) {
      enc.write(this->publishPkt.first);
    }
    this->encodeProperties(enc, propertySize);

    enc.writeBinaryDataNoLen(p.payload);

    return enc.getBuffer();
  }

  uint32_t PublishEncoder::propertySize() const {
    if (!this->publishPkt.second.properties) {
      return 0;
    }
    const mqtt::Publish::Properties& props =
        *this->publishPkt.second.properties;

    uint32_t propertySize = 0;
    propertySize += Property::size(props.payloadFormatIndicator);
    propertySize += Property::size(props.messageExpiryInterval);
    propertySize += Property::size(props.topicAlias);
    propertySize += Property::size(props.responseTopic);
    propertySize += Property::size(props.correlationData);
    propertySize += Property::sizeMoreThanOnce<std::vector<uint32_t>, true>(
        props.subscriptionIdentifiers);
    propertySize += Property::size(props.contentType);

    return propertySize;
  }

  void PublishEncoder::encodeProperties(Encoder& enc,
                                        uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->publishPkt.second.properties) {
      const mqtt::Publish::Properties& props =
          *this->publishPkt.second.properties;
      Property::encode(enc, Property::ID::PayloadFormatIndicatorID,
                       props.payloadFormatIndicator);
      Property::encode(enc, Property::ID::MessageExpiryIntervalID,
                       props.messageExpiryInterval);
      Property::encode(enc, Property::ID::TopicAliasID, props.topicAlias);
      Property::encode(enc, Property::ID::ResponseTopicID, props.responseTopic);
      Property::encode(enc, Property::ID::CorrelationDataID,
                       props.correlationData);
      for (auto v : props.subscriptionIdentifiers) {
        enc.writeVarUint32(
            static_cast<uint32_t>(Property::ID::SubscriptionIdentifierID));
        enc.writeVarUint32(v);
      }
      Property::encode<std::vector<uint32_t>, true>(
          enc, Property::ID::SubscriptionIdentifierID,
          props.subscriptionIdentifiers);
      Property::encode(enc, Property::ID::ContentTypeID, props.contentType);
    }
  }

  //   //
  //   ---------------------------------------------------------------------------

  PublishPacket PublishDecoder::decode(std::vector<uint8_t> buffer,
                                       uint8_t byte0) {
    uint32_t remainingLen = uint32_t(buffer.size());
    Decoder dec(buffer);
    return PublishDecoder::decode(dec, byte0, remainingLen);
  }

  PublishPacket PublishDecoder::decode(Decoder& dec, uint8_t byte0,
                                       uint32_t remainingLen) {
    mqtt::Publish p;
    p.qosLevel = ((byte0 >> 1) & 0x03);
    p.isDup = (byte0 & 0x08);
    p.hasRetain = (byte0 & 0x01);

    p.topicName = dec.read<std::string>();
    remainingLen -= uint32_t(p.topicName.size() + 2);
    uint16_t packetID = 0;
    if (p.qosLevel > 0) {
      packetID = dec.read<uint16_t>();
      remainingLen -= 2;
    }
    auto result = PublishDecoder::decodeProperties(dec);
    remainingLen -= result.second;
    p.properties = result.first;

    p.payload = dec.readBinaryDataNoLen(remainingLen);
    return {packetID, p};
  }

  std::pair<std::shared_ptr<mqtt::Publish::Properties>, uint32_t>
  PublishDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    uint32_t consumed = EncodedVarUint32::size(propertySize) + propertySize;

    std::shared_ptr<mqtt::Publish::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::Publish::Properties>();
    }

    while (propertySize > 0) {
      uint32_t val = dec.read<uint32_t, true>();
      propertySize -= EncodedVarUint32::size(val);
      Property::ID id = static_cast<Property::ID>(val);

      switch (id) {
      case Property::ID::PayloadFormatIndicatorID:
        propertySize -=
            Property::decode(dec, id, props->payloadFormatIndicator);
        break;
      case Property::ID::MessageExpiryIntervalID:
        propertySize -= Property::decode(dec, id, props->messageExpiryInterval);
        break;
      case Property::ID::TopicAliasID:
        propertySize -= Property::decode(dec, id, props->topicAlias);
        break;
      case Property::ID::ResponseTopicID:
        propertySize -= Property::decode(dec, id, props->responseTopic);
        break;
      case Property::ID::CorrelationDataID:
        propertySize -= Property::decode(dec, id, props->correlationData);
        break;
      case Property::ID::SubscriptionIdentifierID: {
        uint32_t value = dec.read<uint32_t, true>();
        props->subscriptionIdentifiers.push_back(value);
        propertySize -= Property::valueSize<uint32_t, true>(value);
      } break;
      case Property::ID::ContentTypeID:
        propertySize -= Property::decode(dec, id, props->contentType);
        break;
      default:
        throwInvalidPropertyID(id, "PUBLISH");
      }
    }

    return {props, consumed};
  }
} // namespace packet
