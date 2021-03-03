#include "doctest/doctest.h"

#include "syncqueue.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>

TEST_CASE("testing syncqueue push/pop") {
  auto q = mqttutils::SyncQueue<uint32_t>();
  const std::vector<uint32_t> items = {1024, 2048, 4096, 8192};
  for (auto item : items) {
    q.push(item);
  }

  CHECK(q.size() == items.size());
  for (auto item : items) {
    uint32_t result = 0;
    CHECK(!q.pop(result));
    CHECK(result == item);
  }
  q.close();
  uint32_t result = 0;
  CHECK(q.pop(result));
}

TEST_CASE("testing syncqueue using multiple threads") {
  const std::vector<std::string> values = {"Hello world!", "Welcome!",
                                           "Willkommen!"};
  auto q = mqttutils::SyncQueue<std::string>();

  const size_t maxConsumedVal = 15;
  size_t consumedVal = 0;
  std::mutex mux;
  std::condition_variable condVar;

  auto producer = [&q, &values, &mux, &condVar, &consumedVal]() {
    for (size_t i = 0; i < 5; ++i) {
      for (const auto& v : values) {
        q.push(v);
      }
    }
    {
      std::unique_lock<std::mutex> lock(mux);
      condVar.wait(lock,
                   [&consumedVal] { return (consumedVal == maxConsumedVal); });
    }

    q.close();
  };

  auto consumer = [&q, &values, &mux, &condVar, &consumedVal]() {
    std::string val;
    while (!q.pop(val)) {
      bool matches = false;
      for (const auto& v : values) {
        if (v == val) {
          matches = true;
          break;
        }
      }
      CHECK(matches);
      size_t consumed = 0;
      {
        std::unique_lock<std::mutex> lock(mux);
        ++consumedVal;
        consumed = consumedVal;
      }
      if (consumed == maxConsumedVal) {
        condVar.notify_one();
      }
    }
  };

  std::thread thread1(consumer);
  std::thread thread2(consumer);
  std::thread thread3(producer);

  thread1.join();
  thread2.join();
  thread3.join();
}
