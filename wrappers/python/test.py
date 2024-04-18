# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
#
# testing for python wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################

import os
import sys
import ptpmgmt

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

class myDisp(ptpmgmt.MessageDispatcher):
  def PRIORITY1_h(self, msg, tlv, tlv_id):
    print("Get reply for %s" % tlv_id)
    print("priority1: %d" % tlv.priority1)
  def USER_DESCRIPTION_h(self, msg, tlv, tlv_id):
    print("Get reply for %s" % tlv_id)
    print("get user desc: %s" % tlv.userDescription.textField)
class myBuild(ptpmgmt.MessageBuilder):
  pr : int
  def PRIORITY1_b(self, msg, tlv):
    tlv.priority1 = self.pr
    return True

sk = ptpmgmt.SockUnix()
msg = ptpmgmt.Message()
buf = ptpmgmt.Buf(1000)
cfg = ptpmgmt.ConfigFile()
opt = ptpmgmt.Options()
dispacher = myDisp()
builder = myBuild(msg)
sequence = 0

def printError(msg):
  if ptpmgmt.Error.isError():
    print(ptpmgmt.Error.getError())
  else:
    print(msg)
  return -1

def nextSequence():
  # Ensure sequence in in range of unsigned 16 bits
  global sequence
  if ++sequence > 0xffff:
    sequence = 1
  return sequence

def setPriority1(newPriority1):
  global sk, msg, buf, dispacher, builder
  useBuild = True
  id = ptpmgmt.PRIORITY1
  if useBuild:
    builder.pr = newPriority1
    builder.buildTlv(ptpmgmt.SET, id)
  else:
    pr1 = ptpmgmt.PRIORITY1_t()
    pr1.priority1 = newPriority1
    msg.setAction(ptpmgmt.SET, id, pr1)
  seq = nextSequence()
  err = msg.build(buf, seq)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("build error %s" % txt)
  if not sk.send(buf, msg.getMsgLen()):
    return printError("send fail")
  msg.clearData()
  if not sk.poll(500):
    return printError("timeout")
  cnt = sk.rcv(buf)
  if cnt <= 0:
    return printError("rcv cnt")
  err = msg.parse(buf, cnt)
  if(err != ptpmgmt.MNG_PARSE_ERROR_OK or msg.getTlvId() != id or
     seq != msg.getSequence()):
    print("set fails")
    return -1
  print("set new priority %d success" % newPriority1)
  msg.setAction(ptpmgmt.GET, id)
  seq = nextSequence()
  err = msg.build(buf, seq)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("build error %s" % txt)
  if not sk.send(buf, msg.getMsgLen()):
    return printError("send fail")
  if not sk.poll(500):
    return printError("timeout")
  cnt = sk.rcv(buf)
  if cnt <= 0:
    return printError("rcv cnt")
  err = msg.parse(buf, cnt)
  if err == ptpmgmt.MNG_PARSE_ERROR_MSG:
    print("error message")
  elif err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    dispacher.callHadler(msg, msg.getTlvId(), msg.getData())
    return 0
  return -1

def main():
  global sk, msg, buf, dispacher
  if not buf.isAlloc():
    print("buffer allocation failed")
    return -1
  if opt.parse_options(sys.argv) != ptpmgmt.Options.OPT_DONE:
    print("fail parsing command line")
    return -1
  cfg_file = opt.val('f')
  if cfg_file == "":
    cfg_file = DEF_CFG_FILE
  print("Use configuration file %s" % cfg_file)
  if not cfg.read_cfg(cfg_file):
    return printError("fail reading configuration file")

  if not sk.setDefSelfAddress() or not sk.init():
    return printError("fail init socket")

  if not sk.setPeerAddress(cfg):
    return printError("fail init peer address")

  prms = msg.getParams()
  prms.self_id.portNumber = os.getpid() & 0xffff
  prms.domainNumber = cfg.domainNumber()
  msg.updateParams(prms)
  msg.useConfig(cfg)
  id = ptpmgmt.USER_DESCRIPTION
  msg.setAction(ptpmgmt.GET, id)
  seq = nextSequence()
  err = msg.build(buf, seq)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("build error %s" % txt)
    return -1

  if not sk.send(buf(), msg.getMsgLen()):
    return printError("send fail")

  # You can get file descriptor with sk.fileno() and use select
  # When using multithreaded Python script, it is better to use
  # native Python select module,
  # as the wrapper uses Python 'Global Interpreter Lock' which might block
  # other threads.
  if not sk.poll(500):
    return printError("timeout")

  cnt = sk.rcv(buf)
  if cnt <= 0:
    return printError("rcv cnt")

  err = msg.parse(buf, cnt)
  if err == ptpmgmt.MNG_PARSE_ERROR_MSG:
    print("error message")
  elif err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    dispacher.callHadler(msg)

  # test setting values
  clk_dec = ptpmgmt.CLOCK_DESCRIPTION_t()
  clk_dec.clockType = 0x800
  physicalAddress = ptpmgmt.Binary()
  physicalAddress.setBin(0, 0xf1)
  physicalAddress.setBin(1, 0xf2)
  physicalAddress.setBin(2, 0xf3)
  physicalAddress.setBin(3, 0xf4)
  print("physicalAddress: %s" % physicalAddress.toId())
  print("physicalAddress: %s" % physicalAddress.toHex())
  clk_dec.physicalAddress.setBin(0, 0xf1)
  clk_dec.physicalAddress.setBin(1, 0xf2)
  clk_dec.physicalAddress.setBin(2, 0xf3)
  clk_dec.physicalAddress.setBin(3, 0xf4)
  print("clk.physicalAddress: %s" % clk_dec.physicalAddress.toId())
  print("clk.physicalAddress: %s" % clk_dec.physicalAddress.toHex())
  print("manufacturerIdentity: %s" %
    ptpmgmt.Binary.bufToId(clk_dec.manufacturerIdentity, 3))
  clk_dec.revisionData.textField = "This is a test"
  print("revisionData: %s" % clk_dec.revisionData.textField)

  # test send
  setPriority1(147)
  setPriority1(153)

  event = ptpmgmt.SUBSCRIBE_EVENTS_NP_t()
  event.setEvent(ptpmgmt.NOTIFY_TIME_SYNC)

  if event.getEvent(ptpmgmt.NOTIFY_TIME_SYNC):
    txt = 'have'
  else:
    txt = 'not'
  print('getEvent(NOTIFY_TIME_SYNC)={}'.format(txt))

  if event.getEvent(ptpmgmt.NOTIFY_PORT_STATE):
    txt = 'have'
  else:
    txt = 'not'
  print('getEvent(NOTIFY_PORT_STATE)={}'.format(txt))

  # test SigEvent that represent std::vector<SLAVE_TX_EVENT_TIMESTAMPS_rec_t>
  # See std_vectors.md for more information
  evnts = ptpmgmt.SigEvent()
  e = ptpmgmt.SLAVE_TX_EVENT_TIMESTAMPS_rec_t()
  e.sequenceId = 1
  e.eventEgressTimestamp.fromFloat(4.5)
  evnts.append(e)
  print('Events size {}, seq[0]={}, ts[0]={}'
    .format(evnts.size(), evnts[0].sequenceId,
    evnts[0].eventEgressTimestamp.string()))
  return 0

main()
sk.close()

# LD_PRELOAD=../../libptpmgmt.so PYTHONPATH=3 python3 test.py
