/*
 * Copyright (c) 2004-present, Facebook, Inc.
 * Copyright (c) 2016, Cavium, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 * 
 */
#include "SaiPortBase.h"
#include "SaiSwitch.h"
#include "SaiPlatformPort.h"
#include "SaiError.h"

extern "C" {
#include "sai.h"
}

namespace facebook { namespace fboss {

SaiPortBase::SaiPortBase(SaiSwitch *hw, sai_object_id_t saiPortId, PortID fbossPortId, SaiPlatformPort *platformPort)
  : hw_(hw),
    platformPort_(platformPort),
    saiPortId_(saiPortId),
    fbossPortId_(fbossPortId) {
  VLOG(6) << "Entering " << __FUNCTION__;

  sai_api_query(SAI_API_PORT, (void **) &saiPortApi_);
}

SaiPortBase::~SaiPortBase() {
  VLOG(4) << "Entering " << __FUNCTION__;
}

void SaiPortBase::init(bool warmBoot) {
  VLOG(6) << "Entering " << __FUNCTION__;

  try {
    setIngressVlan(pvId_);
  } catch (const SaiError &e) {
    LOG(ERROR) << e.what();
  }

  disable();

  initDone_ = true;
}

void SaiPortBase::setPortStatus(bool linkStatus) {
  VLOG(6) << "Entering " << __FUNCTION__;

  if (initDone_ && (linkStatus_ == linkStatus)) {
    return;
  }

  VLOG(6) << "Set port: " << fbossPortId_.t << " status to: " << linkStatus;
  // We ignore the return value.  If we fail to get the port status
  // we just tell the platformPort_ that it is enabled.
  platformPort_->linkStatusChanged(linkStatus, adminMode_);
  linkStatus_ = linkStatus;
}

void SaiPortBase::setIngressVlan(VlanID vlan) {
  VLOG(6) << "Entering " << __FUNCTION__;

  if (initDone_ && (pvId_ == vlan)) {
    return;
  }

  sai_status_t saiStatus = SAI_STATUS_SUCCESS;
  sai_attribute_t attr;

  attr.id = SAI_PORT_ATTR_PORT_VLAN_ID;
  attr.value.u16 = vlan.t;

  saiStatus = saiPortApi_->set_port_attribute(saiPortId_, &attr);
  if(SAI_STATUS_SUCCESS != saiStatus) {
    throw SaiError("Failed to update ingress VLAN for port ", fbossPortId_.t);
  }

  VLOG(6) << "Set port: " << fbossPortId_.t << " ingress VLAN to: " << vlan.t;

  pvId_ = vlan;
}

void SaiPortBase::enable() {
  if (isEnabled()) {
    // Port is already enabled, don't need to do anything
    return;
  }

  sai_attribute_t attr {0};
  attr.id = SAI_PORT_ATTR_ADMIN_STATE;
  attr.value.booldata = true;
  
  sai_status_t saiStatus = saiPortApi_->set_port_attribute(saiPortId_, &attr);
  if(SAI_STATUS_SUCCESS != saiStatus) {
    LOG(ERROR) << "Failed to enable port admin mode. Error: " << saiStatus;
  }

  adminMode_ = true;
  VLOG(3) << "Enabled port: " << fbossPortId_;
  // TODO: Temporary solution. Will need to get operational status from HW.
  setPortStatus(adminMode_);
}

void SaiPortBase::disable() {
  if (initDone_ && !isEnabled()) {
    // Port is already disabled, don't need to do anything
    return;
  }

  sai_attribute_t attr {0};
  attr.id = SAI_PORT_ATTR_ADMIN_STATE;
  attr.value.booldata = false;
  
  sai_status_t saiStatus = saiPortApi_->set_port_attribute(saiPortId_, &attr);
  if(SAI_STATUS_SUCCESS != saiStatus) {
    LOG(ERROR) << "Failed to disable port admin mode. Error: " << saiStatus;
  }

  adminMode_ = false;
  VLOG(3) << "Disabled port: " << fbossPortId_;
  // TODO: Temporary solution. Will need to get operational status from HW.
  setPortStatus(adminMode_);
}

std::string SaiPortBase::statName(folly::StringPiece name) const {
  VLOG(6) << "Entering " << __FUNCTION__;

  return folly::to<std::string>("port", platformPort_->getPortID(), ".", name);
}

void SaiPortBase::updateStats() {
  VLOG(6) << "Entering " << __FUNCTION__;
}

}} // facebook::fboss
