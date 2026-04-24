/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Unit test with go wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

package unit_test

import (
  "ptpmgmt"
  "testing"
)

// Implement ptpmgmt.MessageDispatcher interface
type myDisp struct {
  priority1 byte
  funcVal int
  id string
}
func (self *myDisp) init() {
  self.priority1 = 0
  self.funcVal = 0
  self.id = ""
}
// Callback of ptpmgmt.MessageDispatcher for a PRIORITY1 tlv
func (self *myDisp) PRIORITY1_h(msg ptpmgmt.Message,
                               tlv ptpmgmt.PRIORITY1_t,
                               tlv_id string) {
  self.funcVal += 1
  self.id = tlv_id
  self.priority1 = tlv.GetPriority1()
}
// Callback of ptpmgmt.MessageDispatcher for no TLV
func (self *myDisp) NoTlv(msg ptpmgmt.Message) {
  self.funcVal += 2
}
// Callback of ptpmgmt.MessageDispatcher for missing TLV callback
func (self *myDisp) NoTlvCallBack(msg ptpmgmt.Message, tlv_id string) {
  self.funcVal += 4
  self.id = tlv_id
}

// Implement ptpmgmt.MessageBuilder interface
type myBuild struct {
  run int
}
// Callback of ptpmgmt.MessageBuilder to build a new PRIORITY1 tlv
func (self *myBuild) PRIORITY1_b(msg ptpmgmt.Message,
                                 tlv ptpmgmt.PRIORITY1_t) bool {
  self.run += 1
  tlv.SetPriority1(117)
  return true
}

var msg ptpmgmt.Message = ptpmgmt.NewMessage()
var disp myDisp   // interface for ptpmgmt.MessageDispatcher Director
var build myBuild // interface for ptpmgmt.MessageBuilder Director

// Tests CallHadler with empty TLV
func TestParsedCallHadlerEmptyTLV(t *testing.T) {
  disp.init()
  dispDr := ptpmgmt.NewDirectorMessageDispatcher(&disp)
  defer ptpmgmt.DeleteDirectorMessageDispatcher(dispDr)
  dispDr.CallHadler(msg)
  if disp.funcVal != 2 {
    t.Errorf("TestParsedCallHadlerEmptyTLV wrong funcVal %d; did not call NoTlv",
             disp.funcVal)
  }
  if disp.id != "" {
    t.Errorf("TestParsedCallHadlerEmptyTLV wrong OD '%s'; want empty", disp.id)
  }
  if disp.priority1 != 0 {
    t.Errorf("TestParsedCallHadlerEmptyTLV wrong priority1 '%d'; want 0",
             disp.priority1)
  }
}

// Tests CallHadler method with TLV
func TestParsedCallHadlerTLV(t *testing.T) {
  disp.init()
  tlv := ptpmgmt.NewPRIORITY1_t()
  defer ptpmgmt.DeletePRIORITY1_t(tlv)
  defer msg.ClearData()
  tlv.SetPriority1(117)
  dispDr := ptpmgmt.NewDirectorMessageDispatcher(&disp)
  defer ptpmgmt.DeleteDirectorMessageDispatcher(dispDr)
  dispDr.CallHadler(msg, ptpmgmt.PRIORITY1, tlv)
  if disp.funcVal != 1 {
    t.Errorf("TestParsedCallHadlerTLV wrong funcVal %d; did not call PRIORITY1_h",
             disp.funcVal)
  }
  if disp.id != "PRIORITY1" {
    t.Errorf("TestParsedCallHadlerTLV wrong OD '%s'; want 'PRIORITY1'", disp.id)
  }
  if disp.priority1 != 117 {
    t.Errorf("TestParsedCallHadlerTLV wrong priority1 '%d'; want 117",
             disp.priority1)
  }
}

// Tests CallHadler method with TLV without callback
func TestParsedCallHadlerTLVNoCallback(t *testing.T) {
  disp.init()
  tlv := ptpmgmt.NewPRIORITY2_t()
  defer ptpmgmt.DeletePRIORITY2_t(tlv)
  defer msg.ClearData()
  tlv.SetPriority2(117)
  dispDr := ptpmgmt.NewDirectorMessageDispatcher(&disp)
  defer ptpmgmt.DeleteDirectorMessageDispatcher(dispDr)
  dispDr.CallHadler(msg, ptpmgmt.PRIORITY2, tlv)
  if disp.funcVal != 4 {
    t.Errorf("TestParsedCallHadlerTLVNoCallback wrong funcVal %d;" +
             " did not call NoTlvCallBack",
             disp.funcVal)
  }
  if disp.id != "PRIORITY2" {
    t.Errorf("TestParsedCallHadlerTLVNoCallback wrong OD '%s'; want 'PRIORITY2'",
             disp.id)
  }
  if disp.priority1 != 0 {
    t.Errorf("TestParsedCallHadlerTLVNoCallback wrong priority1 '%d'; want 0",
             disp.priority1)
  }
}

// Tests build empty TLV
func TestBuildEmptyTLV(t *testing.T) {
  build.run = 0
  buildDr := ptpmgmt.NewDirectorMessageBuilder(&build, msg)
  defer ptpmgmt.DeleteDirectorMessageBuilder(buildDr)
  defer buildDr.Clear()
  if !buildDr.BuildTlv(ptpmgmt.COMMAND, ptpmgmt.ENABLE_PORT) {
    t.Errorf("TestBuildEmptyTLV build fail")
  }
  if build.run != 0 {
    t.Errorf("TestBuildEmptyTLV should not call PRIORITY1 callback")
  }
}

// Tests build TLV
func TestBuildTLV(t *testing.T) {
  build.run = 0
  buildDr := ptpmgmt.NewDirectorMessageBuilder(&build, msg)
  defer ptpmgmt.DeleteDirectorMessageBuilder(buildDr)
  defer buildDr.Clear()
  if !buildDr.BuildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY1) {
    t.Errorf("TestBuildTLV fail")
  }
  if build.run != 1 {
    t.Errorf("TestBuildTLV did not call PRIORITY1 callback")
  }
}

// Tests build TLV twice
// Call build twice to verify the first TLV is free on seconds call!
func TestBuildTLVtwice(t *testing.T) {
  build.run = 0
  buildDr := ptpmgmt.NewDirectorMessageBuilder(&build, msg)
  defer ptpmgmt.DeleteDirectorMessageBuilder(buildDr)
  defer buildDr.Clear()
  if !buildDr.BuildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY1) {
    t.Errorf("TestBuildTLVtwice first build fail")
  }
  if !buildDr.BuildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY1) {
    t.Errorf("TestBuildTLVtwice second build fail")
  }
  if build.run != 2 {
    t.Errorf("TestBuildTLVtwice did not call PRIORITY1 callback twice")
  }
}

// Tests build TLV that lack callback
func TestBuildTLVNoCallback(t *testing.T) {
  build.run = 0
  buildDr := ptpmgmt.NewDirectorMessageBuilder(&build, msg)
  defer ptpmgmt.DeleteDirectorMessageBuilder(buildDr)
  defer buildDr.Clear()
  if buildDr.BuildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY2) {
    t.Errorf("TestBuildTLVNoCallback build should return false")
  }
  if build.run != 0 {
    t.Errorf("TestBuildTLVNoCallback should not call PRIORITY1 callback")
  }
}
