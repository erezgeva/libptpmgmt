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
  buf = pmc.conv_buf("X" * SIZE)
  sequance = 1
  err = msg.build(buf, SIZE, sequance)
  if err != pmc.MNG_PARSE_ERROR_OK:
    txt = pmc.message.err2str_c(err)
    print("build error %s" % txt)
    return

  if not sk.send(buf, msg.getMsgLen()):
    print("send fail")
    return

  # You can get file descriptor with sk.getFd() and use select
  if not sk.poll(500):
    print("timeout")
    return

  cnt = sk.rcv(buf, SIZE)
  if cnt <= 0:
    print("rcv error %d" % cnt)
    return

  err = msg.parse(buf, cnt)
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

sk = pmc.sockUnix()
main()
sk.close()

# If libpmc library is not installed in system,
#  run with:
# ln -sf 2/_pmc.so ; LD_LIBRARY_PATH=.. python2 test.py
# ln -sf 3/_pmc.so ; LD_LIBRARY_PATH=.. python3 test.py
