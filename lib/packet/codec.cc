#include "codec.h"
#include <iostream>
#include <sstream>
#include <string>

namespace packet {
  Encoder::Encoder(size_t capacity) {
    buffer.reserve(capacity);
  }

  void Encoder::write(bool value) {
    buffer.push_back(value ? 1 : 0);
  }

  void Encoder::write(uint8_t value) {
    buffer.push_back(value);
  }

  // Write Big Endian 16-bit
  void Encoder::write(uint16_t value) {
    buffer.push_back(uint8_t(value >> 8));
    buffer.push_back(uint8_t(value));
  }

  // Write Big Endian 32-bit
  void Encoder::write(uint32_t value) {
    buffer.push_back(uint8_t(value >> 24));
    buffer.push_back(uint8_t(value >> 16));
    buffer.push_back(uint8_t(value >> 8));
    buffer.push_back(uint8_t(value));
  }

  void Encoder::write(const std::vector<uint8_t>& value) {
    this->writeBinaryData(value);
  }

  void Encoder::write(const std::string& value) {
    this->writeUTF8String(value);
  }

  void Encoder::writeVarUint32(const std::vector<uint32_t>& value) {
    for (uint32_t v : value) {
      this->writeVarUint32(v);
    }
  }

  void Encoder::writeVarUint32(uint32_t value) {
    if (value > maxVarUint32) {
      throw std::overflow_error(
          __PRETTY_FUNCTION__ +
          std::string(": Variable integer contains value of %d which is more "
                      "than the permissible"));
    }
    do {
      uint8_t encodedByte = value % 0x80;
      value = value / 0x80;

      if (value > 0) {
        encodedByte |= 0x80;
      }

      this->write(encodedByte);
    } while (value != 0);
  }

  void Encoder::writeBinaryData(const std::vector<uint8_t>& value) {
    size_t size = value.size();
    if (size > std::numeric_limits<uint16_t>::max()) {
      throw std::overflow_error(__PRETTY_FUNCTION__ +
                                std::string(": positive overflow"));
    }

    this->write(static_cast<uint16_t>(value.size()));
    this->writeBinaryDataNoLen(value);
  }

  void Encoder::writeBinaryDataNoLen(const std::vector<uint8_t>& value) {
    this->buffer.insert(this->buffer.end(), value.begin(), value.end());
  }

  void Encoder::writeUTF8String(const std::string& value) {
    size_t size = value.size();
    if (size > std::numeric_limits<uint16_t>::max()) {
      throw std::overflow_error(__PRETTY_FUNCTION__ +
                                std::string(": positive overflow"));
    }
    this->write(static_cast<uint16_t>(value.size()));
    this->writeBinaryDataNoLen(
        std::vector<uint8_t>(value.begin(), value.end()));
  }

  const std::vector<uint8_t>& Encoder::getBuffer() const {
    return buffer;
  }

  // --------------------------------------------------------------------------------------

  Decoder::Decoder(std::vector<uint8_t> value) : buffer(value), index(0) {}

  uint16_t Decoder::readBigEndianUint16() {
    uint16_t result =
        static_cast<uint16_t>((static_cast<uint16_t>(buffer[index++]) << 8));
    result |= uint16_t(buffer[index++]);
    return result;
  }

  uint32_t Decoder::readBigEndianUint32() {
    uint16_t result =
        static_cast<uint16_t>((static_cast<uint16_t>(buffer[index++]) << 24));
    result |= uint16_t(buffer[index++]) << 16;
    result |= uint16_t(buffer[index++]) << 8;
    result |= uint16_t(buffer[index++]);

    return result;
  }

  uint32_t Decoder::readVarUint32() {
    uint32_t value = 0;
    uint32_t multiplier = 1;
    uint8_t consumed = 0;

    while (true) {
      uint8_t encodedByte = this->read<uint8_t>();
      consumed++;
      if (consumed > 4) {
        std::ostringstream stream;
        stream << __PRETTY_FUNCTION__
               << "  :variable integer contained more than maximum bytes  "
               << consumed;
        throw std::overflow_error(stream.str());
      }
      value += static_cast<uint32_t>(encodedByte & 0x7f) * multiplier;
      if ((encodedByte & 0x80) == 0) {
        break;
      }

      multiplier *= 128;
      if (multiplier > 128 * 128 * 128) {
        std::ostringstream stream;
        stream << __PRETTY_FUNCTION__
               << "  :variable integer contains value of " << multiplier
               << " which is more than the permissible";
        throw std::overflow_error(stream.str());
      }
    }

    return value;
  }

  std::vector<uint8_t> Decoder::readBinaryData() {
    size_t size = this->readBigEndianUint16();
    return this->readBinaryDataNoLen(size);
  }

  std::vector<uint8_t> Decoder::readBinaryDataNoLen(size_t size) {
    std::vector<uint8_t> result;
    result.assign(this->buffer.data() + this->index,
                  this->buffer.data() + (this->index + size));
    this->index += size;
    return result;
  }

  std::string Decoder::readUTF8String() {
    size_t size = this->readBigEndianUint16();
    std::string result(this->buffer.data() + this->index,
                       this->buffer.data() + (this->index + size));
    this->index += size;
    return result;
  }

  uint32_t EncodedVarUint32::size(uint32_t value) {
    uint32_t varSize = 0;
    do {
      // uint8_t encodedByte = value % 0x80;
      value /= 0x80;
      varSize++;
    } while (value != 0);

    return varSize;
  }
} // namespace packet
