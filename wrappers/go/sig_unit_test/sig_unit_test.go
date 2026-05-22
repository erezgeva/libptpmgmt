/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Unit test with signaling with go wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

package unit_test

import (
  "ptpmgmt"
  "testing"
  "utest_help"
)

type MySigCb struct { // Implement ptpmgmt.MessageSigTlvCallback interface
  mask byte
}
func (self *MySigCb) Callback(msg ptpmgmt.Message, tlvType ptpmgmt.TlvType_e,
    sigTlv ptpmgmt.BaseSigTlv) bool {
  if tlvType != ptpmgmt.MANAGEMENT {
    return true // return true on failure!
  }
  id := ptpmgmt.Get_MngTlvId(sigTlv)
  tlv := ptpmgmt.Get_BaseMngTlv(sigTlv)
  // First TLV
  if id == ptpmgmt.PRIORITY2 {
    pr2 := ptpmgmt.Conv_PRIORITY2(tlv)
    self.mask += 1
    return pr2.GetPriority2() != 119; // return false on success!
  }
  // Second TLV
  if id == ptpmgmt.DOMAIN {
    domain := ptpmgmt.Conv_DOMAIN(tlv)
    self.mask += 10
    return domain.GetDomainNumber() != 7; // return false on success!
  }
  return true; // return true on failure!
}

// Tests CallHadler with empty TLV
func TestPtpmgmtTraverseSig(t *testing.T) {
  msg := ptpmgmt.NewMessage()
  defer ptpmgmt.DeleteMessage(msg)
  buf := ptpmgmt.NewBuf(int64(100))
  defer ptpmgmt.DeleteBuf(buf)
  size := utest_help.Get2MngTlvsSig(buf.Get(), buf.Size())
  if size == 0 { t.Errorf("get2MngTlvsSig") }
  prms := msg.GetParams()
  prms.SetRcvSignaling(true)
  prms.SetFilterSignaling(false)
  if !msg.UpdateParams(prms) { t.Errorf("updateParams") }
  if msg.Parse(buf, size) != ptpmgmt.MNG_PARSE_ERROR_SIG { t.Errorf("parse") }
  if msg.GetSigTlvsCount() != 2 { t.Errorf("getSigTlvsCount") }
  if msg.GetSigTlvType(0) != ptpmgmt.MANAGEMENT { t.Errorf("getSigTlvType") }
  if msg.GetSigMngTlvType(0) != ptpmgmt.PRIORITY2 { t.Errorf("getSigMngTlvType") }
  if msg.GetSigTlvType(1) != ptpmgmt.MANAGEMENT { t.Errorf("getSigTlvType") }
  if msg.GetSigMngTlvType(1) != ptpmgmt.DOMAIN { t.Errorf("getSigMngTlvType") }
  sigTlv := msg.GetSigTlv(0)
  if ptpmgmt.Get_MngTlvId(sigTlv) != ptpmgmt.PRIORITY2 { t.Errorf("get_MngTlvId") }
  mngTlv1_1 := msg.GetSigMngTlv(0)
  mngTlv1_2 := ptpmgmt.Get_BaseMngTlv(sigTlv)
  pr2_1 := ptpmgmt.Conv_PRIORITY2(mngTlv1_1)
  pr2_2 := ptpmgmt.Conv_PRIORITY2(mngTlv1_2)
  if pr2_1.GetPriority2() != 119 { t.Errorf("pr2_1.priority2") }
  if pr2_2.GetPriority2() != 119 { t.Errorf("pr2_2.priority2") }
  sigTlv = msg.GetSigTlv(1)
  if ptpmgmt.Get_MngTlvId(sigTlv) != ptpmgmt.DOMAIN { t.Errorf("get_MngTlvId") }
  mngTlv1_1 = msg.GetSigMngTlv(1)
  mngTlv1_2 = ptpmgmt.Get_BaseMngTlv(sigTlv)
  domain1 := ptpmgmt.Conv_DOMAIN(mngTlv1_1)
  domain2 := ptpmgmt.Conv_DOMAIN(mngTlv1_2)
  if domain1.GetDomainNumber() != 7 { t.Errorf("domain1.domainNumber") }
  if domain2.GetDomainNumber() != 7 { t.Errorf("domain2.domainNumber") }
  var cbGo MySigCb // Local ptpmgmt.MessageSigTlvCallback interface implementation
  cb := ptpmgmt.NewDirectorMessageSigTlvCallback(&cbGo)
  defer ptpmgmt.DeleteDirectorMessageSigTlvCallback(cb)
  if msg.TraversSigTlvsCl(cb) { t.Errorf("traversSigTlvsCl") }
  if cbGo.mask != 11 { t.Errorf("mySigCb mask") }
}
