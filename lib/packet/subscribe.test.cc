#include "codec.h"
#include "doctest/doctest.h"
#include "packet.h"
#include "properties.h"
#include "subscribe.h"
#include "utils.h"
#include <mqtt/subscribe.h>

using namespace packet;

TEST_CASE("testing SUBSCRIBE codec - enc/dec") {
  // clang-format off
  std::vector<uint8_t> encoded = {
      0x82, 0x17, 
      0x00, 0x12,            // Packet identifier 18
      0x00,                              // no properties
      0x00, 0x03, 'a',  '/',  'b', 0x01, // a/b with QoS 1
      0x00, 0x03, 'c',  '/',  'd', 0x02, 0x00,
      0x05, 'e',  '/',  'f',  '/', 'g',  0x00,
  };
  // clang-format on
  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::SUBSCRIBE ==
          ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x17) == fhdr.second);

  const auto subscribePkt = SubscribeDecoder::decode(dec, fhdr.second);

  const mqtt::Subscribe& s = subscribePkt.second;
  REQUIRE(subscribePkt.first == uint16_t(0x12));
  REQUIRE(s.subscriptions.size() == 3);
  REQUIRE(s.subscriptions[0].topicFilter == "a/b");
  REQUIRE(s.subscriptions[0].qosLevel == 1);
  REQUIRE_FALSE(s.subscriptions[0].noLocal);
  REQUIRE_FALSE(s.subscriptions[0].retainAsPublished);
  REQUIRE(s.subscriptions[1].topicFilter == "c/d");
  REQUIRE(s.subscriptions[1].qosLevel == 2);
  REQUIRE(s.subscriptions[2].topicFilter == "e/f/g");
  REQUIRE(s.subscriptions[2].qosLevel == 0);

  std::vector<uint8_t> buffer = SubscribeEncoder(subscribePkt).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing SUBSCRIBE codec - enc/dec with props") {
  // clang-format off
  std::vector<uint8_t> encoded = {
		0x82, 0x0B,
		0x00, 0x12, // Packet identifier 18
		0x02,  // property size
    0x0B, 0x0A,
		0x00, 0x03, 0x61, 0x2F, 0x62, 0x01, // a/b with QoS 1
	};
  // clang-format on

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::SUBSCRIBE ==
          ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x0B) == fhdr.second);

  const auto subscribePkt = SubscribeDecoder::decode(dec, fhdr.second);

  const mqtt::Subscribe s = std::move(subscribePkt.second);
  REQUIRE(s.properties != nullptr);
  REQUIRE(s.properties->subscriptionIdentifier);
  REQUIRE(*s.properties->subscriptionIdentifier == 0x0A);

  std::vector<uint8_t> buffer = SubscribeEncoder(subscribePkt).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing SUBSCRIBE codec - enc/dec with invalid props") {
  // clang-format off
  std::vector<uint8_t> encoded = {
		0x82, 0x0E,
		0x00, 0x12, // Packet identifier 18
		0x05,  // property size
    0x0B, 0x0A,
    0x23, 0x00, 0x0A,
		0x00, 0x03, 0x61, 0x2F, 0x62, 0x01, // a/b with QoS 1
	};
  // clang-format on

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::SUBSCRIBE ==
          ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x0E) == fhdr.second);

  CHECK_THROWS_AS(SubscribeDecoder::decode(dec, fhdr.second),
                  std::runtime_error);
}
