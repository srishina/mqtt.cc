#pragma once

#include <optional>
#include <string>
#include <vector>

namespace mqtt {
  class Subscriber {
  public:
    virtual void onData() = 0;
    virtual ~Subscriber();
  };

  using Subscribers = std::vector<std::shared_ptr<Subscriber>>;

  namespace ControlPacket {
    enum ReasonCode {
      // CONNACK, PUBACK, PUBREC, PUBREL, PUBCOMP, UNSUBACK, AUTH
      Success = 0x00,
      NoMatchingSubscribers = 0x10, // PUBACK, PUBREC
      // CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
      UnspecifiedError = 0x80,
      MalformedPacket = 0x81, // CONNACK, DISCONNECT
      ProtocolError = 0x82,   // CONNACK, DISCONNECT
      // CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
      ImplSpecificError = 0x83,
      // CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
      NotAuthorized = 0x87,
      ServerBusy = 0x89,               // CONNACK, DISCONNECT
      BadAuthMethod = 0x8C,            // CONNACK, DISCONNECT
      TopicFilterInvalid = 0x8F,       // SUBACK, UNSUBACK, DISCONNECT
      TopicNameInvalid = 0x90,         // CONNACK, PUBACK, PUBREC, DISCONNECT
      PacketIdentifierInUse = 0x91,    // PUBACK, SUBACK, UNSUBACK
      PacketIdentifierNotFound = 0x92, // PUBREL, PUBCOMP
      PacketTooLarge = 0x95,           // CONNACK, PUBACK, PUBREC, DISCONNECT
      QuotaExceeded = 0x97,            // PUBACK, PUBREC, SUBACK, DISCONNECT
      PayloadFormatInvalid = 0x99,     // CONNACK, DISCONNECT
      RetainNotSupported = 0x9A,       // CONNACK, DISCONNECT
      QoSNotSupported = 0x9B,          // CONNACK, DISCONNECT
      UseAnotherServer = 0x9C,         // CONNACK, DISCONNECT
      ServerMoved = 0x9D,              // CONNACK, DISCONNECT
      SharedSubscriptionsNotSupported = 0x9E,  // SUBACK, DISCONNECT
      ConnectionRateExceeded = 0x9F,           // CONNACK, DISCONNECT
      SubscriptionIdsNotSupported = 0xA1,      // SUBACK, DISCONNECT
      WildcardSubscriptionsNotSupported = 0xA2 // SUBACK, DISCONNECT
    };
  }

} // namespace mqtt
