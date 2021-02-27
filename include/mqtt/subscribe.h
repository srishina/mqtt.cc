#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mqtt {
  struct Subscription {
    std::string topicFilter;
    uint8_t qosLevel;
    bool noLocal;
    bool retainAsPublished;
    uint8_t retainHandling;
  };

  struct Subscribe {
    struct Properties;

    std::vector<Subscription> subscriptions;
    std::shared_ptr<Subscribe::Properties> properties;
  };

  struct Subscribe::Properties {
    std::optional<uint32_t> subscriptionIdentifier;
  };
} // namespace mqtt
