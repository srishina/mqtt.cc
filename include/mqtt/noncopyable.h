#pragma once

namespace mqtt {
  class noncopyable {
  protected:
    noncopyable()  = default;
    ~noncopyable() = default;

    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
  };
} // namespace mqtt
