#include "codec.h"
#include "connack.h"
#include "doctest/doctest.h"
#include "packet.h"
#include "properties.h"
#include "utils.h"
#include <mqtt/connack.h>

using namespace packet;

TEST_CASE("testing CONNACK codec - enc/dec") {
  std::vector<uint8_t> encoded = {
      0x20,
      0x03,
      0x01, // session present
      toUnderlyingType(mqtt::ConnAck::ReasonCode::NotAuthorized), // Reason code
      0x00, // no properties
  };

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::CONNACK == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x03) == fhdr.second);

  mqtt::ConnAck ca = ConnAckDecoder::decode(dec);
  REQUIRE(ca.sessionPresent);
  REQUIRE(ca.reasonCode == mqtt::ConnAck::ReasonCode::NotAuthorized);
  REQUIRE_FALSE(ca.properties);
  std::vector<uint8_t> buffer = ConnAckEncoder(ca).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing CONNACK codec - enc/dec with properties") {
  // clang-format off
  std::vector<uint8_t> encoded = std::vector<uint8_t>{
    0x20, 0x0B, // fixed header
    0x01,
    toUnderlyingType(mqtt::ConnAck::ReasonCode::NotAuthorized),
    0x08,
    toUnderlyingType(Property::ID::ReceiveMaximumID),
    0x00, 0x0A,
    toUnderlyingType(Property::ID::MaximumQoSID),
    0x01,
    toUnderlyingType(Property::ID::TopicAliasMaximumID),
    0x00, 0x0A,
  };
  // clang-format on
  Decoder dec(encoded);
  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::CONNACK == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x0B) == fhdr.second);
  mqtt::ConnAck ca = ConnAckDecoder::decode(dec);
  REQUIRE(ca.properties != nullptr);
  REQUIRE(ca.properties->receiveMaximum);
  REQUIRE(*ca.properties->receiveMaximum == uint16_t(0x0A));
  REQUIRE(ca.properties->maximumQoS);
  REQUIRE(*ca.properties->maximumQoS == uint8_t(0x01));
  REQUIRE(ca.properties->topicAliasMaximum);
  REQUIRE(*ca.properties->topicAliasMaximum == uint16_t(0x0A));

  std::vector<uint8_t> buffer = ConnAckEncoder(ca).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing CONNACK codec - enc/dec with invalid properties") {
  // clang-format off
  	std::vector<uint8_t> encoded = {0x20, 0x0D,
		0x01,                                 // session present
		toUnderlyingType(mqtt::ConnAck::ReasonCode::NotAuthorized),
		0x0A,
    toUnderlyingType(Property::ID::SessionExpiryIntervalID),
		0x00, 0x00, 0x00, 0x0A,
    toUnderlyingType(Property::ID::SessionExpiryIntervalID),
		0x00, 0x00, 0x00, 0x0A,
	};
  // clang-format on

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::CONNACK == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x0D) == fhdr.second);
  CHECK_THROWS_AS(ConnAckDecoder::decode(dec), std::runtime_error);
}
