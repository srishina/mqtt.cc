#include "topic.h"
#include "mqtt/error.h"
#include <regex>

namespace mqttutils {
  std::error_code TopicUtils::validatePublishTopic(const std::string& topic) {
    if (topic.size() > 65535) {
      return mqtt::Error::TopicLenTooLong;
    }

    return (std::string::npos == topic.find_first_of("+#"))
               ? mqtt::Error::Success
               : mqtt::Error::InvalidTopic;
  }

  std::error_code TopicUtils::validateSubscribeTopic(const std::string& topic) {
    // empty subscribe topics are not allowed
    if (topic.size() == 0) {
      return mqtt::Error::EmptySubscriptionTopic;
    }

    if (topic.size() > 65535) {
      return mqtt::Error::TopicLenTooLong;
    }

    char   previousChar = 0;
    size_t topicLen     = topic.size();

    for (size_t i = 0; i < topic.size(); ++i) {
      const char ch = topic[i];
      if (ch == '+') {
        if ((i != 0 && previousChar != '/') ||
            (i < (topicLen - 1) && topic[i + 1] != '/')) {
          return mqtt::Error::InvalidTopic;
        }
      } else if (ch == '#') {
        if ((i != 0 && previousChar != '/') || (i < (topicLen - 1))) {
          return mqtt::Error::InvalidTopic;
        }
      }
      previousChar = ch;
    }
    return mqtt::Error::Success;
  }

  std::vector<std::string> TopicUtils::split(const std::string& topic) {
    std::regex               regex("/");
    std::vector<std::string> out(
        std::sregex_token_iterator(topic.begin(), topic.end(), regex, -1),
        std::sregex_token_iterator());
    if (topic.back() == '/') {
      // add an empty string, due to the behaviour,
      //   /a/b//c/  to {}{a}{b}{}{c}  and ignores the empty string after {c}
      // as the string ended in forward slash. In order for our matching algo
      // to work we need the empty string
      out.emplace_back("");
    }
    return out;
  }
  // ------------------------------------------------------------------
  Trie::Node::Node() : parent(nullptr) {}

  Trie::Node::Node(Node& p, const std::string& part)
      : parent(&p), topicPart(part) {}

  void Trie::insert(const std::string&                topic,
                    std::shared_ptr<mqtt::Subscriber> subscriber) {
    std::vector<std::string>    parts = mqttutils::TopicUtils::split(topic);
    Node*                       cur   = &root;
    std::unique_ptr<Node>       newTree;
    std::lock_guard<std::mutex> guard(this->mux);
    for (size_t i = 0; i < parts.size(); i++) {
      std::unordered_map<std::string, std::unique_ptr<Node>>::iterator it =
          cur->children.find(parts[i]);
      if (it == cur->children.end()) {
        // Not found, create a new tree and merge
        Node* leaf = cur;
        // the loop is a continuation
        for (; i < parts.size(); i++) {
          std::unique_ptr<Node> node = std::make_unique<Node>(*leaf, parts[i]);
          leaf                       = node.get();
          if (!newTree) {
            newTree = std::move(node);
          } else {
            node->parent->children[parts[i]] = std::move(node);
          }
        }
        leaf->subscribers.emplace_back(subscriber);
      } else {
        cur = it->second.get();
      }
    }

    if (newTree) {
      // add as a child,
      const auto& part    = newTree->topicPart;
      cur->children[part] = std::move(newTree);
    } else {
      // now add the subscriber
      cur->subscribers.emplace_back(subscriber);
    }
  }

  // detachChild detach children from the trie that has no subscribers
  // and no further children downwards
  void Trie::detachChild(Trie::Node& child) {
    if (child.parent == nullptr) {
      // root node
      return;
    }
    if (child.subscribers.empty() && child.children.empty()) {
      // remove the child
      Trie::Node* p = child.parent;
      p->children.erase(child.topicPart);
      // Try detaching again till we reaches the root or a node that has
      // children/subscribers
      detachChild(*p);
    }
  }

  void Trie::print() {
    Node* cur = &root;
    printNode(*cur);
  }

  void Trie::printNode(const Node& node) {
    for (const auto& pair : node.children) {
      std::cout << "part: " << pair.first
                << " children: " << pair.second->children.size()
                << " subscriber: " << pair.second->subscribers.size()
                << std::endl;
      this->printNode(*pair.second);
    }
  }

  void Trie::remove(const std::string&                topic,
                    std::shared_ptr<mqtt::Subscriber> subscriber) {
    std::vector<std::string>    parts = mqttutils::TopicUtils::split(topic);
    Node*                       cur   = &root;
    std::lock_guard<std::mutex> guard(this->mux);
    for (const std::string& part : parts) {
      std::unordered_map<std::string, std::unique_ptr<Node>>::iterator it =
          cur->children.find(part);
      if (it == cur->children.end()) {
        // no subscribers registered
        return;
      } else {
        cur = it->second.get();
      }
    }
    // remove the subscriber
    cur->subscribers.erase(std::remove_if(
        cur->subscribers.begin(),
        cur->subscribers.end(),
        [subscriber](const std::shared_ptr<mqtt::Subscriber>& item) {
          return item == subscriber;
        }));
    detachChild(*cur);
  }

  mqtt::Subscribers Trie::match(const std::string& topic) const {
    std::vector<std::string> parts = mqttutils::TopicUtils::split(topic);
    mqtt::Subscribers        subscribers;
    {
      std::lock_guard<std::mutex> guard(this->mux);
      this->match(parts, this->root, subscribers);
    }

    return subscribers;
  }

  static void addSubscribers(mqtt::Subscribers&       subscribers,
                             const mqtt::Subscribers& toAdd) {
    subscribers.insert(
        std::end(subscribers), std::begin(toAdd), std::end(toAdd));
  }

  void Trie::match(const std::vector<std::string>& parts,
                   const Node&                     node,
                   mqtt::Subscribers&              subscribers) const {

    // "foo/#” also matches the singular "foo", since # includes the parent
    // level.
    std::unordered_map<std::string, std::unique_ptr<Node>>::const_iterator it =
        node.children.find("#");
    if (it != node.children.end()) {
      addSubscribers(subscribers, it->second->subscribers);
    }

    if (parts.size() == 0) {
      addSubscribers(subscribers, node.subscribers);
      return;
    }

    // the single-level wildcard matches only a single level, “sport/+” does not
    // match “sport” but it does match “sport/”.
    it = node.children.find("+");
    if (it != node.children.end()) {
      // found +, check it is the last part
      // from MQTTv5 spec
      // e.g “sport/tennis/+” matches “sport/tennis/player1” and
      // “sport/tennis/player2”, but not “sport/tennis/player1/ranking”.
      if (parts.size() == 1) {
        addSubscribers(subscribers, it->second->subscribers);
        this->match(parts, *it->second.get(), subscribers);
      } else {
        this->match(std::vector<std::string>(parts.begin() + 1, parts.end()),
                    *it->second.get(),
                    subscribers);
      }
    }

    it = node.children.find(parts[0]);
    if (it != node.children.end()) {
      this->match(std::vector<std::string>(parts.begin() + 1, parts.end()),
                  *it->second.get(),
                  subscribers);
    }
  }

  // // -----------------------------------------------------------------------
  TopicMatcher::TopicMatcher() : trie(std::make_unique<Trie>()) {}

  std::error_code
  TopicMatcher::subscribe(const std::string&                topic,
                          std::shared_ptr<mqtt::Subscriber> subscriber) {
    std::error_code err = TopicUtils::validateSubscribeTopic(topic);
    if (err) {
      return err;
    }

    this->trie->insert(topic, subscriber);

    return mqtt::Error::Success;
  }

  std::error_code
  TopicMatcher::unsubscribe(const std::string&                topic,
                            std::shared_ptr<mqtt::Subscriber> subscriber) {
    std::error_code err = TopicUtils::validateSubscribeTopic(topic);
    if (err) {
      return err;
    }

    this->trie->remove(topic, subscriber);

    return mqtt::Error::Success;
  }

  void TopicMatcher::print() {
    this->trie->print();
  }

  mqtt::Subscribers TopicMatcher::match(const std::string& topic) const {
    return this->trie->match(topic);
  }

} // namespace mqttutils
