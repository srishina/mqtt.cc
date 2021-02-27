#include "doctest/doctest.h"
#include "properties.h"
#include <array>
#include <exception>
#include <optional>

using namespace packet;

TEST_CASE("testing property size") {
  // The property size is calculated as follows:
  // the size of the data + the property ID size
  // Incase of binary data or string, the size of the data is being added
  // the size is uint16_t (2 bytes)
  CHECK(Property::size(std::optional<uint8_t>(16)) == 2);
  CHECK(Property::size(std::optional<uint8_t>()) == 0);
  CHECK(Property::size(std::optional<uint16_t>(1094)) == 3);
  CHECK(Property::size(std::optional<uint32_t>(8192)) == 5);
  CHECK(Property::size(std::optional<bool>(true)) == 2);
  CHECK(Property::size<std::optional<uint32_t>, true>(
            std::optional<uint32_t>(1)) == 2);
  CHECK(Property::size<std::optional<uint32_t>, true>(
            std::optional<uint32_t>(2097151)) == 4);
  CHECK(Property::sizeMoreThanOnce<std::vector<uint32_t>, true>(
            {1, 1, 2097151}) == 8);
  CHECK(Property::size(std::string("hello")) == 8);
  CHECK(Property::size(std::vector<uint8_t>{'h', 'e', 'l', 'l', 'o'}) == 8);
}

template <typename T, bool varuint32 = false>
void propertyTypeCodecTest(Property::ID id, const T& testValue) {
  size_t size = Property::size<T, varuint32>(testValue);
  Encoder enc(size);
  Property::encode<T, varuint32>(enc, id, testValue);
  Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
  T varDecoded;
  CHECK(id == static_cast<Property::ID>(dec.read<uint32_t, true>()));
  Property::decode<T, varuint32>(dec, id, varDecoded);
  CHECK(varDecoded == testValue);
  Decoder dec2(std::vector<uint8_t>(enc.getBuffer()));
  CHECK_THROWS_AS(Property::decode(dec2, id, varDecoded);, std::runtime_error);
}

TEST_CASE("testing property encode/decode") {
  propertyTypeCodecTest(Property::ID::RequestProblemInfoID,
                        std::optional<bool>(true));
  propertyTypeCodecTest(Property::ID::PayloadFormatIndicatorID,
                        std::optional<uint8_t>(0x64));
  propertyTypeCodecTest(Property::ID::ReceiveMaximumID,
                        std::optional<uint16_t>(0x446));
  propertyTypeCodecTest(Property::ID::MessageExpiryIntervalID,
                        std::optional<uint32_t>(0x1000));
  propertyTypeCodecTest<std::optional<uint32_t>, true>(
      Property::ID::SubscriptionIdentifierID, std::optional<uint32_t>(0x2000));
  propertyTypeCodecTest(Property::ID::AuthenticationDataID,
                        std::vector<uint8_t>{'h', 'e', 'l', 'l', 'o'});
  propertyTypeCodecTest(Property::ID::AuthenticationMethodID,
                        std::string("base64"));
}
