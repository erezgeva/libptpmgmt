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
SIZE = 2000

def setPriority1(sk, msg, pbuf, sequence, newPriority1):
  pr1 = pmc.PRIORITY1_t()
  pr1.priority1 = newPriority1
  id = pmc.PRIORITY1
  msg.setAction(pmc.SET, id, pr1)
  err = msg.build(pbuf, SIZE, sequence)
  if err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.message.err2str_c(err)
    print("build error %s" % txt)
  if not sk.send(pbuf, msg.getMsgLen()):
    print("send fail")
    return
  if not sk.poll(500):
    print("timeout")
    return
  cnt = sk.rcv(pbuf, SIZE)
  if cnt <= 0:
    print("rcv cnt")
    return -1
  err = msg.parse(pbuf, cnt)
  if(err != pmc.MNG_PARSE_ERROR_OK or msg.getTlvId() != id or
     sequence != msg.getSequence()):
    print("set fails")
    return -1
  print("set new priority %d success" % newPriority1)
  msg.setAction(pmc.GET, id)
  err = msg.build(pbuf, SIZE, sequence)
  if err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.message.err2str_c(err)
    print("build error %s" % txt)
  if not sk.send(pbuf, msg.getMsgLen()):
    print("send fail")
    return
  if not sk.poll(500):
    print("timeout")
    return
  cnt = sk.rcv(pbuf, SIZE)
  if cnt <= 0:
    print("rcv cnt")
    return -1
  err = msg.parse(pbuf, cnt)
  if err == pmc.MNG_PARSE_ERROR_MSG:
    print("error message")
  elif err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    rid = msg.getTlvId()
    idstr = pmc.message.mng2str_c(rid)
    print("Get reply for %s" % idstr)
    if rid == id:
      newPr = pmc.conv_PRIORITY1(msg.getData())
      print("priority1: %d" % newPr.priority1)

def main():
  if len(sys.argv) > 1:
    cfg_file = sys.argv[1]
  else:
    cfg_file = DEF_CFG_FILE
  print("Use configuration file %s" % cfg_file)
  cfg = pmc.configFile()
  if not cfg.read_cfg(cfg_file):
    print("fail reading configuration file")
    return

  if not sk.setDefSelfAddress() or not sk.init() or not sk.setPeerAddress(cfg):
    print("fail init socket")
    return

  msg = pmc.message()
  prms = msg.getParams()
  prms.self_id.portNumber = os.getpid()
  msg.updateParams(prms)
  id = pmc.USER_DESCRIPTION
  msg.setAction(pmc.GET, id)
  # Create buffer for sending
  # And convert buffer to buffer pointer
  pbuf = pmc.conv_buf("X" * SIZE)
  sequence = 1
  err = msg.build(pbuf, SIZE, sequence)
  if err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.message.err2str_c(err)
    print("build error %s" % txt)
    return

  if not sk.send(pbuf, msg.getMsgLen()):
    print("send fail")
    return

  # You can get file descriptor with sk.getFd() and use select
  if not sk.poll(500):
    print("timeout")
    return

  cnt = sk.rcv(pbuf, SIZE)
  if cnt <= 0:
    print("rcv error %d" % cnt)
    return

  err = msg.parse(pbuf, cnt)
  if err == pmc.MNG_PARSE_ERROR_MSG:
    print("error message\n")
  elif err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.message.err2str_c(err)
    print("parse error %s" % txt)
  else:
    rid = msg.getTlvId()
    idstr = pmc.message.mng2str_c(rid)
    print("Get reply for %s" % idstr)
    if rid == id:
      user = pmc.conv_USER_DESCRIPTION(msg.getData())
      print("get user desc: %s" % user.userDescription.textField)

  # test setting values
  clk_dec = pmc.CLOCK_DESCRIPTION_t()
  clk_dec.clockType = 0x800
  physicalAddress = pmc.binary()
  physicalAddress.set(0, 0xf1)
  physicalAddress.set(1, 0xf2)
  physicalAddress.set(2, 0xf3)
  physicalAddress.set(3, 0xf4)
  print("physicalAddress: %s" % physicalAddress.toId())
  print("physicalAddress: %s" % physicalAddress.toHex())
  clk_dec.physicalAddress.set(0, 0xf1)
  clk_dec.physicalAddress.set(1, 0xf2)
  clk_dec.physicalAddress.set(2, 0xf3)
  clk_dec.physicalAddress.set(3, 0xf4)
  print("clk.physicalAddress: %s" % clk_dec.physicalAddress.toId())
  print("clk.physicalAddress: %s" % clk_dec.physicalAddress.toHex())
  print("manufacturerIdentity: %s" %
    pmc.binary.bufToId(clk_dec.manufacturerIdentity, 3))
  clk_dec.revisionData.textField = "This is a test";
  print("revisionData: %s" % clk_dec.revisionData.textField);

  # test send
  setPriority1(sk, msg, pbuf, sequence, 147)
  setPriority1(sk, msg, pbuf, sequence, 153)

sk = pmc.sockUnix()
main()
sk.close()

# If libpmc library is not installed in system,
#  run with:
"""
rm -f *.so;ln -sf 2/_pmc.so;LD_LIBRARY_PATH=.. python2 test.py

rm -f *.so;ln -sf 3/_pmc.cpython-37m-x86_64-linux-gnu.so;\
    LD_LIBRARY_PATH=.. python3 test.py
"""
