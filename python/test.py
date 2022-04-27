# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
#
# testing for python wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
###############################################################################

import os
import sys
import ptpmgmt

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

sk = ptpmgmt.SockUnix()
msg = ptpmgmt.Message()
buf = ptpmgmt.Buf(1000)
cfg = ptpmgmt.ConfigFile()
sequence = 0

def nextSequence():
  # Ensure sequence in in range of unsigned 16 bits
  global sequence
  if ++sequence > 0xffff:
    sequence = 1
  return sequence

def setPriority1(newPriority1):
  global sk, msg, buf
  pr1 = ptpmgmt.PRIORITY1_t()
  pr1.priority1 = newPriority1
  id = ptpmgmt.PRIORITY1
  msg.setAction(ptpmgmt.SET, id, pr1)
  seq = nextSequence()
  err = msg.build(buf, seq)
  if err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("build error %s" % txt)
  if not sk.send(buf, msg.getMsgLen()):
    print("send fail")
    return -1
  if not sk.poll(500):
    print("timeout")
    return -1
  cnt = sk.rcv(buf)
  if cnt <= 0:
    print("rcv cnt")
    return -1
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
    print("send fail")
    return -1
  if not sk.poll(500):
    print("timeout")
    return -1
  cnt = sk.rcv(buf)
  if cnt <= 0:
    print("rcv cnt")
    return -1
  err = msg.parse(buf, cnt)
  if err == ptpmgmt.MNG_PARSE_ERROR_MSG:
    print("error message")
  elif err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    rid = msg.getTlvId()
    idstr = ptpmgmt.Message.mng2str_c(rid)
    print("Get reply for %s" % idstr)
    if rid == id:
      newPr = ptpmgmt.conv_PRIORITY1(msg.getData())
      print("priority1: %d" % newPr.priority1)
      return 0
  return -1

def main():
  global sk, msg, buf
  if not buf.isAlloc():
    print("buffer allocation failed")
    return -1
  if len(sys.argv) > 1:
    cfg_file = sys.argv[1]
  else:
    cfg_file = DEF_CFG_FILE
  print("Use configuration file %s" % cfg_file)
  if not cfg.read_cfg(cfg_file):
    print("fail reading configuration file")
    return -1

  if not sk.setDefSelfAddress() or not sk.init():
    print("fail init socket")
    return -1

  if not sk.setPeerAddress(cfg):
    print("fail init peer address")
    return -1

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
    print("send fail")
    return -1

  # You can get file descriptor with sk.fileno() and use select
  # When using multithreaded Python script, it is better to use
  # native Python select module,
  # as the wrapper uses Python 'Global Interpreter Lock' which might block
  # other threads.
  if not sk.poll(500):
    print("timeout")
    return -1

  cnt = sk.rcv(buf)
  if cnt <= 0:
    print("rcv error %d" % cnt)
    return -1

  err = msg.parse(buf, cnt)
  if err == ptpmgmt.MNG_PARSE_ERROR_MSG:
    print("error message")
  elif err != ptpmgmt.MNG_PARSE_ERROR_OK:
    txt = ptpmgmt.Message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    rid = msg.getTlvId()
    idstr = ptpmgmt.Message.mng2str_c(rid)
    print("Get reply for %s" % idstr)
    if rid == id:
      user = ptpmgmt.conv_USER_DESCRIPTION(msg.getData())
      print("get user desc: %s" % user.userDescription.textField)

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
  print('maskEvent(NOTIFY_TIME_SYNC)={}, getEvent(NOTIFY_TIME_SYNC)={}'
    .format(ptpmgmt.SUBSCRIBE_EVENTS_NP_t.maskEvent(ptpmgmt.NOTIFY_TIME_SYNC),
    txt))

  if event.getEvent(ptpmgmt.NOTIFY_PORT_STATE):
    txt = 'have'
  else:
    txt = 'not'
  print('maskEvent(NOTIFY_PORT_STATE)={}, getEvent(NOTIFY_PORT_STATE)={}'
    .format(ptpmgmt.SUBSCRIBE_EVENTS_NP_t.maskEvent(ptpmgmt.NOTIFY_PORT_STATE),
    txt))

  return 0

main()
sk.close()

# If libptpmgmt library is not installed in system, run with:
"""
rm -rf ptpmgmt.pyc __pycache__;ln -sf 2/*.so;LD_LIBRARY_PATH=.. python2 test.py
rm -rf ptpmgmt.pyc __pycache__;ln -sf 3/*.so;LD_LIBRARY_PATH=.. python3 test.py

"""
