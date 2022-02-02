#SPDX-License-Identifier: GPL-3.0-or-later

#
# testing for python wrapper of libpmc
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#

import os
import sys
import pmc

DEF_CFG_FILE = "/etc/linuxptp/ptp4l.conf"

sk = pmc.SockUnix()
msg = pmc.Message()
buf = pmc.Buf(1000)
sequence = 0

def nextSequence():
  # Ensure sequence in in range of unsigned 16 bits
  global sequence
  if ++sequence > 0xffff:
    sequence = 1;
  return sequence

def setPriority1(newPriority1):
  global sk, msg, buf
  pr1 = pmc.PRIORITY1_t()
  pr1.priority1 = newPriority1
  id = pmc.PRIORITY1
  msg.setAction(pmc.SET, id, pr1)
  seq = nextSequence()
  err = msg.build(buf, seq)
  if err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.Message.err2str_c(err)
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
  if(err != pmc.MNG_PARSE_ERROR_OK or msg.getTlvId() != id or
     seq != msg.getSequence()):
    print("set fails")
    return -1
  print("set new priority %d success" % newPriority1)
  msg.setAction(pmc.GET, id)
  seq = nextSequence()
  err = msg.build(buf, seq)
  if err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.Message.err2str_c(err)
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
  if err == pmc.MNG_PARSE_ERROR_MSG:
    print("error message")
  elif err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.Message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    rid = msg.getTlvId()
    idstr = pmc.Message.mng2str_c(rid)
    print("Get reply for %s" % idstr)
    if rid == id:
      newPr = pmc.conv_PRIORITY1(msg.getData())
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
  cfg = pmc.ConfigFile()
  if not cfg.read_cfg(cfg_file):
    print("fail reading configuration file")
    return -1

  if not sk.setDefSelfAddress() or not sk.init() or not sk.setPeerAddress(cfg):
    print("fail init socket")
    return -1

  prms = msg.getParams()
  prms.self_id.portNumber = os.getpid() & 0xffff
  prms.domainNumber = cfg.domainNumber()
  msg.updateParams(prms)
  msg.useConfig(cfg);
  id = pmc.USER_DESCRIPTION
  msg.setAction(pmc.GET, id)
  seq = nextSequence()
  err = msg.build(buf, seq)
  if err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.Message.err2str_c(err)
    print("build error %s" % txt)
    return -1

  if not sk.send(buf(), msg.getMsgLen()):
    print("send fail")
    return -1

  # You can get file descriptor with sk.getFd() and use select
  if not sk.poll(500):
    print("timeout")
    return -1

  cnt = sk.rcv(buf)
  if cnt <= 0:
    print("rcv error %d" % cnt)
    return -1

  err = msg.parse(buf, cnt)
  if err == pmc.MNG_PARSE_ERROR_MSG:
    print("error message")
  elif err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.Message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    rid = msg.getTlvId()
    idstr = pmc.Message.mng2str_c(rid)
    print("Get reply for %s" % idstr)
    if rid == id:
      user = pmc.conv_USER_DESCRIPTION(msg.getData())
      print("get user desc: %s" % user.userDescription.textField)

  # test setting values
  clk_dec = pmc.CLOCK_DESCRIPTION_t()
  clk_dec.clockType = 0x800
  physicalAddress = pmc.Binary()
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
    pmc.Binary.bufToId(clk_dec.manufacturerIdentity, 3))
  clk_dec.revisionData.textField = "This is a test";
  print("revisionData: %s" % clk_dec.revisionData.textField);

  # test send
  setPriority1(147)
  setPriority1(153)
  return 0

main()
sk.close()

# If libpmc library is not installed in system, run with:
"""
rm -rf pmc.pyc __pycache__;ln -sf 2/*.so;LD_LIBRARY_PATH=.. python2 test.py
rm -rf pmc.pyc __pycache__;ln -sf 3/*.so;LD_LIBRARY_PATH=.. python3 test.py

"""
