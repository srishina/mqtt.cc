#pragma once

#include <iostream>
#include <system_error>

namespace mqtt {
  enum class Error {
    Success                = 0,
    TopicLenTooLong        = 1,
    InvalidTopic           = 2,
    EmptySubscriptionTopic = 3,
    InvalidProtocolName = 4,
  };

  class ErrorCategory : public std::error_category {
  public:
    virtual const char* name() const noexcept override final;
    virtual std::string message(int ev) const override final;
  };

  inline const mqtt::ErrorCategory& ErrorCategoryCreate() noexcept {
    static mqtt::ErrorCategory& c = *new mqtt::ErrorCategory;
    return c;
  }

  inline std::error_code make_error_code(Error err) {
    return std::error_code(static_cast<int>(err), ErrorCategoryCreate());
  }
} // namespace mqtt

namespace std {
  template <> struct is_error_code_enum<mqtt::Error> : true_type {};
} // namespace std
