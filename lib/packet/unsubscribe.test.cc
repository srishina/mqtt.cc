#include "codec.h"
#include "doctest/doctest.h"
#include "packet.h"
#include "properties.h"
#include "unsubscribe.h"

using namespace packet;

TEST_CASE("testing UNSUBSCRIBE codec - enc/dec") {
  // clang-format off
  std::vector<uint8_t> encoded = {
      0xA2, 0x0F, 0x00, 0x10, // Packet identifier 16
      0x00,                   // no properties
      0x00, 0x03, 'f',  'o',  'o', 0x00, 0x05, 'h', 'e', 'l', 'l', 'o',
  };
  // clang-format on

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::UNSUBSCRIBE ==
          ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x0F) == fhdr.second);

  const auto unsubackPkt = UnsubscribeDecoder::decode(dec, fhdr.second);
  const mqtt::Unsubscribe& us = unsubackPkt.second;
  REQUIRE(unsubackPkt.first == uint16_t(0x10));
  REQUIRE(us.topicFilters.size() == 2);
  REQUIRE(us.topicFilters[0] == "foo");
  REQUIRE(us.topicFilters[1] == "hello");

  std::vector<uint8_t> buffer = UnsubscribeEncoder(unsubackPkt).encode();
  REQUIRE(buffer == encoded);
}
