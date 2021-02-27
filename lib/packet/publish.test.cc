#include "codec.h"
#include "doctest/doctest.h"
#include "packet.h"
#include "properties.h"
#include "publish.h"

using namespace packet;

TEST_CASE("testing PUBLISH codec - enc/dec") {
  // clang-format off
  std::vector<uint8_t> encoded = {
      0x3B,     // PUBPACKID, DUP, 1, RETAIN
      0x0D, 
      0x00, 0x03, 'a', '/', 'b', 
      0x00, 0x12, // Packet identifier 18
      0x00,     // no properties
      'h',  'e',  'l',  'l', 'o',
  };
  // clang-format on
  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::PUBLISH == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x0D) == fhdr.second);

  const auto publishPkt = PublishDecoder::decode(dec, fhdr.first, fhdr.second);
  const mqtt::Publish& p = publishPkt.second;
  REQUIRE(publishPkt.first == uint16_t(0x12));
  REQUIRE(p.qosLevel == 1);
  REQUIRE(p.isDup);
  REQUIRE(p.hasRetain);
  REQUIRE(p.topicName == "a/b");
  REQUIRE(p.payload == std::vector<uint8_t>{'h', 'e', 'l', 'l', 'o'});

  std::vector<uint8_t> buffer = PublishEncoder(publishPkt).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing PUBLISH codec - enc/dec with QoS 0") {
  const std::vector<uint8_t> payload = {'W', 'e', 'l', 'c', 'c', 'o', 'm', 'e'};
  // clang-format off
  std::vector<uint8_t> encoded = {
		0x31, // PUBLISH, NO-DUP, 0, RETAIN
		0x0E,
		0x00, 0x03, 'a', '/', 'b',
		0x00, // no properties
  };
  encoded.insert(std::end(encoded), std::begin(payload), std::end(payload));
  // clang-format on
  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::PUBLISH == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x0E) == fhdr.second);

  const auto publishPkt = PublishDecoder::decode(dec, fhdr.first, fhdr.second);
  const mqtt::Publish& p = publishPkt.second;
  REQUIRE(publishPkt.first == 0);
  REQUIRE(p.qosLevel == 0);
  REQUIRE(!p.isDup);
  REQUIRE(p.hasRetain);
  REQUIRE(p.topicName == "a/b");
  REQUIRE(p.payload == payload);

  std::vector<uint8_t> buffer = PublishEncoder(publishPkt).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing PUBLISH codec - enc/dec with properties") {
  const uint8_t topicAlias = uint8_t(0x10);
  const std::vector<uint8_t> payload = {'W', 'e', 'l', 'c', 'c', 'o', 'm', 'e'};
  // clang-format off
  std::vector<uint8_t> encoded = {
      0x3D, // PUBPACKID, DUP, 2, RETAIN
      0x13, 
      0x00, 0x03, 'a', '/', 'b',
      0x00, 0x12, // Packet identifier 18
      0x03, 
      0x23, 0x00, topicAlias,
  };
  encoded.insert(std::end(encoded), std::begin(payload), std::end(payload));
  // clang-format on
  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::PUBLISH == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x13) == fhdr.second);

  const auto publishPkt = PublishDecoder::decode(dec, fhdr.first, fhdr.second);
  const mqtt::Publish& p = publishPkt.second;
  REQUIRE(publishPkt.first == uint16_t(0x12));
  REQUIRE(p.qosLevel == 2);
  REQUIRE(p.isDup);
  REQUIRE(p.hasRetain);
  REQUIRE(p.topicName == "a/b");
  REQUIRE(p.payload == payload);
  REQUIRE(p.properties);
  REQUIRE(p.properties->topicAlias);
  REQUIRE(*p.properties->topicAlias == topicAlias);

  std::vector<uint8_t> buffer = PublishEncoder(publishPkt).encode();
  REQUIRE(buffer == encoded);
}
