#!/usr/bin/ruby
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>
#
# Unit test for signaling with Ruby wrapper of libptpmgmt
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2026 Erez Geva
#
###############################################################################

require 'ptpmgmt'
require 'utest_help'
require 'test/unit'
# See: https://ruby-doc.org/stdlib/libdoc/test-unit/rdoc/Test/Unit.html
#      https://ruby-doc.org/stdlib/libdoc/test-unit/rdoc/Test/Unit/Assertions.html

include Ptpmgmt
include Utest_help
BUF_SIZE = 100

class MySigCb < MessageSigTlvCallback
  def initialize
    super
    @mask = 0
  end
  def callback(msg, tlvType, sigTlv)
    if tlvType != MANAGEMENT then
      return true
    end
    id = get_MngTlvId(sigTlv)
    tlv = get_BaseMngTlv(sigTlv)
    # First TLV
    if id == PRIORITY2 then
      pr2 = conv_PRIORITY2(tlv)
      @mask += 1
      return pr2.priority2 != 119; # return false on success!
    end
    # Second TLV
    if id == DOMAIN then
      domain = conv_DOMAIN(tlv)
      @mask += 10
      return domain.domainNumber != 7; # return false on success!
    end
    return true; # return true on failure!
 end
 def mask
    return @mask
 end
end

class TestPtpmgmtTraverseSig < Test::Unit::TestCase
  def test_traverseSig
    msg = Message.new
    buf = Buf.new(BUF_SIZE)
    size = get2MngTlvsSig(buf.get(), BUF_SIZE)
    assert(size > 0, 'get2MngTlvsSig')
    prms = msg.getParams()
    prms.rcvSignaling = true
    prms.filterSignaling = false
    assert(msg.updateParams(prms), 'updateParams')
    assert_equal(msg.parse(buf, size), MNG_PARSE_ERROR_SIG, 'parse')
    assert_equal(msg.getSigTlvsCount(), 2, 'getSigTlvsCount')
    assert_equal(msg.getSigTlvType(0), MANAGEMENT, 'getSigTlvType')
    assert_equal(msg.getSigMngTlvType(0), PRIORITY2, 'getSigMngTlvType')
    assert_equal(msg.getSigTlvType(1), MANAGEMENT, 'getSigTlvType')
    assert_equal(msg.getSigMngTlvType(1), DOMAIN, 'getSigMngTlvType')
    sigTlv = msg.getSigTlv(0)
    assert_equal(get_MngTlvId(sigTlv), PRIORITY2, 'get_MngTlvId')
    mngTlv1_1 = msg.getSigMngTlv(0)
    mngTlv1_2 = get_BaseMngTlv(sigTlv)
    pr2_1 = conv_PRIORITY2(mngTlv1_1)
    pr2_2 = conv_PRIORITY2(mngTlv1_2)
    assert_equal(pr2_1.priority2, 119, 'pr2_1.priority2')
    assert_equal(pr2_2.priority2, 119, 'pr2_2.priority2')
    sigTlv = msg.getSigTlv(1)
    assert_equal(get_MngTlvId(sigTlv), DOMAIN, 'get_MngTlvId')
    mngTlv1_1 = msg.getSigMngTlv(1)
    mngTlv1_2 = get_BaseMngTlv(sigTlv)
    domain1 = conv_DOMAIN(mngTlv1_1)
    domain2 = conv_DOMAIN(mngTlv1_2)
    assert_equal(domain1.domainNumber, 7, 'domain1.domainNumber')
    assert_equal(domain2.domainNumber, 7, 'domain2.domainNumber')
    cb = MySigCb.new()
    assert(!msg.traversSigTlvsCl(cb), 'traversSigTlvsCl')
    assert_equal(cb.mask(), 11, 'mySigCb mask')
  end
end
