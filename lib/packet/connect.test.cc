#include "codec.h"
#include "connect.h"
#include "doctest/doctest.h"
#include "packet.h"
#include <mqtt/connect.h>

using namespace packet;

TEST_CASE("testing CONNECT codec - invalid protocol name/version") {
  CHECK_THROWS_AS(
      ConnectDecoder::decode({0x00, 0x04, 'T', 'T', 'Q', 'M', 0x05}),
      std::runtime_error);
  CHECK_THROWS_AS(
      ConnectDecoder::decode({0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04}),
      std::runtime_error);
}

TEST_CASE("testing CONNECT codec - enc/dec") {
  std::vector<uint8_t> encoded = {
      0x10, 0x1B, 0x00, 0x04, 'M', 'Q', 'T', 'T',
      0x05, // protocol version
      0xC2, // Username=1, password=1, retain=0, qos=0, will=0, clean start=1,
            // reserved=0
      0x00,
      0x18, // Keep alive - 24
      0x00, // properties
      0x00,
      0x00,                                // client id
      0x00, 0x05, 'h', 'e', 'l', 'l', 'o', // username
      0x00, 0x05, 'w', 'o', 'r', 'l', 'd', // username
  };

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::CONNECT == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x1B) == fhdr.second);

  mqtt::Connect c = ConnectDecoder::decode(dec);
  REQUIRE(c.protocolName == "MQTT");
  REQUIRE(c.protocolVersion == 0x05);
  REQUIRE(c.cleanStart);
  REQUIRE(c.keepAlive == uint16_t(24));
  REQUIRE(c.clientID == "");
  REQUIRE(c.userName == "hello");
  REQUIRE(c.password == std::vector<uint8_t>{'w', 'o', 'r', 'l', 'd'});

  std::vector<uint8_t> buffer = ConnectEncoder(c).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing CONNECT codec - enc/dec with properties") {
  std::vector<uint8_t> encoded = {
      0x10, 0x23, 0x00, 0x04, 'M',  'Q',  'T', 'T',
      0x05, // protocol version
      0xC2, 0x00,
      0x18,             // Keep alive - 24
      0x08,             // properties
      0x21, 0x00, 0x0A, // receive maximum
      0x27, 0x00, 0x00, 0x04,
      0x00, // maximum packet size
      0x00,
      0x00, // client id
      0x00, 0x05, 0x68, 0x65, 0x6C, 0x6C,
      0x6F, // username - "hello"
      0x00, 0x05, 0x77, 0x6F, 0x72, 0x6C,
      0x64, // password - "world"
  };

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::CONNECT == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x23) == fhdr.second);
  mqtt::Connect c = ConnectDecoder::decode(dec);
  REQUIRE(c.properties != nullptr);
  REQUIRE(c.properties->receiveMaximum);
  REQUIRE(*c.properties->receiveMaximum == uint16_t(10));
  REQUIRE(c.properties->maximumPacketSize);
  REQUIRE(*c.properties->maximumPacketSize == uint32_t(1024));

  std::vector<uint8_t> buffer = ConnectEncoder(c).encode();
  REQUIRE(buffer == encoded);
}

TEST_CASE("testing CONNECT codec - enc/dec with invalid properties") {
  std::vector<uint8_t> encoded = {
      0x10, 0x23, 0x00, 0x04, 'M',  'Q',  'T',  'T',
      0x05, // protocol version
      0xC2, 0x00,
      0x18,                         // Keep alive - 24
      0x08,                         // properties
      0x23, 0x00, 0x0A,             // topic alias, invalid property for CONNECT
      0x27, 0x00, 0x00, 0x04, 0x00, // maximum packet size
      0x00, 0x00,                   // client id
      0x00, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, // username - "hello"
      0x00, 0x05, 0x77, 0x6F, 0x72, 0x6C, 0x64, // password - "world"
  };

  Decoder dec(encoded);

  FixedHeader fhdr = FixedHeaderReader::read(dec);
  REQUIRE(ControlPacket::Type::CONNECT == ControlPacket::Type(fhdr.first >> 4));
  REQUIRE(uint32_t(0x23) == fhdr.second);
  CHECK_THROWS_AS(ConnectDecoder::decode(dec), std::runtime_error);
}
