#include "codec.h"
#include "doctest/doctest.h"
#include "packet.h"
#include "properties.h"
#include "suback.h"

using namespace packet;

TEST_CASE("testing SUBACK codec - enc/dec") {
  // clang-format off
	std::vector<uint8_t> encoded = {
		0x90, 0x05,
		0x00, 0x12, // Packet identifier 18
		0x00, // no properties
		toUnderlyingType(mqtt::SubAck::ReasonCode::GrantedQoS1),
		toUnderlyingType(mqtt::SubAck::ReasonCode::GrantedQoS2)
	};
  // clang-format on
  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::SUBACK == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x05) == fhdr.second);

  const auto subackPkt = SubAckDecoder::decode(dec, fhdr.second);
  const mqtt::SubAck& s = subackPkt.second;
  REQUIRE(subackPkt.first == uint16_t(0x12));
  REQUIRE(s.reasonCodes.size() == 2);
  REQUIRE(s.reasonCodes[0] == mqtt::SubAck::ReasonCode::GrantedQoS1);
  REQUIRE(s.reasonCodes[1] == mqtt::SubAck::ReasonCode::GrantedQoS2);

  std::vector<uint8_t> buffer = SubAckEncoder(subackPkt).encode();
  REQUIRE(buffer == encoded);
}
