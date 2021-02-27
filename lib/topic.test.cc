#include "doctest/doctest.h"
#include "topic.h"
#include <mqtt/error.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

TEST_CASE("testing publish topic validation") {
  std::vector<std::string> validPublishTopics{
      "pub/topic", "pub//topic", "pub/ /topic"};

  for (const auto& topic : validPublishTopics) {
    CHECK(mqttutils::TopicUtils::validatePublishTopic(topic) ==
          mqtt::Error::Success);
  }

  std::vector<std::string> invalidPublishTopics{"+pub/topic",
                                                "pub+/topic",
                                                "pub/+topic",
                                                "pub/topic+",
                                                "pub/topic/+",
                                                "#pub/topic",
                                                "pub#/topic",
                                                "pub/#topic",
                                                "pub/topic#",
                                                "pub/topic/#",
                                                "+/pub/topic"};
  for (const auto& topic : invalidPublishTopics) {
    CHECK(mqttutils::TopicUtils::validatePublishTopic(topic) ==
          mqtt::Error::InvalidTopic);
  }
}

TEST_CASE("testing subscribe topic validation") {
  std::vector<std::string> validSubscribeTopics{"sub/topic",
                                                "sub//topic",
                                                "sub/ /topic",
                                                "sub/+/topic",
                                                "+/+/+",
                                                "+",
                                                "sub/topic/#",
                                                "sub//topic/#",
                                                "sub/ /topic/#",
                                                "sub/+/topic/#",
                                                "+/+/+/#",
                                                "#",
                                                "/#",
                                                "sub/topic/+/#"};
  for (const auto& topic : validSubscribeTopics) {
    CHECK(mqttutils::TopicUtils::validateSubscribeTopic(topic) ==
          mqtt::Error::Success);
  }
  std::vector<std::string> invalidSubscribeTopics{"+sub/topic",
                                                  "sub+/topic",
                                                  "sub/+topic",
                                                  "sub/topic+",
                                                  "#sub/topic",
                                                  "sub#/topic",
                                                  "sub/#topic",
                                                  "sub/topic#",
                                                  "#/sub/topic",
                                                  ""};
  for (const auto& topic : invalidSubscribeTopics) {
    CHECK(mqttutils::TopicUtils::validateSubscribeTopic(topic) !=
          mqtt::Error::Success);
  }
}

TEST_CASE("testing split topic") {
  std::map<std::string, std::vector<std::string>> elements = {
      {"foo/bar", std::vector<std::string>{"foo", "bar"}},
      {"foo//bar", std::vector<std::string>{"foo", "", "bar"}},
      {"foo//bar/", std::vector<std::string>{"foo", "", "bar", ""}},
      {"foo/bar/", std::vector<std::string>{"foo", "bar", ""}},
      {"sub/+/topic/#", std::vector<std::string>{"sub", "+", "topic", "#"}}};

  for (std::pair<std::string, std::vector<std::string>> element : elements) {
    CHECK(mqttutils::TopicUtils::split(element.first) == element.second);
  }
}

class SubscriberImpl : public mqtt::Subscriber {
public:
  virtual void onData() override final;
};

void SubscriberImpl::onData() {}

static void matchHelper(std::string topicFilter, std::string topic) {
  mqttutils::TopicMatcher matcher;
  auto                    s = std::make_shared<SubscriberImpl>();
  CHECK(matcher.subscribe(topicFilter, s) == mqtt::Error::Success);
  auto subscribers = matcher.match(topic);
  CHECK_MESSAGE(subscribers.size() == 1,
                "match failed Topic Filter: ",
                topicFilter,
                " Matching Topic: ",
                topic);
}

TEST_CASE("testing trie match") {
  matchHelper("foo/#", "foo/");
  matchHelper("foo/+/#", "foo/bar/baz");
  matchHelper("#", "foo/bar/baz");
  matchHelper("/#", "/foo/bar");

  std::map<std::string, std::string> subscribeTopics = {
      {"foo/#", "foo"},
      {"foo//bar", "foo//bar"},
      {"foo//+", "foo//bar"},
      {"foo/+/+/baz", "foo///baz"},
      {"foo/bar/+", "foo/bar/"},
      {"foo/bar", "foo/bar"},
      {"foo/+", "foo/bar"},
      {"foo/+/baz", "foo/bar/baz"},
      {"A/B/+/#", "A/B/B/C"},
      {"foo/+/#", "foo/bar"},
      {"#", "foo/bar/baz"},
      {"/#", "/foo/bar"}};

  for (std::pair<std::string, std::string> subscribeTopic : subscribeTopics) {
    matchHelper(subscribeTopic.first, subscribeTopic.second);
  }
}

static void noMatchHelper(std::string topicFilter, std::string topic) {
  mqttutils::TopicMatcher matcher;
  auto                    s = std::make_shared<SubscriberImpl>();
  if (matcher.subscribe(topicFilter, s) != mqtt::Error::Success) {
    // some of the subscribe topicFilters are malformed, so Subscribe can return
    // InvalidTopic or EmptySubscriptionTopic
    return;
  }
  auto subscribers = matcher.match(topic);
  CHECK_MESSAGE(subscribers.size() == 0,
                "match failed Topic Filter: ",
                topicFilter,
                " Matching Topic: ",
                topic);
}

TEST_CASE("testing trie no-match") {
  std::map<std::string, std::string> subscribeTopics = {
      {"test/6/#", "test/3"},
      {"test/6/#", "test/3"},
      {"test/6/#", "test/3"},
      {"test/6/#", "test/^^3"},
      {"foo/bar", "foo"},
      {"foo/+", "foo/bar/baz"},
      {"foo/+/baz", "foo/bar/bar"},
      {"foo/+/#", "fo2/bar/baz"},
      {"/#", "foo/bar"},
      {"+foo", "+foo"},
      {"fo+o", "fo+o"},
      {"foo+", "foo+"},
      {"+foo/bar", "+foo/bar"},
      {"foo+/bar", "foo+/bar"},
      {"foo/+bar", "foo/+bar"},
      {"foo/bar+", "foo/bar+"},
      {"+foo", "afoo"},
      {"fo+o", "foao"},
      {"foo+", "fooa"},
      {"+foo/bar", "afoo/bar"},
      {"foo+/bar", "fooa/bar"},
      {"foo/+bar", "foo/abar"},
      {"foo/bar+", "foo/bara"},
      {"#foo", "#foo"},
      {"fo#o", "fo#o"},
      {"foo#", "foo#"},
      {"#foo/bar", "#foo/bar"},
      {"foo#/bar", "foo#/bar"},
      {"foo/#bar", "foo/#bar"},
      {"foo/bar#", "foo/bar#"},
      {"foo+", "fooa"}};
  for (std::pair<std::string, std::string> subscribeTopic : subscribeTopics) {
    noMatchHelper(subscribeTopic.first, subscribeTopic.second);
  }
}

TEST_CASE("testing trie match sub/unsub") {
  mqttutils::TopicMatcher matcher;
  auto                    s = std::make_shared<SubscriberImpl>();
  CHECK(matcher.subscribe("sport/tennis/player1/#", s) == mqtt::Error::Success);
  // MUST match  “sport/tennis/player1”
  // “sport/tennis/player1/ranking"
  // “sport/tennis/player1/score/wimbledon”
  CHECK(matcher.match("sport/tennis/player1").size() == 1);
  CHECK(matcher.match("sport/tennis/player1/ranking").size() == 1);
  CHECK(matcher.match("sport/tennis/player1/score/wimbledon").size() == 1);
  CHECK(matcher.unsubscribe("sport/tennis/player1/#", s) ==
        mqtt::Error::Success);
  CHECK(matcher.match("sport/tennis/player1").size() == 0);

  CHECK(matcher.subscribe("sport/tennis/+", s) == mqtt::Error::Success);
  // matches “sport/tennis/player1” and “sport/tennis/player2”, but not
  // “sport/tennis/player1/ranking”
  CHECK(matcher.match("sport/tennis/player1").size() == 1);
  CHECK(matcher.match("sport/tennis/player2").size() == 1);
  CHECK(matcher.match("sport/tennis/player1/ranking").size() == 0);
  CHECK(matcher.unsubscribe("sport/tennis/+", s) == mqtt::Error::Success);
  CHECK(matcher.match("sport/tennis/player1").size() == 0);

  CHECK(matcher.subscribe("sport/+", s) == mqtt::Error::Success);
  CHECK(matcher.match("sport").size() == 0);
  CHECK(matcher.match("sport/").size() == 1);
  CHECK(matcher.unsubscribe("sport/+", s) == mqtt::Error::Success);
}
