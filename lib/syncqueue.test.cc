#include "doctest/doctest.h"

#include "syncqueue.h"

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
