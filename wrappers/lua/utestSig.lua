-- SPDX-License-Identifier: BSD-3-Clause
-- SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com>

--[[
 - Unit test for signaling with Lua wrapper of libptpmgmt

 - @author Erez Geva <ErezGeva2@@gmail.com>
 - @copyright © 2026 Erez Geva
 - ]]

require 'ptpmgmt'
require 'utest_help'
luaunit = require 'luaunit'
-- See: https://luaunit.readthedocs.io/

TestSig = {}
  function TestSig:testPtpmgmtTraverseSig()
    local msg = ptpmgmt.Message()
    local buf = ptpmgmt.Buf(100)
    local size = utest_help.get2MngTlvsSig(buf:get(), buf:size())
    luaunit.assertTrue(size > 0, 'get2MngTlvsSig')
    local prms = msg:getParams()
    prms.rcvSignaling = true
    prms.filterSignaling = false
    luaunit.assertTrue(msg:updateParams(prms), 'updateParams')
    luaunit.assertEquals(msg:parse(buf, size) , ptpmgmt.MNG_PARSE_ERROR_SIG, 'parse')
    luaunit.assertEquals(msg:getSigTlvsCount() , 2, 'getSigTlvsCount')
    luaunit.assertEquals(msg:getSigTlvType(0) , ptpmgmt.MANAGEMENT, 'getSigTlvType')
    luaunit.assertEquals(msg:getSigMngTlvType(0) , ptpmgmt.PRIORITY2, 'getSigMngTlvType')
    luaunit.assertEquals(msg:getSigTlvType(1) , ptpmgmt.MANAGEMENT, 'getSigTlvType')
    luaunit.assertEquals(msg:getSigMngTlvType(1) , ptpmgmt.DOMAIN, 'getSigMngTlvType')
    local sigTlv = msg:getSigTlv(0)
    luaunit.assertEquals(ptpmgmt.get_MngTlvId(sigTlv) , ptpmgmt.PRIORITY2, 'get_MngTlvId')
    local mngTlv1_1 = msg:getSigMngTlv(0)
    local mngTlv1_2 = ptpmgmt.get_BaseMngTlv(sigTlv)
    local pr2_1 = ptpmgmt.conv_PRIORITY2(mngTlv1_1)
    local pr2_2 = ptpmgmt.conv_PRIORITY2(mngTlv1_2)
    luaunit.assertEquals(pr2_1.priority2 , 119, 'pr2_1.priority2')
    luaunit.assertEquals(pr2_2.priority2 , 119, 'pr2_2.priority2')
    sigTlv = msg:getSigTlv(1)
    luaunit.assertEquals(ptpmgmt.get_MngTlvId(sigTlv) , ptpmgmt.DOMAIN, 'get_MngTlvId')
    mngTlv1_1 = msg:getSigMngTlv(1)
    mngTlv1_2 = ptpmgmt.get_BaseMngTlv(sigTlv)
    local domain1 = ptpmgmt.conv_DOMAIN(mngTlv1_1)
    local domain2 = ptpmgmt.conv_DOMAIN(mngTlv1_2)
    luaunit.assertEquals(domain1.domainNumber , 7, 'domain1.domainNumber')
    luaunit.assertEquals(domain2.domainNumber , 7 , 'domain2.domainNumber')
    -- TODO use msg:traversSigTlvsCl in SWIG 4.5.0 once directors are added?
  end

os.exit(luaunit.LuaUnit.run())
