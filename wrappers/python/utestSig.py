#!/usr/bin/python3
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>
#
# Unit test for signaling with Python wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2026 Erez Geva
#
###############################################################################

from ptpmgmt import *
import utest_help
import unittest
# See: https://wiki.python.org/moin/PyUnit

class MySigCb(MessageSigTlvCallback):
  def __init__(self):
    MessageSigTlvCallback.__init__(self)
    self.mask = 0
  def callback(msg, tlvType, sigTlv):
    if tlvType != MANAGEMENT:
      return True # return true on failure!
    # First TLV
    if get_MngTlvId(sigTlv) == PRIORITY2:
      tlv = get_BaseMngTlv(sigTlv)
      pr2 = conv_PRIORITY2(tlv)
      self.mask += 1
      return pr2.priority2 != 119; # return false on success!
    # Second TLV
    if get_MngTlvId(sigTlv) == DOMAIN:
      tlv = get_BaseMngTlv(sigTlv)
      domain = conv_DOMAIN(tlv)
      self.mask += 10
      return domain.domainNumber != 7; # return false on success!
    return True; # return true on failure!

class TestPtpmgmtTraverseSig(unittest.TestCase):
  def test_traverseSig(self):
    msg = Message.new
    buf = utest_help.get2MngTlvsSig()
    prms = msg.getParams()
    prms.rcvSignaling = True
    prms.filterSignaling = False
    assert msg.updateParams(prms), 'updateParams'
    assert msg.parse(buf.buf, buf.size) == MNG_PARSE_ERROR_SIG, 'parse'
    assert msg.getSigTlvsCount() == 2, 'getSigTlvsCount'
    assert msg.getSigTlvType(0) == MANAGEMENT, 'getSigTlvType'
    assert msg.getSigMngTlvType(0) == PRIORITY2, 'getSigMngTlvType'
    assert msg.getSigTlvType(1) == MANAGEMENT, 'getSigTlvType'
    assert msg.getSigMngTlvType(1) == DOMAIN, 'getSigMngTlvType'
    sigTlv = msg.getSigTlv(0)
    assert get_MngTlvId(sigTlv) == PRIORITY2, 'get_MngTlvId'
    mngTlv1_1 = msg.getSigMngTlv(0)
    mngTlv1_2 = get_BaseMngTlv(sigTlv)
    pr2_1 = conv_PRIORITY2(mngTlv1_1)
    pr2_2 = conv_PRIORITY2(mngTlv1_2)
    assert pr2_1.priority2 == 119, 'pr2_1.priority2'
    assert pr2_2.priority2 == 119, 'pr2_2.priority2'
    sigTlv = msg.getSigTlv(1)
    assert get_MngTlvId(sigTlv) == DOMAIN, 'get_MngTlvId'
    mngTlv1_1 = msg.getSigMngTlv(1)
    mngTlv1_2 = get_BaseMngTlv(sigTlv)
    domain1 = conv_DOMAIN(mngTlv1_1)
    domain2 = conv_DOMAIN(mngTlv1_2)
    assert domain1.domainNumber == 7, 'domain1.domainNumber'
    assert domain2.domainNumber == 7, 'domain2.domainNumber'
    cb = MySigCb.new()
    assert not msg.traversSigTlvsCl(cb), 'traversSigTlvsCl'
    assert cb.mask() == 11, 'mySigCb mask'
