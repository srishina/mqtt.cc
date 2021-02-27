#include "codec.h"
#include "doctest/doctest.h"
#include "packet.h"
#include "properties.h"
#include "publishresponse.h"

using namespace packet;

TEST_CASE("testing PUBACK/PUBREC/PUBREL/PUBCOMP codec - enc/dec") {
  using RespPacket = std::vector<uint8_t>;
  // clang-format off
  std::vector<uint8_t> encodedPubAck = {
    0x40, // PUBACK,
    0x03,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::NoMatchingSubscribers),
  };
  
  std::vector<uint8_t> encodedPubRec = {
    0x50, // PUBREC
    0x03,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::QuotaExceeded),
  };
  
  std::vector<uint8_t> encodedPubRel = {
    0x62, // PUBREL,
    0x03,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::PacketIdentifierInUse),
  };
  
  std::vector<uint8_t> encodedPubComp = {
    0x70, // PUBCOMP,
    0x03,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::PacketIdentifierInUse),
  };  

  std::vector<ControlPacket::Type> packetTypes = {
    ControlPacket::Type::PUBACK,
    ControlPacket::Type::PUBREC,
    ControlPacket::Type::PUBREL,
    ControlPacket::Type::PUBCOMP,
  };

  std::vector<mqtt::PublishResponse::ReasonCode> reasonCodes = {
    mqtt::PublishResponse::ReasonCode::NoMatchingSubscribers,
    mqtt::PublishResponse::ReasonCode::QuotaExceeded,
    mqtt::PublishResponse::ReasonCode::PacketIdentifierInUse,
    mqtt::PublishResponse::ReasonCode::PacketIdentifierInUse
  };

  std::vector<RespPacket> packets = {encodedPubAck, encodedPubRec,
                                     encodedPubRel, encodedPubComp};

  // clang-format on
  for (size_t i = 0; i < packets.size(); i++) {
    Decoder dec(packets[i]);

    FixedHeader fhdr = FixedHeaderReader::read(dec);
    REQUIRE(packetTypes[i] == ControlPacket::Type(fhdr.first >> 4));
    REQUIRE(uint32_t(0x03) == fhdr.second);

    const auto resp =
        PublishResponseDecoder::decode(dec, packetTypes[i], fhdr.second);
    REQUIRE(uint32_t(0x12) == resp.packetID);
    REQUIRE(reasonCodes[i] == resp.response.reasonCode);
    std::vector<uint8_t> buffer = PublishResponseEncoder(resp).encode();
    REQUIRE(buffer == packets[i]);
  }
}

TEST_CASE("testing PUBACK/PUBREC/PUBREL/PUBCOMP codec - with success code and "
          "props") {
  using RespPacket = std::vector<uint8_t>;
  // clang-format off
  std::vector<uint8_t> encodedPubAck = {
    0x40, // PUBACK,
    0x09,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::Success),
    0x05, 0x1F, 0x00, 0x02, 'a', 'b'
  };
  
  std::vector<uint8_t> encodedPubRec = {
    0x50, // PUBREC
    0x09,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::Success),
    0x05, 0x1F, 0x00, 0x02, 'a', 'b'
  };
  
  std::vector<uint8_t> encodedPubRel = {
    0x62, // PUBREL,
    0x09,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::Success),
    0x05, 0x1F, 0x00, 0x02, 'a', 'b'
  };
  
  std::vector<uint8_t> encodedPubComp = {
    0x70, // PUBCOMP,
    0x09,
    0x00, 0x12, // Packet identifier 18
    toUnderlyingType(mqtt::PublishResponse::ReasonCode::Success),
    0x05, 0x1F, 0x00, 0x02, 'a', 'b'    
  };  

  std::vector<ControlPacket::Type> packetTypes = {
    ControlPacket::Type::PUBACK,
    ControlPacket::Type::PUBREC,
    ControlPacket::Type::PUBREL,
    ControlPacket::Type::PUBCOMP,
  };

  std::vector<mqtt::PublishResponse::ReasonCode> reasonCodes = {
    mqtt::PublishResponse::ReasonCode::Success,
    mqtt::PublishResponse::ReasonCode::Success,
    mqtt::PublishResponse::ReasonCode::Success,
    mqtt::PublishResponse::ReasonCode::Success
  };

  std::vector<RespPacket> packets = {encodedPubAck, encodedPubRec,
                                     encodedPubRel, encodedPubComp};

  // clang-format on
  for (size_t i = 0; i < packets.size(); i++) {
    Decoder dec(packets[i]);

    FixedHeader fhdr = FixedHeaderReader::read(dec);
    REQUIRE(packetTypes[i] == ControlPacket::Type(fhdr.first >> 4));
    REQUIRE(uint32_t(0x09) == fhdr.second);

    const auto resp =
        PublishResponseDecoder::decode(dec, packetTypes[i], fhdr.second);
    REQUIRE(uint32_t(0x12) == resp.packetID);
    REQUIRE(reasonCodes[i] == resp.response.reasonCode);
    std::vector<uint8_t> buffer = PublishResponseEncoder(resp).encode();
    REQUIRE(buffer == packets[i]);
  }
}

TEST_CASE("testing PUBACK/PUBREC/PUBREL/PUBCOMP codec - with success code and "
          "no props ") {
  using RespPacket = std::vector<uint8_t>;
  // clang-format off
  std::vector<uint8_t> encodedPubAck = {
    0x40, // PUBACK,
    0x02,
    0x00, 0x12, // Packet identifier 18
  };
  
  std::vector<uint8_t> encodedPubRec = {
    0x50, // PUBREC
    0x02,
    0x00, 0x12, // Packet identifier 18
  };
  
  std::vector<uint8_t> encodedPubRel = {
    0x62, // PUBREL,
    0x02,
    0x00, 0x12, // Packet identifier 18
  };
  
  std::vector<uint8_t> encodedPubComp = {
    0x70, // PUBCOMP,
    0x02,
    0x00, 0x12, // Packet identifier 18
  };  

  std::vector<ControlPacket::Type> packetTypes = {
    ControlPacket::Type::PUBACK,
    ControlPacket::Type::PUBREC,
    ControlPacket::Type::PUBREL,
    ControlPacket::Type::PUBCOMP,
  };

  std::vector<mqtt::PublishResponse::ReasonCode> reasonCodes = {
    mqtt::PublishResponse::ReasonCode::Success,
    mqtt::PublishResponse::ReasonCode::Success,
    mqtt::PublishResponse::ReasonCode::Success,
    mqtt::PublishResponse::ReasonCode::Success
  };

  std::vector<RespPacket> packets = {encodedPubAck, encodedPubRec,
                                     encodedPubRel, encodedPubComp};

  // clang-format on
  for (size_t i = 0; i < packets.size(); i++) {
    Decoder dec(packets[i]);

    FixedHeader fhdr = FixedHeaderReader::read(dec);
    REQUIRE(packetTypes[i] == ControlPacket::Type(fhdr.first >> 4));
    REQUIRE(uint32_t(0x02) == fhdr.second);

    const auto resp =
        PublishResponseDecoder::decode(dec, packetTypes[i], fhdr.second);
    REQUIRE(uint32_t(0x12) == resp.packetID);
    REQUIRE(reasonCodes[i] == resp.response.reasonCode);
    std::vector<uint8_t> buffer = PublishResponseEncoder(resp).encode();
    REQUIRE(buffer == packets[i]);
  }
}
