#pragma once

#include <mqtt/mqtt.h>
#include <optional>
#include <string>
#include <vector>

namespace mqtt {
  struct Publish {
    struct Properties;

    uint8_t qosLevel{0};
    bool isDup{false};
    bool hasRetain{false};
    std::string topicName;
    std::shared_ptr<Publish::Properties> properties;
    std::vector<uint8_t> payload;
  };

  struct Publish::Properties {
    std::optional<bool> payloadFormatIndicator;
    std::optional<uint32_t> messageExpiryInterval;
    std::optional<uint16_t> topicAlias;
    std::string responseTopic;
    std::vector<uint8_t> correlationData;
    std::vector<uint32_t> subscriptionIdentifiers;
    std::string contentType;
  };
} // namespace mqtt
