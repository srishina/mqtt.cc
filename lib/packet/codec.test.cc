#include "codec.h"
#include "doctest/doctest.h"

#include <iterator>
#include <map>
#include <string>

using namespace packet;

TEST_CASE("testing codec byte") {
  uint8_t testValue = uint8_t(0x64);
  Encoder enc(1);
  enc.write(testValue);
  Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
  CHECK(dec.read<uint8_t>() == testValue);
}

TEST_CASE("testing codec bool") {
  bool testValue = true;
  Encoder enc(1);
  enc.write(testValue);
  Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
  CHECK(dec.read<bool>() == testValue);
}

TEST_CASE("testing codec Big Endian uint16") {
  uint16_t testValue = uint16_t(128);
  Encoder enc(2);
  enc.write(testValue);
  Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
  CHECK(dec.read<uint16_t>() == testValue);
}

TEST_CASE("testing codec Big Endian uint32") {
  uint32_t testValue = uint32_t(4096);
  Encoder enc(4);
  enc.write(testValue);
  Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
  CHECK(dec.read<uint32_t>() == testValue);
}

TEST_CASE("testing codec var uint32") {
  std::map<uint32_t, std::vector<uint8_t>> elements = {
      {0, std::vector<uint8_t>{0x00}},
      {127, std::vector<uint8_t>{0x7f}},
      {128, std::vector<uint8_t>{0x80, 0x01}},
      {16383, std::vector<uint8_t>{0xFF, 0x7F}},
      {16384, std::vector<uint8_t>{0x80, 0x80, 0x01}},
      {2097151, std::vector<uint8_t>{0xFF, 0xFF, 0x7F}},
      {2097152, std::vector<uint8_t>{0x80, 0x80, 0x80, 0x01}},
      {268435455, std::vector<uint8_t>{0xFF, 0xFF, 0xFF, 0x7F}}};

  for (std::pair<uint32_t, std::vector<uint8_t>> element : elements) {
    Encoder enc(4);
    enc.writeVarUint32(element.first);
    CHECK(enc.getBuffer() == element.second);
    Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
    CHECK(dec.read<uint32_t, true>() == element.first);
  }
}

TEST_CASE("testing codec UTF8 string") {
  std::map<std::string, std::vector<uint8_t>> elements = {
      {std::string("hello"),
       std::vector<uint8_t>{0x00, 0x05, 'h', 'e', 'l', 'l', 'o'}},
      {std::string("\uFEFF"),
       std::vector<uint8_t>{0x00, 0x03, 0xEF, 0xBB, 0xBF}}};

  for (std::pair<std::string, std::vector<uint8_t>> element : elements) {
    Encoder enc(element.first.size() + 2);
    enc.write(element.first);
    CHECK(enc.getBuffer() == element.second);
    Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
    CHECK(dec.read<std::string>() == element.first);
  }
}

TEST_CASE("testing codec binary data") {
  std::map<std::vector<uint8_t>, std::vector<uint8_t>> elements = {
      {std::vector<uint8_t>{'h', 'e', 'l', 'l', 'o'},
       std::vector<uint8_t>{0x00, 0x05, 'h', 'e', 'l', 'l', 'o'}},
      {std::vector<uint8_t>{0xEF, 0xBB, 0xBF},
       std::vector<uint8_t>{0x00, 0x03, 0xEF, 0xBB, 0xBF}}};

  for (std::pair<std::vector<uint8_t>, std::vector<uint8_t>> element :
       elements) {
    Encoder enc(element.first.size() + 2);
    enc.write(element.first);
    CHECK(enc.getBuffer() == element.second);
    Decoder dec(std::vector<uint8_t>(enc.getBuffer()));
    CHECK(dec.read<std::vector<uint8_t>>() == element.first);
  }
}

TEST_CASE("testing codec var uint32 size") {
  std::map<uint32_t, size_t> elements = {
      {0, 1},     {127, 1},     {128, 2},     {16383, 2},
      {16384, 3}, {2097151, 3}, {2097152, 4}, {268435455, 4}};

  for (std::pair<uint32_t, size_t> element : elements) {
    CHECK(EncodedVarUint32::size(element.first) == element.second);
  }
}
