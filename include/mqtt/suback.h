#pragma once

#include <cstdint>
#include <mqtt/mqtt.h>
#include <string>

namespace mqtt {
  struct SubAck {
    struct Properties;
    enum class ReasonCode : uint8_t {
      GrantedQoS0 = ControlPacket::ReasonCode::Success,
      GrantedQoS1 = 0x01,
      GrantedQoS2 = 0x02,
      UnspecifiedError = ControlPacket::ReasonCode::UnspecifiedError,
      ImplSpecificError = ControlPacket::ReasonCode::ImplSpecificError,
      NotAuthorized = ControlPacket::ReasonCode::NotAuthorized,
      TopicFilterInvalid = ControlPacket::ReasonCode::TopicFilterInvalid,
      PacketIdentifierInUse = ControlPacket::ReasonCode::PacketIdentifierInUse,
      QuotaExceeded = ControlPacket::ReasonCode::QuotaExceeded,
      SharedSubscriptionsNotSupported =
          ControlPacket::ReasonCode::SharedSubscriptionsNotSupported,
      SubscriptionIdsNotSupported =
          ControlPacket::ReasonCode::SubscriptionIdsNotSupported,
      WildcardSubscriptionsNotSupported =
          ControlPacket::ReasonCode::WildcardSubscriptionsNotSupported,
    };

    std::shared_ptr<SubAck::Properties> properties;
    std::vector<SubAck::ReasonCode> reasonCodes;
  };

  struct SubAck::Properties {
    std::string reasonString;
  };
} // namespace mqtt
