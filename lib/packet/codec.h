#pragma once

#include "mqtt/noncopyable.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace packet {
  const uint32_t maxVarUint32 = 268435455;
  class Encoder : private mqtt::noncopyable {
  public:
    explicit Encoder(size_t capacity);
    // todo: check later, change to operator <<
    void write(bool value);
    void write(uint8_t value);
    void write(uint16_t value);
    void write(uint32_t value);
    void write(const std::vector<uint8_t>& value);
    void write(const std::string& value);

    void writeVarUint32(const std::vector<uint32_t>& value);
    void writeVarUint32(uint32_t value);
    void writeBinaryDataNoLen(const std::vector<uint8_t>& value);

    const std::vector<uint8_t>& getBuffer() const;

  private:
    void writeBinaryData(const std::vector<uint8_t>& value);
    void writeUTF8String(const std::string& value);

  private:
    std::vector<uint8_t> buffer;
  };

  template <typename T> struct return_item { typedef T type; };

  class Decoder : private mqtt::noncopyable {
  public:
    Decoder(std::vector<uint8_t> value);

    template <typename T, bool varuint32 = false>
    inline typename return_item<T>::type read() {
      static_assert(!varuint32, "varuint32 cannot be read by this");
      return T(buffer[index++]);
    }

    // todo: check later, change to operator >>
    std::vector<uint8_t> readBinaryDataNoLen(size_t size);

  private:
    uint16_t readBigEndianUint16();
    uint32_t readBigEndianUint32();
    uint32_t readVarUint32();
    std::vector<uint8_t> readBinaryData();
    std::string readUTF8String();

  private:
    std::vector<uint8_t> buffer;
    size_t index;
  };

  template <> inline return_item<uint16_t>::type Decoder::read<uint16_t>() {
    return this->readBigEndianUint16();
  }

  template <> inline return_item<uint32_t>::type Decoder::read<uint32_t>() {
    return this->readBigEndianUint32();
  }

  template <>
  inline return_item<uint32_t>::type Decoder::read<uint32_t, true>() {
    return this->readVarUint32();
  }

  template <>
  inline return_item<std::vector<uint8_t>>::type
  Decoder::read<std::vector<uint8_t>>() {
    return this->readBinaryData();
  }

  template <>
  inline return_item<std::string>::type Decoder::read<std::string>() {
    return this->readUTF8String();
  }

  class EncodedVarUint32 {
  public:
    static uint32_t size(uint32_t value);

  private:
    EncodedVarUint32() {}
  };

} // namespace packet

namespace std {
  template <typename T>
  std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    copy(v.begin(), v.end(), std::ostream_iterator<T>(os));
    return os;
  }
} // namespace std
