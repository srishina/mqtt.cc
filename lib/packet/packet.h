#pragma once

#include <cstdint>
#include <iostream>

namespace packet {
  class Encoder;
  class Decoder;
} // namespace packet

namespace packet {
  using FixedHeader = std::pair<uint8_t, uint32_t>;
  struct FixedHeaderReader {
    static FixedHeader read(Decoder& dec);
  };
  namespace ControlPacket {
    enum class Type {
      RESERVED = 0,
      CONNECT = 1,
      CONNACK = 2,
      PUBLISH = 3,
      PUBACK = 4,
      PUBREC = 5,
      PUBREL = 6,
      PUBCOMP = 7,
      SUBSCRIBE = 8,
      SUBACK = 9,
      UNSUBSCRIBE = 10,
      UNSUBACK = 11,
      PINGREQ = 12,
      PINGRESP = 13,
      DISCONNECT = 14,
      AUTH = 15
    };
  } // namespace ControlPacket

} // namespace packet
