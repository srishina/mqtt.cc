#pragma once

#include <mqtt/mqtt.h>
#include <string>
#include <vector>

namespace mqtt {
  struct UnsubAck {
    struct Properties;
    enum class ReasonCode : uint8_t {
      Success = ControlPacket::ReasonCode::Success,
      NoSubscriptionExisted = 0x11,
      UnspecifiedError = ControlPacket::ReasonCode::UnspecifiedError,
      ImplSpecificError = ControlPacket::ReasonCode::ImplSpecificError,
      NotAuthorized = ControlPacket::ReasonCode::NotAuthorized,
      TopicFilterInvalid = ControlPacket::ReasonCode::TopicFilterInvalid,
      PacketIdentifierInUse = ControlPacket::ReasonCode::PacketIdentifierInUse
    };

    std::shared_ptr<UnsubAck::Properties> properties;
    std::vector<UnsubAck::ReasonCode> reasonCodes;
  };

  struct UnsubAck::Properties {
    std::string reasonString;
  };

} // namespace mqtt
