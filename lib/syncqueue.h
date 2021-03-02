#pragma once

#include <condition_variable>
#include <mqtt/noncopyable.h>
#include <mutex>
#include <queue>

namespace mqttutils {
  template <typename T> class SyncQueue : public mqtt::noncopyable {
  public:
    SyncQueue();

    void close();

    std::pair<T, bool> pop();
    bool pop(T& item);

    void push(const T& item);
    void push(T&& item);

    size_t size() const;

  private:
    std::queue<T> queue;
    mutable std::mutex mux;
    std::condition_variable cond;
    bool closed;
  };

  template <typename T> SyncQueue<T>::SyncQueue() : closed(false) {}

  template <typename T> void SyncQueue<T>::close() {
    {
      std::unique_lock<std::mutex> lock(this->mux);
      closed = true;
    }

    this->cond.notify_all();
  }

  template <typename T> std::pair<T, bool> SyncQueue<T>::pop() {
    T item;
    bool qclosed = this->pop(item);
    return {item, qclosed};
  }

  template <typename T> bool SyncQueue<T>::pop(T& item) {
    std::unique_lock<std::mutex> lock(this->mux);
    if (!this->closed && this->queue.empty()) {
      this->cond.wait(lock);
    }

    if (this->closed) {
      return true;
    }

    // copy the item first
    item = std::move(this->queue.front());
    // now pop
    this->queue.pop();
    return false;
  }

  template <typename T> void SyncQueue<T>::push(const T& item) {
    bool wasEmpty = false;
    {
      std::unique_lock<std::mutex> lock(this->mux);
      if (closed) {
        return;
      }
      wasEmpty = this->queue.empty();
      this->queue.push(item);
    }

    // notify that we have an item available when the queue was empty
    if (wasEmpty) {
      this->cond.notify_one();
    }
  }

  template <typename T> void SyncQueue<T>::push(T&& item) {
    {
      std::unique_lock<std::mutex> lock(this->mux);
      if (closed) {
        return;
      }
      this->queue.push(item);
    }

    // notify that we have an item available
    this->cond.notify_one();
  }

  template <typename T> size_t SyncQueue<T>::size() const {
    std::unique_lock<std::mutex> lock(this->mux);
    return this->queue.size();
  }

} // namespace mqttutils
