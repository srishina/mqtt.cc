#pragma once

#include "codec.h"
#include "utils.h"
#include <map>
#include <optional>
#include <sstream>
#include <type_traits>

namespace packet {
  class Property {
  public:
    enum class ID : uint8_t {
      // PayloadFormatIndicatorID Payload format indicator
      PayloadFormatIndicatorID = 0x01,

      // MessageExpiryIntervalID Message expiry interval
      MessageExpiryIntervalID = 0x02,

      // ContentTypeID Content type
      ContentTypeID = 0x03,

      // ResponseTopic response topic
      ResponseTopicID = 0x08,

      // CorrelationDataID Correlation data
      CorrelationDataID = 0x09,

      // SubscriptionIdentifierID Subscription Identifier
      SubscriptionIdentifierID = 0x0B,

      // SessionExpiryIntervalID session expiry property  identifier
      SessionExpiryIntervalID = 0x11,

      // AssignedClientIdentifierID Assigned Client Identifier
      AssignedClientIdentifierID = 0x12,

      // ServerKeepAliveID Server Keep Alive
      ServerKeepAliveID = 0x13,

      // AuthenticationMethodID maximum packet size id
      AuthenticationMethodID = 0x15,

      // AuthenticationDataID maximum packet size id
      AuthenticationDataID = 0x16,

      // RequestProblemInfoID maximum packet size id
      RequestProblemInfoID = 0x17,

      // WillDelayIntervalID Will Delay Interval
      WillDelayIntervalID = 0x18,

      // RequestResponseInfoID maximum packet size id
      RequestResponseInfoID = 0x19,

      // ResponseInformationID Response Information
      ResponseInformationID = 0x1A,

      // ServerReferenceID Server Reference
      ServerReferenceID = 0x1C,
      // ReasonStringID Reason String
      ReasonStringID = 0x1F,

      // ReceiveMaximumID receive maximum id
      ReceiveMaximumID = 0x21,

      // TopicAliasMaximumID maximum packet size id
      TopicAliasMaximumID = 0x22,

      // TopicAliasID Topic Alias
      TopicAliasID = 0x23,

      // MaximumQoSID Maximum QoS
      MaximumQoSID = 0x24,

      // RetainAvailableID Retain Available
      RetainAvailableID = 0x25,

      // UserPropertyID User property id
      UserPropertyID = 0x26,

      // MaximumPacketSizeID maximum packet size id
      MaximumPacketSizeID = 0x27,

      // WildcardSubscriptionAvailableID Wildcard Subscription Available
      WildcardSubscriptionAvailableID = 0x28,

      // SubscriptionIdentifierAvailableID Subscription Identifier Available
      SubscriptionIdentifierAvailableID = 0x29,

      // SharedSubscriptionAvailableID Shared Subscription Available
      SharedSubscriptionAvailableID = 0x2A,
    };

    static const char* Text(ID id);

    template <typename T, bool varuint32 = false>
    static uint32_t size(const T& value);

    template <typename T, bool varuint32 = false>
    static uint32_t sizeMoreThanOnce(const T& value);

    template <typename T, bool varuint32 = false>
    static uint32_t valueSize(const T& value);

    template <typename T, bool varuint32 = false>
    static void encode(Encoder& enc, ID id, const T& value);

    template <typename T, bool varuint32 = false>
    static uint32_t decode(Decoder& dec, ID id, T& value);
  };

  inline uint32_t propertyIDSize() {
    return 1;
  }

  inline void throwErrorIfNotEmpty(Property::ID id, bool empty) {
    if (!empty) {
      std::ostringstream stream;
      stream << Property::Text(id) << "  must not be included more than once";
      throw std::runtime_error(stream.str());
    }
  }

  [[noreturn]] inline void throwInvalidPropertyID(Property::ID id,
                                                  const char* packetName) {
    std::ostringstream stream;
    stream << Property::Text(id) << "  wrong property with identifier "
           << Property::Text(id) << " in " << packetName;
    throw std::runtime_error(stream.str());
  }

  namespace detail {
    template <typename T, typename /*U*/ = void>
    struct is_optional : std::false_type {};

    template <typename T>
    struct is_optional<std::optional<T>> : std::true_type {};

    // specialize only for std::vector<uint8_t or uint32_t> or std::string
    template <typename T> struct is_stl_container : std::false_type {};

    template <>
    struct is_stl_container<std::vector<uint8_t>> : std::true_type {};

    template <>
    struct is_stl_container<std::vector<uint32_t>> : std::true_type {};

    template <> struct is_stl_container<std::string> : std::true_type {};
  } // namespace detail

  template <typename T>
  constexpr bool is_optional = detail::is_optional<T>::value;

  template <typename T>
  constexpr bool is_stl_container = detail::is_stl_container<T>::value;

  // returns the property value size and the ID size
  template <typename T, bool varuint32>
  inline uint32_t Property::size(const T& value) {
    uint32_t valueSize = Property::valueSize<T, varuint32>(value);
    return (valueSize > 0) ? valueSize + propertyIDSize() : valueSize;
  }

  template <typename T, bool varuint32>
  inline uint32_t Property::sizeMoreThanOnce(const T& value) {
    static_assert(is_stl_container<T> &&
                      std::is_pod<typename T::value_type>::value,
                  "Property type must be std::vector<T>  where T = POD");
    uint32_t valueSize = 0;
    for (auto v : value) {
      if constexpr (varuint32) {
        valueSize += EncodedVarUint32::size(v);
      } else {
        valueSize += sizeof(typename T::value_type);
      }
    }
    return (valueSize > 0) ? valueSize + (static_cast<uint32_t>(value.size()) *
                                          propertyIDSize())
                           : valueSize;
  }

  // returns the size of the property value, if value is empty the function
  // returns 0
  template <typename T, bool varuint32>
  inline uint32_t Property::valueSize(const T& value) {
    if constexpr (is_optional<T>) {
      static_assert(std::is_pod<typename T::value_type>::value,
                    "underlying type of std::optional must be POD");
      if (value) {
        if constexpr (varuint32) {
          return EncodedVarUint32::size(*value);
        } else {
          return sizeof(typename T::value_type);
        }
      }
    } else if constexpr (is_stl_container<T>) {
      return value.empty() ? 0 : (static_cast<uint32_t>(value.size()) + 2);
    }
    return 0;
  }

  template <typename T, bool varuint32>
  inline void Property::encode(Encoder& enc, ID id, const T& value) {
    static_assert(is_optional<T> || is_stl_container<T>,
                  "Property type must be std::optional or std::vector<uint8_t> "
                  "or std::string ");

    if constexpr (is_optional<T>) {
      static_assert(std::is_pod<typename T::value_type>::value,
                    "underlying type of std::optional must be POD");
      if (value) {
        enc.writeVarUint32(toUnderlyingType(id));
        if constexpr (varuint32) {
          enc.writeVarUint32(*value);
        } else {
          enc.write(*value);
        }
      }
    } else if constexpr (is_stl_container<T>) {
      if (!value.empty()) {
        enc.writeVarUint32(toUnderlyingType(id));
        if constexpr (varuint32) {
          enc.writeVarUint32(value);
        } else {
          enc.write(value);
        }
      }
    }
  }

  template <typename T, bool varuint32>
  inline uint32_t Property::decode(Decoder& dec, ID id, T& value) {
    static_assert(is_optional<T> || is_stl_container<T>,
                  "Property type must be std::optional or std::vector<uint8_t> "
                  "or std::string ");

    if constexpr (is_optional<T>) {
      static_assert(std::is_pod<typename T::value_type>::value,
                    "underlying type of std::optional must be POD");
      throwErrorIfNotEmpty(id, !value);
      value = dec.read<typename T::value_type, varuint32>();
    } else if constexpr (is_stl_container<T>) {
      throwErrorIfNotEmpty(id, value.empty());
      value = dec.read<T>();
    }
    return Property::valueSize<T, varuint32>(value);
  }
} // namespace packet
