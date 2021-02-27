#include "codec.h"
#include "doctest/doctest.h"
#include "packet.h"
#include "properties.h"
#include "unsuback.h"

using namespace packet;

TEST_CASE("testing UNSUBACK codec - enc/dec") {
  // clang-format off
	std::vector<uint8_t> encoded = {
		0xB0, 0x06,
		0x00, 0x10, // Packet identifier 16
		0x00, // no properties
        toUnderlyingType(mqtt::UnsubAck::ReasonCode::NoSubscriptionExisted),
        toUnderlyingType(mqtt::UnsubAck::ReasonCode::NotAuthorized),
		toUnderlyingType(mqtt::UnsubAck::ReasonCode::Success)
	};
  // clang-format on
  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::UNSUBACK ==
          ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x06) == fhdr.second);

  const auto subackPkt = UnsubAckDecoder::decode(dec, fhdr.second);
  const mqtt::UnsubAck& us = subackPkt.second;
  REQUIRE(subackPkt.first == uint16_t(0x10));
  REQUIRE(us.reasonCodes.size() == 3);
  REQUIRE(us.reasonCodes[0] ==
          mqtt::UnsubAck::ReasonCode::NoSubscriptionExisted);
  REQUIRE(us.reasonCodes[1] == mqtt::UnsubAck::ReasonCode::NotAuthorized);
  REQUIRE(us.reasonCodes[2] == mqtt::UnsubAck::ReasonCode::Success);

  std::vector<uint8_t> buffer = UnsubAckEncoder(subackPkt).encode();
  REQUIRE(buffer == encoded);
}
