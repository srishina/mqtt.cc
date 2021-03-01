#include "packet.h"
#include "codec.h"

namespace packet {
  FixedHeader FixedHeaderReader::read(Decoder& dec) {
    uint8_t byte0 = dec.read<uint8_t>();
    uint32_t remainingLen = dec.read<uint32_t, true>();
    return {byte0, remainingLen};
  }
} // namespace packet
