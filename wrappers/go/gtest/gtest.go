/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief testing for go wrapper of libptpmgmt
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

package main

import (
  "ptpmgmt"
  "fmt"
  "os"
)

type myDisp struct { // Implement ptpmgmt.MessageDispatcher interface
}
func (self *myDisp) CallHadler(msg ptpmgmt.Message, a ...interface{}) {
  ptpmgmt.MessageDispatcherCallHadler(self, msg, a)
}
func (self *myDisp) PRIORITY1_h(msg ptpmgmt.Message,
                               bTlv ptpmgmt.BaseMngTlv,
                               tlv_id string) {
  tlv := ptpmgmt.Conv_PRIORITY1(bTlv)
  fmt.Println("Get reply for", tlv_id)
  fmt.Println("priority1:", tlv.GetPriority1())
}
func (self *myDisp) USER_DESCRIPTION_h(msg ptpmgmt.Message,
                                      bTlv ptpmgmt.BaseMngTlv,
                                      tlv_id string) {
  tlv := ptpmgmt.Conv_USER_DESCRIPTION(bTlv)
  fmt.Println("Get reply for", tlv_id)
  fmt.Println("get user desc:", tlv.GetUserDescription().GetTextField())
}

type myBuild struct { // Implement ptpmgmt.MessageBuilder
  newPriority1 byte
}
func (self *myBuild) BuildTlv(msg ptpmgmt.Message,
                             actionField ptpmgmt.ActionField_e,
                             tlv_id ptpmgmt.Mng_vals_e) bool {
  return ptpmgmt.MessageBuilderBuildTlv(self, msg, actionField, tlv_id)
}
func (self *myBuild) PRIORITY1_b(msg ptpmgmt.Message) ptpmgmt.BaseMngTlv {
  tlv := ptpmgmt.NewPRIORITY1_t()
  tlv.SetPriority1(self.newPriority1)
  return tlv
}

const DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"
/* These Global objects are deleted at the end of main */
var sk ptpmgmt.SockUnix = ptpmgmt.NewSockUnix()
var msg ptpmgmt.Message = ptpmgmt.NewMessage()
var buf ptpmgmt.Buf = ptpmgmt.NewBuf(int64(1000))
var opt ptpmgmt.Options = ptpmgmt.NewOptions()
var dispacher = new(myDisp)
var builder = new(myBuild)
var sequence uint16 = 0

func printError(msg string) int {
  if(ptpmgmt.ErrorIsError()) {
    fmt.Println(ptpmgmt.ErrorGetError())
  } else {
    fmt.Println(msg)
  }
  return -1
}

func nextSequence() uint16 {
  // Ensure sequence in in range of unsigned 16 bits
  sequence++
  if sequence > 0xffff || sequence == 0 {
    sequence = 1
  }
  return sequence
}

func setPriority1(newPriority1 byte) int {
  useBuild := true
  id := ptpmgmt.PRIORITY1
  if (useBuild) {
    builder.newPriority1 = newPriority1
    if builder.BuildTlv(msg, ptpmgmt.SET, id) {
      /* Nake sure Message object remove the reference to the TLV */
      defer msg.ClearData()
    }
  } else {
    pr1 := ptpmgmt.NewPRIORITY1_t()
    /* When deleting the TLV, we should also notify the Message object */
    defer msg.ClearData()
    defer ptpmgmt.DeletePRIORITY1_t(pr1)
    pr1.SetPriority1(newPriority1)
    msg.SetAction(ptpmgmt.SET, id, pr1)
  }
  seq := nextSequence()
  err := msg.Build(buf, seq)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK {
    return printError("build error " + ptpmgmt.MessageErr2str_c(err))
  }
  if !sk.Send(buf, msg.GetMsgLen()) {
    return printError("send")
  }
  if !sk.Poll(uint64(500)) {
    return printError("timeout")
  }
  cnt := sk.Rcv(buf)
  if cnt <= 0 {
    return printError("rcv cnt")
  }
  err = msg.Parse(buf, cnt)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK || msg.GetTlvId() != id ||
     seq != msg.GetSequence() {
    return printError("set fails")
  }
  fmt.Printf("set new priority %v success\n", newPriority1)
  msg.SetAction(ptpmgmt.GET, id)
  seq = nextSequence()
  err = msg.Build(buf, seq)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK {
    return printError("build error " + ptpmgmt.MessageErr2str_c(err))
  }
  if !sk.Send(buf, msg.GetMsgLen()) {
    return printError("send")
  }
  if !sk.Poll(uint64(500)) {
    return printError("timeout")
  }
  cnt = sk.Rcv(buf)
  if cnt <= 0 {
    return printError("rcv cnt")
  }
  err = msg.Parse(buf, cnt)
  if err == ptpmgmt.MNG_PARSE_ERROR_MSG {
    return printError("error message")
  } else if err != ptpmgmt.MNG_PARSE_ERROR_OK {
    return printError("Parse error " + ptpmgmt.MessageErr2str_c(err))
  } else {
    dispacher.CallHadler(msg, msg.GetTlvId(), msg.GetData())
  }
  return 0
}

func main() {
  if !buf.IsAlloc() {
    printError("buffer allocation failed")
    return
  }

  if opt.Parse_options(os.Args) != ptpmgmt.OptionsOPT_DONE {
    printError("fail parsing command line")
    return
  }
  cfg_file := opt.Val('f')
  if cfg_file == "" {
    cfg_file = DEF_CFG_FILE
  }
  fmt.Println("Use configuration file", cfg_file)
  cfg := ptpmgmt.NewConfigFile()
  defer ptpmgmt.DeleteConfigFile(cfg)
  if !cfg.Read_cfg(cfg_file) {
    printError("fail reading configuration file")
    return
  }
  if !sk.SetDefSelfAddress() || !sk.Init() || !sk.SetPeerAddress(cfg) {
    printError("fail init socket")
    return
  }
  prms := msg.GetParams()
  self_id := prms.GetSelf_id()
  self_id.SetPortNumber(uint16(os.Getpid() & 0xffff))
  prms.SetSelf_id(self_id)
  prms.SetBoundaryHops(1)
  prms.SetDomainNumber(cfg.DomainNumber())
  msg.UpdateParams(prms)
  if !msg.UseConfig(cfg) {
    printError("fail using configuration for message")
    return
  }

  id := ptpmgmt.USER_DESCRIPTION
  msg.SetAction(ptpmgmt.GET, id)
  seq := nextSequence()
  err := msg.Build(buf, seq)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK {
    printError("Build error " + ptpmgmt.MessageErr2str_c(err))
    return
  }
  if !sk.Send(buf, msg.GetMsgLen()) {
    printError("send fail")
    return
  }

  // You can get file descriptor with sk.Fileno() and use go select
  if !sk.Poll(uint64(500)) {
    printError("timeout")
    return
  }
  cnt := sk.Rcv(buf)
  if cnt <= 0 {
    printError("rcv cnt")
    return
  }
  err = msg.Parse(buf, cnt)
  if(err == ptpmgmt.MNG_PARSE_ERROR_MSG) {
    fmt.Println("error message")
  } else if err != ptpmgmt.MNG_PARSE_ERROR_OK {
    printError("Parse error " + ptpmgmt.MessageErr2str_c(err))
    return
  } else {
    dispacher.CallHadler(msg)
  }

  // test setting values
  clk_dec := ptpmgmt.NewCLOCK_DESCRIPTION_t()
  defer ptpmgmt.DeleteCLOCK_DESCRIPTION_t(clk_dec)
  clk_dec.SetClockType(0x800)
  physicalAddress := ptpmgmt.NewBinary()
  defer ptpmgmt.DeleteBinary(physicalAddress)
  physicalAddress.SetBin(int64(0), byte(0xf1))
  physicalAddress.SetBin(int64(1), byte(0xf2))
  physicalAddress.SetBin(int64(2), byte(0xf3))
  physicalAddress.SetBin(int64(3), byte(0xf4))
  fmt.Println("physicalAddress:", physicalAddress.ToId())
  fmt.Println("physicalAddress:", physicalAddress.ToHex())
  clk_dec.SetPhysicalAddress(physicalAddress)
  clk_physicalAddress := clk_dec.GetPhysicalAddress()
  fmt.Println("clk.physicalAddress:", clk_physicalAddress.ToId())
  fmt.Println("clk.physicalAddress:", clk_physicalAddress.ToHex())
  manufacturerIdentity := clk_dec.GetManufacturerIdentity()
  fmt.Println("manufacturerIdentity:",
              ptpmgmt.BinaryBufToId(manufacturerIdentity, 3))
  clk_dec.GetRevisionData().SetTextField("This is a test")
  fmt.Println("revisionData:", clk_dec.GetRevisionData().GetTextField())

  setPriority1(147)
  setPriority1(153)

  event := ptpmgmt.NewSUBSCRIBE_EVENTS_NP_t()
  defer ptpmgmt.DeleteSUBSCRIBE_EVENTS_NP_t(event)
  event.SetEvent(ptpmgmt.GetNOTIFY_TIME_SYNC())
  var eStr string
  if event.GetEvent(ptpmgmt.GetNOTIFY_TIME_SYNC()) {
    eStr = "have"
  } else {
    eStr = "not"
  }
  fmt.Printf("getEvent(NOTIFY_TIME_SYNC)=%v\n", eStr)
  if event.GetEvent(ptpmgmt.GetNOTIFY_PORT_STATE()) {
    eStr = "have"
  } else {
    eStr = "not"
  }
  fmt.Printf("getEvent(NOTIFY_PORT_STATE)=%v\n", eStr)
  /* test SigEvent that represent stdVector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
     See std_vectors.Md for more information */
  evnts := ptpmgmt.NewSigEvent()
  defer ptpmgmt.DeleteSigEvent(evnts)
  e := ptpmgmt.NewSLAVE_TX_EVENT_TIMESTAMPS_rec_t()
  defer ptpmgmt.DeleteSLAVE_TX_EVENT_TIMESTAMPS_rec_t(e)
  e.SetSequenceId(1)
  tm := ptpmgmt.NewTimestamp_t()
  defer ptpmgmt.DeleteTimestamp_t(tm)
  tm.FromFloat(4.5)
  e.SetEventEgressTimestamp(tm)
  evnts.Add(e)
  fmt.Printf("Events size %v, seq[0]=%v, ts[0]=%v\n",
    evnts.Size(),
    evnts.Get(0).GetSequenceId(),
    evnts.Get(0).GetEventEgressTimestamp().String())

  /* Free all global objects */
  sk.Close()
  ptpmgmt.DeleteSockUnix(sk)
  ptpmgmt.DeleteMessage(msg)
  ptpmgmt.DeleteBuf(buf)
  ptpmgmt.DeleteOptions(opt)
}

// LD_PRELOAD=../libptpmgmt.so gtest/gtest
