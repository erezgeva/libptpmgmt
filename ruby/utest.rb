#!/usr/bin/ruby
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# Unit test with ruby wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
###############################################################################

require 'ptpmgmt'
require 'test/unit'

class MyDisp < Ptpmgmt::MessageDispatcher
  def initialize
    @priority1 = 0
    @func = 0
    @id = ''
  end
  def priority1()
    return @priority1
  end
  def func()
    return @func
  end
  def id()
    return @id
  end
  def PRIORITY1_h(msg, tlv, tlv_id)
    @func |= 0x1
    @id = tlv_id
    @priority1 = tlv.priority1
  end
  def noTlv(msg)
    @func |= 0x2
  end
  def noTlvCallBack(msg, tlv_id)
    @func |= 0x4
    @id = tlv_id
  end
end
class MyBuild < Ptpmgmt::MessageBuilder
  def run
    ret = @run
    @run = 0
    return ret
  end
  def PRIORITY1_b(msg, tlv)
    @run = 1
    tlv.priority1 = 117;
    return true
  end
end

class TestPtpmgmtMessageDispBuild < Test::Unit::TestCase
  def setup
    @msg = Ptpmgmt::Message.new
    @disp = MyDisp.new
    @build = MyBuild.new(@msg)
    @build.run()
  end

  # Tests callHadler with empty TLV
  def test_parsedCallHadlerEmptyTLV
    @disp.callHadler(@msg)
    assert_equal(@disp.func(), 0x2,    'should call noTlv.')
    assert_equal(@disp.id(), '',       'should have empty ID')
    assert_equal(@disp.priority1(), 0, 'should not have priority1 value')
  end

  # Tests callHadler method with TLV
  def test_parsedCallHadlerTLV
    tlv = Ptpmgmt::PRIORITY1_t.new
    tlv.priority1 = 117
    @disp.callHadler(@msg, Ptpmgmt::PRIORITY1, tlv)
    assert_equal(@disp.func(), 0x1,       'should call PRIORITY1_h')
    assert_equal(@disp.id(), 'PRIORITY1', 'should have PRIORITY1 ID')
    assert_equal(@disp.priority1(), 117,  'should have priority1 value')
  end

  # Tests callHadler method with TLV without callback
  def test_parsedCallHadlerTLVNoCallback
    tlv = Ptpmgmt::PRIORITY2_t.new
    tlv.priority2 = 117
    @disp.callHadler(@msg, Ptpmgmt::PRIORITY2, tlv)
    assert_equal(@disp.func(), 0x4,       'should call noTlvCallBack')
    assert_equal(@disp.id(), 'PRIORITY2', "should have PRIORITY2 ID")
    assert_equal(@disp.priority1(), 0,    "shouldn't have priority1 value")
  end

  # Tests build empty TLV
  def test_buildEmptyTLV
    assert(@build.buildTlv(Ptpmgmt::COMMAND, Ptpmgmt::ENABLE_PORT), 'should pass')
    assert_equal(@build.run(), 0, "shouldn't call PRIORITY1 callback")
  end

  # Tests build TLV
  def test_buildTLV
    assert(@build.buildTlv(Ptpmgmt::SET, Ptpmgmt::PRIORITY1), 'should pass')
    assert_equal(@build.run(), 1, 'should call PRIORITY1 callback')
  end

  # Tests build TLV that lack callback
  def test_buildTLVNoCallback
    assert(!@build.buildTlv(Ptpmgmt::SET, Ptpmgmt::PRIORITY2), 'should no pass')
    assert_equal(@build.run(), 0, "shouldn't call PRIORITY1 callback")
  end
end
