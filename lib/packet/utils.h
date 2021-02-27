#pragma once

namespace packet {
  template <typename Enum> constexpr auto toUnderlyingType(Enum e) {
    return static_cast<typename std::underlying_type<Enum>::type>(e);
  }

} // namespace packet
