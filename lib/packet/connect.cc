#include "connect.h"
#include "codec.h"
#include "packet.h"
#include "properties.h"

namespace packet {
  ConnectEncoder::ConnectEncoder(const mqtt::Connect& c) : connect(c) {}

  std::vector<uint8_t> ConnectEncoder::encode() const {
    uint32_t propertySize = this->propertySize();
    // calculate the remaining length
    // 10 = protocolname + version + flags + keepalive
    uint32_t remainingLength = 10 + propertySize +
                               EncodedVarUint32::size(propertySize) +
                               uint32_t(2 + this->connect.clientID.size());
    uint8_t connectFlags = 0;
    if (this->connect.cleanStart) {
      connectFlags |= 0x02;
    }

    if (this->connect.userName.size() > 0) {
      connectFlags |= 0x80;
      remainingLength += uint32_t(2 + this->connect.userName.size());
    }

    if (this->connect.password.size() > 0) {
      connectFlags |= 0x40;
      remainingLength += uint32_t(2 + this->connect.password.size());
    }

    Encoder enc(remainingLength + 1 + EncodedVarUint32::size(remainingLength));
    enc.write(static_cast<uint8_t>(
        static_cast<uint32_t>(ControlPacket::Type::CONNECT) << 4));
    enc.writeVarUint32(remainingLength);

    enc.write(std::vector<uint8_t>{'M', 'Q', 'T', 'T'});
    enc.write(uint8_t(0x05)); // version
    enc.write(connectFlags);
    enc.write(this->connect.keepAlive);

    // encode properties
    this->encodeProperties(enc, propertySize);

    enc.write(this->connect.clientID);

    if (this->connect.userName.size() > 0) {
      enc.write(this->connect.userName);
    }

    if (this->connect.password.size() > 0) {
      enc.write(this->connect.password);
    }

    return enc.getBuffer();
  }

  uint32_t ConnectEncoder::propertySize() const {
    uint32_t propertySize = 0;
    if (this->connect.properties) {
      const mqtt::Connect::Properties& props = *this->connect.properties;
      propertySize += Property::size(props.sessionExpiryInterval);
      propertySize += Property::size(props.receiveMaximum);
      propertySize += Property::size(props.maximumPacketSize);
      propertySize += Property::size(props.topicAliasMaximum);
      propertySize += Property::size(props.requestProblemInfo);
      propertySize += Property::size(props.requestResponseInfo);
      propertySize += Property::size(props.authenticationMethod);
      propertySize += Property::size(props.authenticationData);
    }
    return propertySize;
  }

  void ConnectEncoder::encodeProperties(Encoder& enc,
                                        uint32_t propertySize) const {
    enc.writeVarUint32(propertySize);

    if (this->connect.properties) {
      const mqtt::Connect::Properties& props = *this->connect.properties;
      Property::encode(enc, Property::ID::SessionExpiryIntervalID,
                       props.sessionExpiryInterval);
      Property::encode(enc, Property::ID::ReceiveMaximumID,
                       props.receiveMaximum);
      Property::encode(enc, Property::ID::MaximumPacketSizeID,
                       props.maximumPacketSize);
      Property::encode(enc, Property::ID::TopicAliasMaximumID,
                       props.topicAliasMaximum);
      Property::encode(enc, Property::ID::RequestProblemInfoID,
                       props.requestProblemInfo);
      Property::encode(enc, Property::ID::RequestResponseInfoID,
                       props.requestResponseInfo);
      Property::encode(enc, Property::ID::AuthenticationMethodID,
                       props.authenticationMethod);
      Property::encode(enc, Property::ID::AuthenticationDataID,
                       props.authenticationData);
    }
  }

  // ----------------------------------------------------------------------

  mqtt::Connect ConnectDecoder::decode(std::vector<uint8_t> buffer) {
    Decoder dec(buffer);
    return ConnectDecoder::decode(dec);
  }

  mqtt::Connect ConnectDecoder::decode(Decoder& dec) {
    std::string protocolName = dec.read<std::string>();
    if (protocolName != "MQTT") {
      throw std::runtime_error(
          "CONNECT: received protocol name is invalid, must be MQTT");
    }

    if (dec.read<uint8_t>() != 0x05) {
      throw std::runtime_error(
          "CONNECT: received MQTT protocol version is invalid, must be 0x05");
    }
    mqtt::Connect c;
    c.protocolName = "MQTT";

    uint8_t connectFlag = dec.read<uint8_t>();

    c.keepAlive = dec.read<uint16_t>();
    c.properties = ConnectDecoder::decodeProperties(dec);
    c.clientID = dec.read<std::string>();
    // Username flag present?
    if (connectFlag & 0x80) {
      c.userName = dec.read<std::string>();
    }

    // password flag present?
    if (connectFlag & 0x80) {
      c.password = dec.read<std::vector<uint8_t>>();
    }

    return c;
  }

  std::shared_ptr<mqtt::Connect::Properties>
  ConnectDecoder::decodeProperties(Decoder& dec) {
    uint32_t propertySize = dec.read<uint32_t, true>();

    std::shared_ptr<mqtt::Connect::Properties> props;
    if (propertySize > 0) {
      props = std::make_shared<mqtt::Connect::Properties>();
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
      case Property::ID::MaximumPacketSizeID:
        propertySize -= Property::decode(dec, id, props->maximumPacketSize);
        break;
      case Property::ID::TopicAliasMaximumID:
        propertySize -= Property::decode(dec, id, props->topicAliasMaximum);
        break;
      case Property::ID::RequestProblemInfoID:
        propertySize -= Property::decode(dec, id, props->requestProblemInfo);
        break;
      case Property::ID::AuthenticationMethodID:
        propertySize -= Property::decode(dec, id, props->authenticationMethod);
        break;
      case Property::ID::AuthenticationDataID:
        propertySize -= Property::decode(dec, id, props->authenticationData);
        break;
      default:
        throwInvalidPropertyID(id, "CONNECT");
      }
    };
    return props;
  }

} // namespace packet
