#include "properties.h"
#include <map>

namespace packet {
  using propertymap_t = std::map<Property::ID, const char*>;

  const char* Property::Text(ID id) {
    static propertymap_t& propertyMap = *new propertymap_t(
        {{Property::ID::PayloadFormatIndicatorID, "Payload format indicator"},
         {Property::ID::MessageExpiryIntervalID, "Message expiry interval"},
         {Property::ID::ContentTypeID, "Content type"},
         {Property::ID::ResponseTopicID, "response topic"},
         {Property::ID::CorrelationDataID, "Correlation data"},
         {Property::ID::SubscriptionIdentifierID, "Subscription Identifier"},
         {Property::ID::SessionExpiryIntervalID, "Session Expiry Interval"},
         {Property::ID::AssignedClientIdentifierID,
          "Assigned Client Identifier"},
         {Property::ID::ServerKeepAliveID, "Server Keep Alive"},
         {Property::ID::AuthenticationMethodID, "Authentication Method"},
         {Property::ID::AuthenticationDataID, "Authentication Data"},
         {Property::ID::RequestProblemInfoID, "Request Problem Information"},
         {Property::ID::RequestResponseInfoID, "Request Response Information"},
         {Property::ID::WillDelayIntervalID, "Will Delay Interval"},
         {Property::ID::ResponseInformationID, "Response Information"},
         {Property::ID::ServerReferenceID, "Server Reference"},
         {Property::ID::ReasonStringID, "Reason String"},
         {Property::ID::ReceiveMaximumID, "Receive Maximum"},
         {Property::ID::TopicAliasMaximumID, "Topic Alias Maximum"},
         {Property::ID::TopicAliasID, "Topic Alias"},
         {Property::ID::MaximumQoSID, "Maximum QoS"},
         {Property::ID::RetainAvailableID, "Retain Available"},
         {Property::ID::UserPropertyID, "User Property"},
         {Property::ID::MaximumPacketSizeID, "Maximum Packet Size"},
         {Property::ID::WildcardSubscriptionAvailableID,
          "Wildcard Subscription Available"},
         {Property::ID::SubscriptionIdentifierAvailableID,
          "Subscription Identifier Available"},
         {Property::ID::SharedSubscriptionAvailableID,
          "Shared Subscription Available"}});

    auto it = propertyMap.find(id);
    if (it != propertyMap.end()) {
      return it->second;
    }
    return "unknown property ID";
  }

} // namespace packet
