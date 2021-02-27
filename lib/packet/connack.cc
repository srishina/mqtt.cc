#include "connack.h"
#include "codec.h"
#include "packet.h"
#include "properties.h"

namespace packet {
  ConnAckEncoder::ConnAckEncoder(const mqtt::ConnAck& ca) : connack(ca) {}

  std::vector<uint8_t> ConnAckEncoder::encode() const {
    uint32_t propertySize = this->propertySize();
    // calculate the remaining length
    // 2 = session present + reason code
    uint32_t remainingLength =
        2 + propertySize + EncodedVarUint32::size(propertySize);
    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));
    enc.write(static_cast<uint8_t>(
        static_cast<uint32_t>(ControlPacket::Type::CONNACK) << 4));
    enc.writeVarUint32(remainingLength);

    enc.write(this->connack.sessionPresent);
    enc.write(static_cast<uint8_t>(this->connack.reasonCode));

    this->encodeProperties(enc, propertySize);

    return enc.getBuffer();
  }

  uint32_t ConnAckEncoder::propertySize() const {
    if (!this->connack.properties) {
      return 0;
    }

    const mqtt::ConnAck::Properties& props = *this->connack.properties;
    uint32_t propertySize = 0;

    propertySize += Property::size(props.sessionExpiryInterval);
    propertySize += Property::size(props.receiveMaximum);
    propertySize += Property::size(props.maximumQoS);
    propertySize += Property::size(props.retainAvailable);
    propertySize += Property::size(props.maximumPacketSize);
    propertySize += Property::size(props.assignedClientIdentifier);
    propertySize += Property::size(props.topicAliasMaximum);
    propertySize += Property::size(props.reasonString);
    propertySize += Property::size(props.wildcardSubscriptionAvailable);
    propertySize += Property::size(props.subscriptionIdentifierAvailable);
    Property::size(props.sharedSubscriptionAvailable);
    Property::size(props.responseInformation);
    Property::size(props.serverReference);
    Property::size(props.authenticationMethod);
    Property::size(props.authenticationData);

    return propertySize;
  }

  void ConnAckEncoder::encodeProperties(Encoder& enc,
                                        uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->connack.properties) {
      const mqtt::ConnAck::Properties& props = *this->connack.properties;

      Property::encode(enc, Property::ID::SessionExpiryIntervalID,
                       props.sessionExpiryInterval);
      Property::encode(enc, Property::ID::ReceiveMaximumID,
                       props.receiveMaximum);
      Property::encode(enc, Property::ID::MaximumQoSID, props.maximumQoS);
      Property::encode(enc, Property::ID::RetainAvailableID,
                       props.retainAvailable);
      Property::encode(enc, Property::ID::MaximumPacketSizeID,
                       props.maximumPacketSize);
      Property::encode(enc, Property::ID::AssignedClientIdentifierID,
                       props.assignedClientIdentifier);
      Property::encode(enc, Property::ID::TopicAliasMaximumID,
                       props.topicAliasMaximum);
      Property::encode(enc, Property::ID::ReasonStringID, props.reasonString);
      Property::encode(enc, Property::ID::WildcardSubscriptionAvailableID,
                       props.wildcardSubscriptionAvailable);
      Property::encode(enc, Property::ID::SubscriptionIdentifierAvailableID,
                       props.subscriptionIdentifierAvailable);
      Property::encode(enc, Property::ID::SharedSubscriptionAvailableID,
                       props.sharedSubscriptionAvailable);
      Property::encode(enc, Property::ID::ResponseInformationID,
                       props.responseInformation);
      Property::encode(enc, Property::ID::ServerReferenceID,
                       props.serverReference);
      Property::encode(enc, Property::ID::AuthenticationMethodID,
                       props.authenticationMethod);
      Property::encode(enc, Property::ID::AuthenticationDataID,
                       props.authenticationData);
    }
  }

  mqtt::ConnAck ConnAckDecoder::decode(std::vector<uint8_t> buffer) {
    Decoder dec(buffer);
    return ConnAckDecoder::decode(dec);
  }

  mqtt::ConnAck ConnAckDecoder::decode(Decoder& dec) {
    mqtt::ConnAck ca;

    ca.sessionPresent = dec.read<bool>();
    ca.reasonCode = static_cast<mqtt::ConnAck::ReasonCode>(dec.read<uint8_t>());
    ca.properties = ConnAckDecoder::decodeProperties(dec);

    return ca;
  }

  std::shared_ptr<mqtt::ConnAck::Properties>
  ConnAckDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    std::shared_ptr<mqtt::ConnAck::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::ConnAck::Properties>();
    }

    while (propertySize > 0) {
      uint32_t val = dec.read<uint32_t, true>();
      propertySize -= EncodedVarUint32::size(val);
      Property::ID id = static_cast<Property::ID>(val);
      switch (id) {
      case Property::ID::SessionExpiryIntervalID:
        propertySize -= Property::decode(dec, id, props->sessionExpiryInterval);
        break;
      case Property::ID::ReceiveMaximumID:
        propertySize -= Property::decode(dec, id, props->receiveMaximum);
        break;
      case Property::ID::MaximumQoSID:
        propertySize -= Property::decode(dec, id, props->maximumQoS);
        break;
      case Property::ID::RetainAvailableID:
        propertySize -= Property::decode(dec, id, props->retainAvailable);
        break;
      case Property::ID::MaximumPacketSizeID:
        propertySize -= Property::decode(dec, id, props->maximumPacketSize);
        break;
      case Property::ID::AssignedClientIdentifierID:
        propertySize -=
            Property::decode(dec, id, props->assignedClientIdentifier);
        break;
      case Property::ID::TopicAliasMaximumID:
        propertySize -= Property::decode(dec, id, props->topicAliasMaximum);
        break;
      case Property::ID::ReasonStringID:
        propertySize -= Property::decode(dec, id, props->reasonString);
        break;
      case Property::ID::WildcardSubscriptionAvailableID:
        propertySize -=
            Property::decode(dec, id, props->wildcardSubscriptionAvailable);
        break;
      case Property::ID::SubscriptionIdentifierAvailableID:
        propertySize -=
            Property::decode(dec, id, props->subscriptionIdentifierAvailable);
        break;
      case Property::ID::SharedSubscriptionAvailableID:
        propertySize -=
            Property::decode(dec, id, props->sharedSubscriptionAvailable);
        break;
      case Property::ID::ResponseInformationID:
        propertySize -= Property::decode(dec, id, props->responseInformation);
        break;
      case Property::ID::ServerReferenceID:
        propertySize -= Property::decode(dec, id, props->serverReference);
        break;
      case Property::ID::AuthenticationMethodID:
        propertySize -= Property::decode(dec, id, props->authenticationMethod);
        break;
      case Property::ID::AuthenticationDataID:
        propertySize -= Property::decode(dec, id, props->authenticationData);
        break;
      default:
        throwInvalidPropertyID(id, "CONNACK");
      }
    }
    return props;
  }
} // namespace packet
