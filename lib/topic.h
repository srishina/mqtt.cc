#pragma once

#include "mqtt/mqtt.h"
#include "mqtt/noncopyable.h"
#include <mutex>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace mqttutils {
  class TopicUtils {
  public:
    static std::error_code validatePublishTopic(const std::string& topic);
    static std::error_code validateSubscribeTopic(const std::string& topic);
    static std::vector<std::string> split(const std::string& topic);
  };

  class Trie : private mqtt::noncopyable {
    struct Node;

  public:
    void              insert(const std::string&                topic,
                             std::shared_ptr<mqtt::Subscriber> subscriber);
    void              remove(const std::string&                topic,
                             std::shared_ptr<mqtt::Subscriber> subscriber);
    mqtt::Subscribers match(const std::string& topic) const;

    void print();

  private:
    void detachChild(Trie::Node& child);
    void match(const std::vector<std::string>& parts,
               const Node&                     node,
               mqtt::Subscribers&              subscribers) const;

    void printNode(const Node& node);

  private:
    struct Node {
      Node();
      Node(Node& p, const std::string& part);

      Node*                                                  parent;
      std::string                                            topicPart;
      std::vector<std::shared_ptr<mqtt::Subscriber>>         subscribers;
      std::unordered_map<std::string, std::unique_ptr<Node>> children;
    };

    mutable std::mutex mux;
    Node               root;
  };

  class TopicMatcher : private mqtt::noncopyable {
  public:
    TopicMatcher();

    std::error_code   subscribe(const std::string&                topic,
                                std::shared_ptr<mqtt::Subscriber> subscriber);
    std::error_code   unsubscribe(const std::string&                topic,
                                  std::shared_ptr<mqtt::Subscriber> subscriber);
    mqtt::Subscribers match(const std::string& topic) const;

    void print();

  private:
    std::unique_ptr<Trie> trie;
  };
} // namespace mqttutils
