#!/usr/bin/python3
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# Unit test with Python 3 wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
###############################################################################

import ptpmgmt
import unittest
# See: https://wiki.python.org/moin/PyUnit

class myDisp(ptpmgmt.MessageDispatcher):
  def __init__(self):
    self.priority1 = 0
    self.func = 0
    self.id = ''
  def PRIORITY1_h(self, msg, tlv, tlv_id):
    self.func |= 0x1
    self.id = tlv_id
    self.priority1 = tlv.priority1
  def noTlv(self, msg):
    self.func |= 0x2
  def noTlvCallBack(self, msg, tlv_id):
    self.func |= 0x4
    self.id = tlv_id
class myBuild(ptpmgmt.MessageBuilder):
  def PRIORITY1_b(self, msg, tlv):
    self.run = 1
    tlv.priority1 = 117
    return True

class TestPtpmgmtMessageDispBuild(unittest.TestCase):
  def setUp(self):
    """Call before every test case."""
    self.msg = ptpmgmt.Message()
    self.disp = myDisp()
    self.build = myBuild(self.msg)
    self.build.run = 0

  def test_parsedCallHadlerEmptyTLV(self):
    """ Tests callHadler with empty TLV"""
    self.disp.callHadler(self.msg)
    assert self.disp.func == 0x2,    'should call noTlv'
    assert self.disp.id == '',       'should have empty ID'
    assert self.disp.priority1 == 0, 'should not have priority1 value'

  def test_parsedCallHadlerTLV(self):
    """ Tests callHadler method with TLV"""
    tlv = ptpmgmt.PRIORITY1_t()
    tlv.priority1 = 117
    self.disp.callHadler(self.msg, ptpmgmt.PRIORITY1, tlv)
    assert self.disp.func == 0x1,       'should call PRIORITY1_h'
    assert self.disp.id == 'PRIORITY1', 'should have PRIORITY1 ID'
    assert self.disp.priority1 == 117,  'should have priority1 value'

  def test_parsedCallHadlerTLVNoCallback(self):
    """ Tests callHadler method with TLV without callback"""
    tlv = ptpmgmt.PRIORITY2_t()
    tlv.priority2 = 117
    self.disp.callHadler(self.msg, ptpmgmt.PRIORITY2, tlv)
    assert self.disp.func == 0x4,       'should call noTlvCallBack'
    assert self.disp.id == 'PRIORITY2', "should have PRIORITY2 ID"
    assert self.disp.priority1 == 0,    "shouldn't have priority1 value"

  def test_buildEmptyTLV(self):
    """ Tests build empty TLV"""
    assert self.build.buildTlv(ptpmgmt.COMMAND, ptpmgmt.ENABLE_PORT), 'should pass'
    assert self.build.run == 0, "shouldn't call PRIORITY1 callback"

  def test_buildTLV(self):
    """ Tests build TLV"""
    assert self.build.buildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY1), 'should pass'
    assert self.build.run == 1, 'should call PRIORITY1 callback'

  def test_buildTLVNoCallback(self):
    """ Tests build TLV that lack callback"""
    assert not self.build.buildTlv(ptpmgmt.SET, ptpmgmt.PRIORITY2), 'should no pass'
    assert self.build.run == 0, "shouldn't call PRIORITY1 callback"

if __name__ == "__main__":
  unittest.main() # run all tests
