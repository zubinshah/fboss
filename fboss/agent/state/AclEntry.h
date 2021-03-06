/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include "fboss/agent/gen-cpp/switch_config_types.h"
#include "fboss/agent/types.h"
#include "fboss/agent/state/NodeBase.h"

#include <string>
#include <folly/IPAddress.h>

namespace facebook { namespace fboss {

struct AclEntryFields {
  explicit AclEntryFields(AclEntryID id) : id(id) {}

  template<typename Fn>
  void forEachChild(Fn) {}

  folly::dynamic toFollyDynamic() const;
  static AclEntryFields fromFollyDynamic(const folly::dynamic& json);

  const AclEntryID id{0};
  folly::CIDRNetwork srcIp;
  folly::CIDRNetwork dstIp;
  uint16_t l4SrcPort;
  uint16_t l4DstPort;
  uint8_t proto;
  uint8_t tcpFlags;
  uint8_t tcpFlagsMask;
  cfg::AclAction action{cfg::AclAction::PERMIT};
};

/*
 * AclEntry stores state about one of the access control entries on
 * the switch.
 */
class AclEntry :
    public NodeBaseT<AclEntry, AclEntryFields> {
 public:
  explicit AclEntry(AclEntryID id);
  static std::shared_ptr<AclEntry>
  fromFollyDynamic(const folly::dynamic& json) {
    const auto& fields = AclEntryFields::fromFollyDynamic(json);
    return std::make_shared<AclEntry>(fields);
  }

  static std::shared_ptr<AclEntry>
  fromJson(const folly::fbstring& jsonStr) {
    return fromFollyDynamic(folly::parseJson(jsonStr));
  }

  folly::dynamic toFollyDynamic() const override {
    return getFields()->toFollyDynamic();
  }

  AclEntryID getID() const {
    return getFields()->id;
  }

  cfg::AclAction getAction() const {
    return getFields()->action;
  }

  void setAction(const cfg::AclAction& action) {
    writableFields()->action = action;
  }

  folly::CIDRNetwork getSrcIp() const {
    return getFields()->srcIp;
  }

  void setSrcIp(const folly::CIDRNetwork& ip) {
    writableFields()->srcIp = ip;
  }

  folly::CIDRNetwork getDstIp() const {
    return getFields()->dstIp;
  }

  void setDstIp(const folly::CIDRNetwork& ip) {
    writableFields()->dstIp = ip;
  }

  uint16_t getL4SrcPort() const {
    return getFields()->l4SrcPort;
  }

  void setL4SrcPort(const uint16_t& port) {
    writableFields()->l4SrcPort = port;
  }

  uint16_t getL4DstPort() const {
    return getFields()->l4DstPort;
  }

  void setL4DstPort(const uint16_t& port) {
    writableFields()->l4DstPort = port;
  }

  uint16_t getProto() const {
    return getFields()->proto;
  }

  void setProto(const uint16_t& proto) {
    writableFields()->proto = proto;
  }

  uint16_t getTcpFlags() const {
    return getFields()->tcpFlags;
  }

  void setTcpFlags(const uint16_t& flags) {
    writableFields()->tcpFlags = flags;
  }

  uint16_t getTcpFlagsMask() const {
    return getFields()->tcpFlagsMask;
  }

  void setTcpFlagsMask(const uint16_t& flagsMask) {
    writableFields()->tcpFlagsMask = flagsMask;
  }

 private:
  // Inherit the constructors required for clone()
  using NodeBaseT::NodeBaseT;
  friend class CloneAllocator;
};

}} // facebook::fboss
