/* SPDX-License-Identifier: GPL-3.0-or-later
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
func (self *myDisp) CallHadler(msg ptpmgmt.Message, a ...interface{}) {
  ptpmgmt.MessageDispatcherCallHadler(self, msg, a)
}
func (self *myDisp) PRIORITY1_h(msg ptpmgmt.Message,
                               bTlv ptpmgmt.BaseMngTlv,
                               tlv_id string) {
  tlv := ptpmgmt.Conv_PRIORITY1(bTlv)
  self.funcVal += 1
  self.id = tlv_id
  self.priority1 = tlv.GetPriority1()
}
func (self *myDisp) NoTlv(msg ptpmgmt.Message) {
  self.funcVal += 2
}
func (self *myDisp) NoTlvCallBack(msg ptpmgmt.Message, tlv_id string) {
  self.funcVal += 4
  self.id = tlv_id
}

// Implement ptpmgmt.MessageBuilder
type myBuild struct {
  run int
}
func (self *myBuild) BuildTlv(msg ptpmgmt.Message,
                             actionField ptpmgmt.ActionField_e,
                             tlv_id ptpmgmt.Mng_vals_e) bool {
  return ptpmgmt.MessageBuilderBuildTlv(self, msg, actionField, tlv_id)
}
func (self *myBuild) PRIORITY1_b(msg ptpmgmt.Message) ptpmgmt.BaseMngTlv {
  tlv := ptpmgmt.NewPRIORITY1_t()
  self.run = 1
  tlv.SetPriority1(117)
  return tlv.SwigGetBaseMngTlv()
}

var msg ptpmgmt.Message = ptpmgmt.NewMessage()
var disp = new(myDisp)
var build = new(myBuild)

// Tests CallHadler with empty TLV
func TestParsedCallHadlerEmptyTLV(t *testing.T) {
  disp.init()
  disp.CallHadler(msg)
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
  defer msg.ClearData()
  defer ptpmgmt.DeletePRIORITY1_t(tlv)
  tlv.SetPriority1(117)
  disp.CallHadler(msg, ptpmgmt.PRIORITY1, tlv) // .SwigGetBaseMngTlv())
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
  defer msg.ClearData()
  defer ptpmgmt.DeletePRIORITY2_t(tlv)
  tlv.SetPriority2(117)
  disp.CallHadler(msg, ptpmgmt.PRIORITY2, tlv)
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
  if build.BuildTlv(msg, ptpmgmt.COMMAND, ptpmgmt.ENABLE_PORT) {
    defer msg.ClearData()
  } else {
    t.Errorf("TestBuildEmptyTLV build fail")
  }
  if build.run != 0 {
    t.Errorf("TestBuildEmptyTLV should not call PRIORITY1 callback")
  }
}

// Tests build TLV
func TestBuildTLV(t *testing.T) {
  build.run = 0
  if build.BuildTlv(msg, ptpmgmt.SET, ptpmgmt.PRIORITY1) {
    defer msg.ClearData()
  } else {
    t.Errorf("TestBuildTLV fail")
  }
  if build.run != 1 {
    t.Errorf("TestBuildTLV did not call PRIORITY1 callback")
  }
}

// Tests build TLV that lack callback
func TestBuildTLVNoCallback(t *testing.T) {
  build.run = 0
  if build.BuildTlv(msg, ptpmgmt.SET, ptpmgmt.PRIORITY2) {
    defer msg.ClearData()
    t.Errorf("TestBuildTLVNoCallback build should return false")
  }
  if build.run != 0 {
    t.Errorf("TestBuildTLVNoCallback should not call PRIORITY1 callback")
  }
}
