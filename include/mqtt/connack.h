#pragma once
#include <cstdint>
#include <memory>
#include <mqtt/mqtt.h>
#include <optional>
#include <string>
#include <vector>

namespace mqtt {
  struct ConnAck {
    struct Properties;
    enum class ReasonCode : uint8_t {
      Success = ControlPacket::ReasonCode::Success,
      UnspecifiedError = ControlPacket::ReasonCode::UnspecifiedError,
      MalformedPacket = ControlPacket::ReasonCode::MalformedPacket,
      ProtocolError = ControlPacket::ReasonCode::ProtocolError,
      ImplSpecificError = ControlPacket::ReasonCode::ImplSpecificError,
      UnsupportedProtocolVer = 0x84,
      ClientIDNotValud = 0x85,
      BadUsernameOrPWD = 0x86,
      NotAuthorized = ControlPacket::ReasonCode::NotAuthorized,
      ServerUnavailable = 0x88,
      ServerBusy = ControlPacket::ReasonCode::ServerBusy,
      Banned = 0x8A,
      BadAuthMethod = 0x8C,
      TopicNameInvalid = ControlPacket::ReasonCode::TopicNameInvalid,
      PacketTooLarge = ControlPacket::ReasonCode::PacketTooLarge,
      QuotaExceeded = ControlPacket::ReasonCode::QuotaExceeded,
      PayloadFormatInvalid = ControlPacket::ReasonCode::PayloadFormatInvalid,
      RetainNotSupported = ControlPacket::ReasonCode::RetainNotSupported,
      QoSNotSupported = ControlPacket::ReasonCode::QoSNotSupported,
      UseAnotherServer = ControlPacket::ReasonCode::UseAnotherServer,
      ServerMoved = ControlPacket::ReasonCode::ServerMoved,
      ConnectionRateExceeded = ControlPacket::ReasonCode::ConnectionRateExceeded
    };

    bool sessionPresent{false};
    ReasonCode reasonCode{ReasonCode::Success};
    std::shared_ptr<ConnAck::Properties> properties;

    std::string getReasonText() const;
    std::string getReasonDescription() const;
  };

  struct ConnAck::Properties {
    std::optional<uint32_t> sessionExpiryInterval;
    std::optional<uint16_t> receiveMaximum;
    std::optional<uint8_t> maximumQoS;
    std::optional<bool> retainAvailable;
    std::optional<uint32_t> maximumPacketSize;
    std::string assignedClientIdentifier;
    std::optional<uint16_t> topicAliasMaximum;
    std::string reasonString;
    std::optional<bool> wildcardSubscriptionAvailable;
    std::optional<bool> subscriptionIdentifierAvailable;
    std::optional<bool> sharedSubscriptionAvailable;
    std::optional<uint16_t> serverKeepAlive;
    std::string responseInformation;
    std::string serverReference;
    std::string authenticationMethod;
    std::vector<uint8_t> authenticationData;
  };

} // namespace mqtt
