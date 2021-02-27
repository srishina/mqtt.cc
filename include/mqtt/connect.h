#pragma once

#include <optional>
#include <string>
#include <vector>

namespace mqttutils {
  class Encoder;
  class Decoder;
} // namespace mqttutils

namespace mqtt {
  struct Connect {
    struct Properties;
    uint8_t                              protocolVersion{0x05};
    bool                                 cleanStart{true};
    uint16_t                             keepAlive{0};
    std::shared_ptr<Connect::Properties> properties;
    std::string                          protocolName;
    std::string                          clientID;
    std::string                          userName;
    std::vector<uint8_t>                 password;
  };

  struct Connect::Properties {
    std::optional<uint32_t> sessionExpiryInterval;
    std::optional<uint16_t> receiveMaximum;
    std::optional<uint32_t> maximumPacketSize;
    std::optional<uint16_t> topicAliasMaximum;
    std::optional<bool>     requestProblemInfo;
    std::optional<bool>     requestResponseInfo;
    std::string             authenticationMethod;
    std::vector<uint8_t>    authenticationData;
  };

  //   class ConnectProperties : public noncopyable {
  //     friend class Connect;

  //   public:
  //     ~ConnectProperties();

  //     const std::optional<uint32_t>& getSessionExpiryInterval() const;
  //     const std::optional<uint16_t>& getReceiveMaximum() const;
  //     const std::optional<uint32_t>& getMaximumPacketSize() const;
  //     const std::optional<uint16_t>& getTopicAliasMaximum() const;
  //     const std::optional<bool>& getRequestProblemInfo() const;
  //     const std::optional<bool>& getRequestResponseInfo() const;
  //     const std::string& getAuthenticationMethod() const;
  //     const std::vector<uint8_t>& getAuthenticationData() const;

  //   private:
  //     uint32_t size() const;
  //     void encode(mqttutils::Encoder& enc) const;
  //     void decode(mqttutils::Decoder& dec, uint32_t propertyLen);

  //   private:
  //     std::optional<uint32_t> sessionExpiryInterval;
  //     std::optional<uint16_t> receiveMaximum;
  //     std::optional<uint32_t> maximumPacketSize;
  //     std::optional<uint16_t> topicAliasMaximum;
  //     std::optional<bool> requestProblemInfo;
  //     std::optional<bool> requestResponseInfo;
  //     std::string authenticationMethod;
  //     std::vector<uint8_t> authenticationData;
  //   };

  //   class Connect : public noncopyable {
  //     friend class ConnectBuilder;

  //   public:
  //     uint8_t getProtocolVersion() const;
  //     bool isCleanStart() const;
  //     uint16_t getKeepAlive() const;
  //     bool hasProperties() const;
  //     const ConnectProperties& getProperties() const;
  //     const std::string& getClientID() const;
  //     const std::string& getUserName() const;
  //     const std::vector<uint8_t>& getPassword() const;

  //   private:
  //     explicit Connect(bool cleanStart);
  //     uint32_t propertySize() const;
  //     void encodeProperties(mqttutils::Encoder& enc) const;
  //     void writeTo(std::ostream& ostream) const;
  //     void readFrom(std::istream& istream);

  //   private:
  //     uint8_t protocolVersion;
  //     bool cleanStart;
  //     uint16_t keepAlive;
  //     std::unique_ptr<ConnectProperties> properties;
  //     std::string protocolName;
  //     std::string clientID;
  //     std::string userName;
  //     std::vector<uint8_t> password;
  //   };

  //   class ConnectBuilder {
  //   public:
  //     ConnectBuilder(bool cleanStart) : mqttConnect(new Connect(cleanStart))
  //     {}

  //     ConnectBuilder& withKeepAlive(uint16_t keepAlive) {
  //       mqttConnect->keepAlive = keepAlive;
  //       return *this;
  //     }

  //     ConnectBuilder& withClientID(const std::string& cid) {
  //       mqttConnect->clientID = cid;
  //       return *this;
  //     }

  //     ConnectBuilder& withUsername(const std::string& uname) {
  //       mqttConnect->userName = uname;
  //       return *this;
  //     }

  //     ConnectBuilder& withPassword(const std::vector<uint8_t>& pwd) {
  //       mqttConnect->password = pwd;
  //       return *this;
  //     }

  //     std::unique_ptr<Connect> build() {

  //       return std::move(mqttConnect);
  //     }

  //   private:
  //     std::unique_ptr<Connect> mqttConnect;
  //   };
} // namespace mqtt
