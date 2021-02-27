#include "mqtt/error.h"

namespace mqtt {
  const char* ErrorCategory::name() const noexcept {
    return "MQTTError";
  }

  std::string ErrorCategory::message(int ev) const {
    switch (static_cast<Error>(ev)) {
    case Error::Success:
      return "Success";
    case Error::TopicLenTooLong:
      return "Topic length is too long, max = 65335";
    case Error::InvalidTopic:
      return "Invalid topic";
    case Error::EmptySubscriptionTopic:
      return "Empty subscription topics are not allowed";
    case Error::InvalidProtocolName:
      return "Protocol name is invalid";
    }
  }

} // namespace mqtt
