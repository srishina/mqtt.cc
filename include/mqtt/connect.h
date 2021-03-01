#pragma once

#include <optional>
#include <string>
#include <vector>

namespace mqttutils {
  class Encoder;
  class Decoder;
} // namespace mqttutils

namespace mqtt {
  struct Connect {
    struct Properties;
    uint8_t protocolVersion{0x05};
    bool cleanStart{true};
    uint16_t keepAlive{0};
    std::shared_ptr<Connect::Properties> properties;
    std::string protocolName;
    std::string clientID;
    std::string userName;
    std::vector<uint8_t> password;
  };

  struct Connect::Properties {
    std::optional<uint32_t> sessionExpiryInterval;
    std::optional<uint16_t> receiveMaximum;
    std::optional<uint32_t> maximumPacketSize;
    std::optional<uint16_t> topicAliasMaximum;
    std::optional<bool> requestProblemInfo;
    std::optional<bool> requestResponseInfo;
    std::string authenticationMethod;
    std::vector<uint8_t> authenticationData;
  };

} // namespace mqtt
