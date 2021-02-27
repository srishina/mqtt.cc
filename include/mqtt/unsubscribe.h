#pragma once

#include <string>
#include <vector>

namespace mqtt {
  struct Unsubscribe {
    struct Properties;

    std::shared_ptr<Unsubscribe::Properties> properties;
    std::vector<std::string> topicFilters;
  };

  struct Unsubscribe::Properties {};

} // namespace mqtt
