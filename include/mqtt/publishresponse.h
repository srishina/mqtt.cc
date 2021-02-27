#pragma once

#include <mqtt/mqtt.h>
#include <string>

namespace mqtt {
  // For QoS 1 and 2 the responses can be generalized. As such
  // the response code/properties for PubAck and PubRec are
  // returned as part of the response
  struct PublishResponse {
    struct Properties;
    enum class ReasonCode {
      Success = ControlPacket::ReasonCode::Success,
      NoMatchingSubscribers = ControlPacket::ReasonCode::NoMatchingSubscribers,
      UnspecifiedError = ControlPacket::ReasonCode::UnspecifiedError,
      ImplSpecificError = ControlPacket::ReasonCode::ImplSpecificError,
      ImplNotAuthorized = ControlPacket::ReasonCode::NotAuthorized,
      TopicNameInvalid = ControlPacket::ReasonCode::TopicNameInvalid,
      PacketIdentifierInUse = ControlPacket::ReasonCode::PacketIdentifierInUse,
      QuotaExceeded = ControlPacket::ReasonCode::QuotaExceeded,
      PayloadFormatInvalid = ControlPacket::ReasonCode::PayloadFormatInvalid,
    };
    PublishResponse::ReasonCode reasonCode{ReasonCode::Success};
    std::shared_ptr<PublishResponse::Properties> properties;
  };

  struct PublishResponse::Properties {
    std::string reasonString;
  };
} // namespace mqtt
